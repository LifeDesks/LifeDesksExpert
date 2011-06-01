<?php

function classification_edit_form(&$form_state, $vid_open = '', $tid = '') {

  global $user, $module_path;

  //clear the caches to avoid the classification_status check stickiness
  cache_clear_all();

  $module_path = drupal_get_path('module', 'classification') .'/';

  $vocabularies = taxonomy_get_vocabularies();

  foreach($vocabularies as $vocab) {
    if($vocab->module == 'classification') $vocabs[$vocab->vid] = $vocab->vid;
  }

  $form = array(
    '#cache' => FALSE,
  );

  //add tooltips jQuery extension
  drupal_add_js($module_path . 'js/tipsy/javascripts/jquery.tipsy.js');
  drupal_add_css($module_path . 'js/tipsy/stylesheets/tipsy.css');

  $jstree_files = array(
    'jquery.listen',
    'jquery.metadata',
    'sarissa',
    'sarissa_ieemu_xpath',
    'jquery.xslt',
    'jquery.cookie',
    'jquery.hotkeys',
    'jquery.scrollTo',
    'jquery.fastSerialize'
  );

  jstree_add($jstree_files);

  //add juggernaut client files
  drupal_add_js($module_path . 'juggernaut/json.js');
  drupal_add_js($module_path . 'juggernaut/socket_io.js');
  drupal_add_js($module_path . 'juggernaut/juggernaut.js');

  drupal_add_css($module_path . 'css/classification.css');
  drupal_add_js($module_path . 'js/jquery.ba-dotimeout.min.js');
  drupal_add_js($module_path . 'js/jquery.json-2.2.min.js');
  drupal_add_js($module_path . 'js/classification.js');

  if(module_exists('jquery_ui')) {
    jquery_ui_add(array('effects.highlight'));  
  }

  //add all the term_relation_type titles into some js to make our lives easier
  $classification_flags = variable_get('classification_flags', array());
  $classification_flags[CLASSIFICATION_VERNS] = "";
  $classification_flags[CLASSIFICATION_RANKS] = "";
  $classification_flags[CLASSIFICATION_RELATIONS] = "";

  foreach($classification_flags as $flag => $settings) {
    $flag_name = db_result(db_query("SELECT trt.name FROM {term_relation_types} trt WHERE vid=%d",$flag));
    //clean up the $flag name just in case
    $relations[strtolower(str_replace(" ","_",$flag_name))] = $flag;
  }

  $show_messages = isset($user->classification_show_messages) ? $user->classification_show_messages : 1;
  $vid_open = (array_key_exists($vid_open, $vocabs) && $vid_open !== CLASSIFICATION_PRIMARY) ? $vid_open : "";

  $juggernaut_host = variable_get("juggernaut_host", "www.lifedesks.org");
  $juggernaut_port = variable_get("juggernaut_port", 8080);
  $juggernaut_channel = variable_get("juggernaut_channel", "site.lifedesks.org");

  drupal_add_js(array('classification_primary' => CLASSIFICATION_PRIMARY, 'classification_deletions' => CLASSIFICATION_DELETIONS, 'classification_lost_found' => CLASSIFICATION_LOST_FOUND, 'classification_relations' => $relations, 'classification_show_messages' => $show_messages, 'classification_import_copypaste_limit' => variable_get('classification_import_copypaste_limit', 1000), 'classification_open' => $vid_open, 'juggernaut_host' => $juggernaut_host, 'juggernaut_port' => $juggernaut_port, 'juggernaut_channel' => $juggernaut_channel, 'sid' => $user->sid), 'setting');

  if($tid) {
    $ancestry = array();
    $current->tid = $tid;
    while ($parents = taxonomy_get_parents($current->tid)) {
      $current = array_shift($parents);
      $ancestry[] = 'n' . $current->tid;
    }
    $ancestry = array_unique(array_reverse($ancestry));
    $ancestry = implode(",", $ancestry);
    drupal_add_js(array('classification_tags' => array('ancestry' => $ancestry, 'tid' => 'n' . $tid)), 'setting');
  }
  else {
    drupal_add_js(array('classification_tags' => array('ancestry' => '', 'tid' => '')), 'setting');
  }

  $shortname = variable_get('shortname','');

  $primary_name = (strlen(variable_get('site_name','LifeDesk')) > 20) ? substr(variable_get('site_name','LifeDesk'),0,20) . '... ' . t('Classification') : variable_get('site_name','LifeDesk') . ' ' . t('Classification');

  $form['tree_wrapper'] = array(
    '#type' => 'markup',
    '#value' => ' ',
    '#prefix' => '<div class="tree_wrapper left_tree"><h3 class="classification_title">' . $primary_name . '</h3><div class="tree_actions_bar classification_primary">' . classification_tree_menu(null,CLASSIFICATION_PRIMARY) . '</div><div class="classification_show_metadata"><a href="#" onclick="javascript:TREE.show_metadata(\'left\');return false;" rel="show_metadata"><span class="left">' . t('Show') . '</span> ' . t('Name Data Panel') . '</a></div>',
    '#suffix' => '</div>',
  );

  $form += classification_metadata_form();

  $form['tree_wrapper']['tree'] = array(
    '#type' => 'markup',
    '#value' => '<div id="classification_tree_' . CLASSIFICATION_PRIMARY .'" class="classification_tree"></div>',
  );

  $form += classification_additional_trees_form();

  return $form;
}

/**
 * Form for handling metadata for names
 */
function classification_metadata_form() {

  $form = array(
    '#cache' => FALSE,
  );

  $form['name_metadata'] = array(
     '#type' => 'markup',
     '#value' => '',
     '#prefix' => '<div id="classification_name_metadata"><div id="classification_metadata_wrapper"><span><a href="#" onclick="javascript:return false;">' . t('Close') . '</a></span></div>',
     '#suffix' => '</div>',
  );

  $form['name_metadata']['name'] = array(
     '#type' => 'markup',
     '#value' => '<div id="metadata_taxon_header"><div id="metadata_taxon_title"></div><div id="metadata_taxon_tools"><div id="metadata_taxon_content"><span>' . t('Has content') . '</span></div><div id="metadata_taxon_link"><span><a href="#" target="_blank" class="metadata_taxon_link_href">' . t('View taxon page') . '</a></span></div></div></div>',
  );

  $form['name_metadata']['rank'] = array(
    '#type' => 'textfield',
    '#title' => t('Taxonomic Rank'),
    '#autocomplete_path' => 'classification/autocomplete/' . CLASSIFICATION_RANKS,
    '#size' => 20,
    '#attributes' => array('class' => 'classification_ranks'),
    '#parents' => array('classification_flags',CLASSIFICATION_RANKS),
    '#prefix' => '<div id="classification_rank_wrapper">',
    '#suffix' => '</div>',
  );

  $sql = db_query("SELECT td.tid, td.name FROM {term_data} td WHERE td.vid=%d",CLASSIFICATION_RELATIONS);
  $relationships[''] = 'taxonomic child';
    while ($row = db_fetch_object($sql)) {
     $relationships[$row->tid] = check_plain($row->name);
    }

  $form['name_metadata']['relation_type'] = array(
    '#type' => 'select',
    '#options' => $relationships,
    '#title' => t('Relation to ') . '<span id="metadata_parent_name"></span>',
    '#prefix' => '<div id="classification_relation_wrapper">',
    '#suffix' => "</div>",
    '#parents' => array('classification_flags',CLASSIFICATION_RELATIONS),
    '#attributes' => array('class' => 'classification_relations', 'onchange' => 'javascript:TREE.check_vern(this)'),
  );

  $form['name_metadata']['vern_lang'] = array(
    '#type' => 'textfield',
    '#title' => t('Vernacular Language Code'),
    '#autocomplete_path' => 'classification/autocomplete_vernaculars/' . CLASSIFICATION_VERNS,
    '#size' => 20,
    '#prefix' => '<div id="classification_vernacular_wrapper">',
    '#suffix' => "</div>",
    '#parents' => array('classification_flags',CLASSIFICATION_VERNS),
    '#attributes' => array('class' =>'classification_vernaculars'),
    '#description' => t('Leave unspecified if not applicable'),
  );

  //remove some flags we don't want to appear
  $classification_flags = variable_get('classification_flags', array());
  unset($classification_flags[CLASSIFICATION_RANKS], $classification_flags[CLASSIFICATION_VERNS], $classification_flags[CLASSIFICATION_RELATIONS]);

  if(!empty($classification_flags)) {
    $form['name_metadata']['flags'] = array(
      '#type' => 'fieldset',
      '#title' => t('Biological Flags'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#attributes' => array('id' => 'classification_flags'),
    );

    foreach($classification_flags as $vid => $settings) {
      $req = $settings['settings']['req'] ? '<span class="form-required" title="' . t('This field is required.') . '">*</span>' : '';
      $req_class = $settings['settings']['req'] ? ' classification_required' : '';
      $vocab = taxonomy_vocabulary_load($vid);

      switch($settings['settings']['opt']) {
        case 'textfield':
          $form['name_metadata']['flags'][$vid] = array(
            '#type' => 'textfield',
            '#title' => check_plain($vocab->name) . $req,
            '#description' => check_plain($vocab->help),
            '#autocomplete_path' => 'classification/autocomplete/'.$vid,
            '#size' => 25,
            '#weight' => $settings['settings']['wt'],
            '#parents' => array('classification_flags',$vid),
            '#attributes' => array('class' => 'classification_flags'.$req_class),
          );
        break;

        case 'options':
        default:
          $options = _classification_populate_flags($vid);
          $description = (count($options)==1) ? t('Add flags for <strong>@vocab</strong> !here', array('@vocab' => $vocab->name, '!here' => l(t('here'), 'admin/content/taxonomy/'.$vid.'/add/term'))) : check_plain($vocab->help);
          $form['name_metadata']['flags'][$vid] = array(
            '#type' => 'select',
            '#title' => check_plain($vocab->name) . $req,
            '#description' => $description,
            '#options' => $options,
            '#weight' => $settings['settings']['wt'],
            '#parents' => array('classification_flags',$vid),
            '#attributes' => array('class' => 'classification_flags'.$req_class),
          );
      }
    }
  }
  else {
    $form['name_metadata']['flags'] = array(
      '#type' => 'fieldset',
      '#title' => t('Biological Flags'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['name_metadata']['flags']['flags_message'] = array(
      '#type' => 'markup',
      '#value' => '<div class="messages status classification_flags_message" style="padding-left:30px !important; padding-right:0em">' . t('Create vocabularies for flags !here', array('!here' => l(t('here'), 'admin/content/taxonomy/add/vocabulary/flags'))) . '</div>',
      '#parents' => array('flags'),
    );
  }

  $form['name_metadata']['notes'] = array(
    '#type' => 'fieldset',
    '#title' => t('Taxonomic Notes'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['name_metadata']['notes']['description'] = array(
    '#type' => 'textarea',
    '#cols' => 30,
  );

  $form['name_metadata']['submit'] = array(
    '#type' => 'submit',
    '#prefix' => '<div class="classification_submit">',
    '#suffix' => '</div>',
    '#value' => t('Save'),
    '#weight' => 20,
    '#attributes' => array('onclick' => 'javascript:TREE.save_metadata();return false'),
  );

// Dummy submit button with AHAH to circumvent the Enter key from re-rendering the page
  $form['name_metadata']['dummy_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#attributes' => array('class' => 'classification-buttons hidden'),
    '#ahah' => array(
       'event' => 'click',
       'path' => 'classification/update_metadata',
       'wrapper' => 'message',
       'progress' => array('type' => 'none', 'message' => ''),
    ),
    '#weight' => 20,
  );

  return $form;
}

/**
 * Helper function to populate drop-down selections for name flags
 */
function _classification_populate_flags($vid) {
    $options[''] = t('--None--');
    $query = db_query("SELECT tid,name FROM {term_data} WHERE vid=%d ORDER BY weight ASC",$vid);
    while($result = db_fetch_object($query)) {
        $options[$result->tid] = $result->name;
    }
    return $options;
}

/**
 * Form for handling additional classifications
 */
function classification_additional_trees_form() {

  $vocabs = taxonomy_get_vocabularies();

  $classification_list = '';
  foreach($vocabs as $vocab) {
    if($vocab->module == 'classification' && $vocab->vid !== CLASSIFICATION_PRIMARY) {
        $vocab_name = (strlen(check_plain($vocab->name)) > 27) ? substr(check_plain($vocab->name), 0, 27) . '...' : check_plain($vocab->name);
        $class  = ($vocab->vid == CLASSIFICATION_DELETIONS || $vocab->vid == CLASSIFICATION_LOST_FOUND) ? "important" : "";
        if($vocab->vid == CLASSIFICATION_LOST_FOUND) {
          $class .= " separator";
        }
        elseif ($vocab->vid == CLASSIFICATION_DELETIONS && !CLASSIFICATION_LOST_FOUND) {
          $class .= " separator";
        }
        $classification_list .= '<li class="' . $class . '"rel="' . $vocab->vid . '"><a href="#" onclick="javascript:return false;" alt="' . $vocab_name . '" title="' . $vocab_name . '">' . $vocab_name . '</a></li>';
    }
  }

  $form = array(
    '#cache' => FALSE,
  );

  $tree_wrapper_prefix  = '<div class="tree_wrapper right_tree" rel="0">';
  $tree_wrapper_prefix .= '<ul class="classification_topnav">';
  $tree_wrapper_prefix .= '<li>
            <a href="#" onclick="javascript:return false" alt="' . t('Pick a different tree') . '" title="' . t('Pick a different tree') . '">' . t('Other Classifications / Deletions') . '</a>
            <ul class="classification_subnav">' . $classification_list . '</ul>
        <span></span></li>';
  $tree_wrapper_prefix .= '</ul>';
  $tree_wrapper_prefix .= (user_access('delete classification')) ? '<span class="classification_alt_del"><a href="#" onclick="javascript:TREE.delete_classification();return false">' . t('delete') . '</a></span>' : '';
  $tree_wrapper_prefix .= '<div class="tree_actions_bar classification_secondary">' . classification_tree_menu('alt',null) . '</div><div class="classification_show_metadata_alt"><a href="#" onclick="javascript:TREE.show_metadata(\'right\');return false;" rel="show_metadata"><span class="right">' . t('Show') . '</span>' . ' ' . t('Name Data Panel') . '</a></div>';

  $form['tree_wrapper_alt'] = array(
    '#type' => 'markup',
    '#value' => ' ',
    '#prefix' => $tree_wrapper_prefix,
    '#suffix' => '</div>',
  );

  $form['tree_wrapper_alt']['classification_trees'] = array(
     '#type' => 'markup',
     '#value' => ' ',
     '#prefix' => '<div id="deleted_names_message"><span>' . t('Names with content are in yellow') . '</span><br />' . l(t('Clear deleted names'), 'admin/classification/biological/edit', array('attributes' => array('id' => 'classification_purge_deleted', 'onclick' => 'javascript:return false;'))) . '</div><div id="classification_tree_alternate">',
     '#suffix' => '</div>',
  );

  return $form;
}

function classification_tree_menu($type='',$vid='') {

  global $base_url,$module_path;

  $path_prefix = $base_url . '/';
  $classification_icons_path = $path_prefix . $module_path;
  $domain_classification_path = url($base_url, array('absolute' => TRUE)) . '/classification/';

  if($type) {
    drupal_add_js(array('alt_search' => $domain_classification_path . 'autocomplete/'),'setting');
  }

    $tree_menu = '<a href="#" class="classification-tooltip" title="' . t('New child') . '" rel="create_child';
    $tree_menu .= $type ? '_alt' : '';
    $tree_menu .= '" onclick="javascript:return false;"><img src="' . $classification_icons_path . 'images/page_white_add.gif" /></a>
    <a href="#" class="classification-tooltip" title="' . t('Bulk insert') . '" rel="bulk_insert';
    $tree_menu .= $type ? '_alt' : '';
    $tree_menu .= '" onclick="javascript:return false;"><img src="' . $classification_icons_path . 'images/table_add.gif" /></a>
    <a href="#" class="classification-tooltip" title="' . t('Rename') . '" rel="rename';
    $tree_menu .= $type ? '_alt' : '';
    $tree_menu .= '" onclick="javascript:return false;"><img src="' . $classification_icons_path . 'images/page_white_edit.gif" /></a>
    <a href="#" class="classification-tooltip" title="' . t('Cut') . '" rel="cut';
    $tree_menu .= $type ? '_alt' : '';
    $tree_menu .= '" onclick="javascript:return false;"><img src="' . $classification_icons_path . 'images/cut.gif" /></a>
    <a href="#" class="classification-tooltip" title="' . t('Copy') . '" rel="copy';
    $tree_menu .= $type ? '_alt' : '';
    $tree_menu .= '" onclick="javascript:return false;"><img src="' . $classification_icons_path . 'images/page_white_copy.gif" /></a>
    <a href="#" class="classification-tooltip" title="' . t('Paste') . '" rel="paste';
    $tree_menu .= $type ? '_alt' : '';
    $tree_menu .= '" onclick="javascript:return false;"><img src="' . $classification_icons_path . 'images/page_white_paste.gif" /></a>
    <a href="#" class="classification-tooltip" title="' . t('Delete') . '" rel="delete_name_warning';
    $tree_menu .= $type ? '_alt' : '';
    $tree_menu .= '" onclick="javascript:return false;"><img src="' . $classification_icons_path . 'images/bin_closed.gif" /></a>
    <a href="#" class="classification-tooltip" title="' . t('Refresh') . '" rel="refresh';
    $tree_menu .= $type ? '_alt' : '';
    $tree_menu .= '" onclick="javascript:return false;"><img src="' . $classification_icons_path . 'images/arrow_refresh.gif" /></a>
    <a href="#" class="classification-tooltip" title="' . t('Collapse all') . '" rel="collapse';
    $tree_menu .= $type ? '_alt' : '';
    $tree_menu .= '" onclick="javascript:return false;"><img src="' . $classification_icons_path . 'images/door.gif" /></a>';

    $tree_menu .= '<div id="classification_edit_search';
    $tree_menu .= $type ? '2"' : '"';
    $tree_menu .= ' class="classification_edit_search">';
    $tree_menu .= '<input id="edit-classification-search';
    $tree_menu .= $type ? '2' : '';
    $tree_menu .= '" type="text" class="form-text form-autocomplete classification-search-form-text" maxlength="128" size="18" value="" title="' . t('Search for a name') . '" autocomplete="OFF" />
    <a href="#" class="classification-tooltip classification_edit_search_button';
    $tree_menu .= $type ? ' alt_search_button' : '';
    $tree_menu .= '" alt="' . t('Search') . '" title="' . t('Search') . '" onclick="javascript:return false"><img src="' . $classification_icons_path . 'images/magnifier.gif" /></a>
    <input id="edit-classification-search';
    $tree_menu .= $type ? '2' : '';
    $tree_menu .= '-autocomplete" class="autocomplete" type="hidden" disabled="disabled" value="' . $domain_classification_path . 'autocomplete/' . $vid . '" /></div>';

    return $tree_menu;
}

/**
 * Callback for rendering tree
 */
 function classification_js_tree($vid,$type='xml',$tid='',$search='') {

     global $NAMES_OPTIONS;

     $matches = array();

     if($_GET['tid']) {
       $tid = $_GET['tid'];
     }

     if(!$tid) {
        $tid = 0;
     }
     else {
        $tid = str_replace("n", "",$tid);
     }

     if($_GET['search']) {
       $search = $_GET['search'];
     }
     else {
       $search = '';
     }

     if(!$search) {

       switch ($NAMES_OPTIONS) {
         // Show all names
         case 0:
           $query = db_query("
             SELECT
               td.tid as tid,
               td.name as name,
               td2.name as relationship,
               CASE COALESCE(td2.name,'valid') WHEN 'valid' THEN 0 WHEN 'vernacular name' THEN 2 ELSE 1 END AS relation_order
             FROM
               {term_data} td
             INNER JOIN
               {term_hierarchy} th ON (td.tid=th.tid)
             LEFT JOIN
               ({term_relation} tr
                INNER JOIN
                {term_data} td2 ON (tr.tid2=td2.tid)
               ) ON (tr.tid1=td.tid AND td2.vid=%d)
             WHERE
               th.parent=%d AND td.vid=%d",CLASSIFICATION_RELATIONS,$tid,$vid);
         break;
       // Hide taxonomic relationships & only show valid names
       case 1:
         $query = db_query("
           SELECT
             td.tid,
             td.name,
             null as relationship,
             0 as relation_order
           FROM
             {term_data} td
           INNER JOIN
             {term_hierarchy} th ON (td.tid=th.tid)
           LEFT JOIN
             ({term_relation} tr
              INNER JOIN
              {term_data} td2 ON (tr.tid2=td2.tid)
             ) ON (tr.tid1=td.tid AND td2.vid = %d)
         WHERE
           th.parent=%d AND td.vid=%d AND ISNULL(td2.name)",CLASSIFICATION_RELATIONS,$tid,$vid);
         break;
       // Show all names again
       default:
         $query = db_query("
           SELECT
             td.tid as tid,
             td.name as name,
             td2.name as relationship,
             CASE COALESCE(td2.name,'valid') WHEN 'valid' THEN 0 WHEN 'vernacular name' THEN 2 ELSE 1 END AS relation_order
           FROM
             {term_data} td
           INNER JOIN
             {term_hierarchy} th ON (td.tid=th.tid)
           LEFT JOIN
             ({term_relation} tr
               INNER JOIN
               {term_data} td2 ON (tr.tid2=td2.tid)
             ) ON (tr.tid1=td.tid AND td2.vid=%d)
           WHERE
             th.parent=%d AND td.vid=%d",CLASSIFICATION_RELATIONS,$tid,$vid);
     }

     while ($result = db_fetch_object($query)) {
       $tid = $result->tid;
       $name = $result->name;
       $patterns = array('/name/', '/ = homotypic/', '/ = heterotypic/');

       //get the relations so we can apply css to names.
       $relation = trim(preg_replace($patterns,'',$result->relationship));
       $content = '';
       if($vid == CLASSIFICATION_DELETIONS) {
         $content = classification_media_check(CLASSIFICATION_DELETIONS,$result->tid) ? 'hascontent' : 'nocontent';
       }
       if($vid == CLASSIFICATION_LOST_FOUND) {
         $content = classification_media_check(CLASSIFICATION_LOST_FOUND,$result->tid) ? 'hascontent' : 'nocontent';
       }
       $matches[] = array(
         'name' => $name,
         'tid' => $tid,
         'relation' => $relation,
         'relation_order' => $result->relation_order,
         'content' => $content,
       );
     }

     foreach($matches as $key => $row) {
       $sort_name[$key] = $row['name'];
       $sort_order[$key] = $row['relation_order'];
     }

     array_multisort($sort_order, SORT_ASC, $sort_name, SORT_ASC, $matches);

     if($type=='json') {
       echo classification_to_js($matches,$vid);
     }
     else {
       echo classification_to_xml($matches,$vid);
     }
     
  }
  else {
     $result = db_query("
     SELECT
       td.tid
     FROM {term_data} td
     WHERE
       td.vid = %d AND LOWER(td.name) LIKE LOWER('%%%s%%')", $vid, $search);
      while($object = db_fetch_object($result)) {
        $allparents = taxonomy_get_parents_all($object->tid);
        foreach($allparents as $parent) {
          $parents[] = 'n' . $parent->tid;
        }
      }

      if(!$parents) {
        $parents[] = 'n0';
      }
      $parents = array_unique($parents);
      sort($parents);

      echo implode(",",$parents);
  }
}

function classification_sort(&$dataSet) {
    $args = func_get_args();
    $callString = 'array_multisort(';
    $usedColumns = array();
    for($i = 1, $count = count($args); $i < $count; ++$i) {
        switch(gettype($args[$i])) {
            case 'string':
                $callString .= '$dataSet[\''.$args[$i].'\'], ';
                array_push($usedColumns, $args[$i]);
                break;
            case 'integer':
                $callString .= $args[$i].', ';
                break;
            default:
                throw new Exception('expected string or integer, given '.gettype($args[$i]));
        }
    }
    foreach($dataSet as $column => $array) {
        if(in_array($column, $usedColumns)) continue;
        $callString .= '$dataSet[\''.$column.'\'], ';
    }
    eval(substr($callString, 0, -2).');');
}

/**
  * Helper function to build inline metadata for rulesets
  */
function classification_mdata($relation,$vid,$type="json") {

    if($type=='xml') {
      $str = ' mdata="{';
    }
    else {
      $str = ', "mdata" : "{';
    }

    $rules = array();

    $rules[] = "deletable : true";

    if($vid != CLASSIFICATION_DELETIONS && $vid != CLASSIFICATION_LOST_FOUND) {
      $rules[] = "copyable : true";
    }
    else {
      $rules[] = "creatable : false, renameable : false";
    }

    if($relation) {
      $rules[] = "creatable : false";
    }

    $str .= implode(", ",$rules);

    $str .= '}"';

    return $str;
}

function classification_media_check($vid,$tid) {
  //look through the tid & its children to return true if content and false if not

  $result = db_fetch_object(db_query("
    SELECT
      n.nid
    FROM {node} n
    JOIN {term_node} tn ON (n.nid=tn.nid)
    WHERE tn.tid = %d LIMIT 1",$tid));

  if($result->nid) {
    return true;
  }
  else {
    $content = false;
    $tree = classification_find_nodes($vid,$tid);
    if($tree) {
      $content = true;
    }
    return $content;
  }
}

/**
 * Build some json for the tree renderer
 */
function classification_to_js($var,$vid) {

   drupal_set_header('Content-Type: text/plain; charset=utf-8');
   drupal_set_header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0, max-age=0');
   drupal_set_header('Pragma: no-cache');
   drupal_set_header('Expires: Thu, 01 Jan 1970 00:00:00 GMT');

   if(!$var) return '{ }';
   $str = '';
   if(count($var) > 1) {
      $str .= "[" . "\n";
   }
   foreach($var as $parent) {
      $str .= "{" . "\n";
      $str .= "\"attributes\" : { \"id\" : \"n" . $parent['tid'] . "\"";
      if($parent['relation'] || $parent['content']) {
        $str .= ", \"class\" : \"" . trim($parent['relation'] . " " . $parent['content']) . "\"";
      }
      $str .= classification_mdata($parent['relation'],$vid);
      $str .= " }," . "\n";
      $str .= "\"data\" : \"" . $parent['name'] . "\"";

      $children = classification_count_children($vid,$parent['tid']);

      if($children > 0){
         $str .= "," . "\n" . "\"state\" : \"closed\"" . "\n";
      }
      else {
         $str .= "\n";
      }

      $str .= "}";
      $counter++;
      if($counter !== count($var)){
         $str .= ","  . "\n";
      }
   }
   if(count($var) > 1) {
      $str .= "\n" . "]" . "\n";
   }

   return $str;
}

/**
 * Build some xml for the tree renderer
 */
function classification_to_xml($var,$vid) {

   drupal_set_header('Content-Type: text/xml; charset: utf-8');
   drupal_set_header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0, max-age=0');
   drupal_set_header('Pragma: no-cache');
   drupal_set_header('Expires: Thu, 01 Jan 1970 00:00:00 GMT');

   if(!$var) return '<root></root>';

   $str = '<root>' . "\n";

   foreach($var as $parent) {
      $str .= '<item id="n' . $parent['tid'] . '"';
      $children = classification_count_children($vid,$parent['tid']);
      if($children > 0){
         $str .= ' state="closed" hasChildren="true"';
      }
      if($parent['relation'] || $parent['content']) {
        $str .= ' class="' . trim($parent['relation'] . ' ' . $parent['content']) . '"';
      }
      $str .= classification_mdata($parent['relation'],$vid,'xml');
      $str .= '>' . "\n";
      $str .= '<content>' . "\n";
      $str .= '<name>';
      $str .= '<![CDATA[' . $parent['name'] . ']]></name>' . "\n";
      $str .= '</content>' . "\n";
      $str .= '</item>' . "\n";
   }

   $str .= '</root>';

   return $str;
}

/**
 * Helper function to determine if term has children
 * Nested sets doesn't really help here because we're just interested in the immediate children
 */
function classification_count_children($vid,$tid,$output = false) {

   global $NAMES_OPTIONS;

   switch ($NAMES_OPTIONS) {
     // Show all names
     case 0:
       $result = db_result(db_query("
         SELECT
           COUNT(*) AS count
         FROM
           {term_data} td
         INNER JOIN
           {term_hierarchy} th ON (td.tid = th.tid)
         WHERE
           th.parent = %d AND td.vid = %d", $tid,$vid));
       break;
     // Hide synonyms and vernaculars: nestedset doesn't really help us here
     case 1:
     $result = db_result(db_query("
       SELECT
         COUNT(*) AS count
       FROM
         {term_data} td
       INNER JOIN
         {term_hierarchy} th ON (td.tid=th.tid)
       LEFT JOIN
         ({term_relation} tr
          INNER JOIN
          {term_data} td2 ON (tr.tid2=td2.tid)
         ) ON (tr.tid1=td.tid AND td2.vid = %d)
     WHERE
       th.parent=%d AND td.vid=%d AND ISNULL(td2.name)",CLASSIFICATION_RELATIONS,$tid,$vid));
       break;
     // Show all names by default
     default:
       $result = db_result(db_query("
         SELECT
           COUNT(*) AS count
         FROM
           {term_data} td
         INNER JOIN
           {term_hierarchy} th ON (td.tid=th.tid)
         WHERE
           td.vid = %d AND th.parent = %d", $vid,$tid));
   }

   if($output) {
     drupal_json(array('status' => 'ok', 'count' => $result));
   }
   else {
    return $result;
  }
}

/**
 * Callback for autocomplete of filter
 */
function classification_autocomplete_tree($vid,$string='') {

  global $NAMES;

  $regexp = '%(?:^|,\ *)("(?>[^"]*)(?>""[^"]* )*"|(?: [^",]*))%x';
  preg_match_all($regexp, $string, $matches);
  $array = $matches[1];

  $last_string = trim(array_pop($array));
  $matches = array();
  $classification_autocomplete = variable_get('taxon_description_autocomplete_size',3);
  if (strlen($last_string) >= $classification_autocomplete ) {
    $result = db_query_range("
    SELECT
      a.tid, a.name
    FROM
      {term_data} a
    INNER JOIN
      {taxonomy_enhancer_value_text} b ON a.tid = b.tid
    WHERE
      a.vid = %d AND b.fid = 'field_rank' AND b.value <> 'Species' AND  b.value <> 'Subspecies' AND LENGTH(b.value)>1 AND LOWER(a.name) LIKE LOWER('%%%s%%') ORDER BY a.name", 't', 'tid', $vid, $last_string, 0, 10);

    while ($tag = db_fetch_object($result)) {
      $n = $tag->name;

    $matches[$n] = check_plain($NAMES->canonical_form($tag->name));
    }
  }
  classification_autocomplete_to_json($matches);
}

function classification_autocomplete_to_json($var = NULL) {
  drupal_set_header('Content-Type: text/javascript; charset=utf-8');
  if (isset($var)) {
    echo classification_autocomplete_to_js($var);
  }
}

function classification_autocomplete_to_js($var) {
  switch (gettype($var)) {
    case 'boolean':
      return $var ? 'true' : 'false'; // Lowercase necessary!
    case 'integer':
    case 'double':
      return $var;
    case 'resource':
    case 'string':
      return '"'. str_replace(array("\r", "\n", "<", ">", "&"),
                              array('\r', '\n', '\x3c', '\x3e', '\x26'),
                              addslashes($var)) .'"';
    case 'array':
      // Arrays in JSON can't be associative. If the array is empty or if it
      // has sequential whole number keys starting with 0, it's not associative
      // so we can go ahead and convert it as an array.
      if (empty ($var) || array_keys($var) === range(0, sizeof($var) - 1)) {
        $output = array();
        foreach ($var as $v) {
          $output[] = classification_autocomplete_to_js($v);
        }
        return '[ '. implode(', ', $output) .' ]';
      }
      // Otherwise, fall through to convert the array as an object.
    case 'object':
      $output = array();
      foreach ($var as $k => $v) {
        $output[] = classification_autocomplete_to_js(strval($k)) .': '. classification_autocomplete_to_js($v);
      }
      return '{ '. implode(', ', $output) .' }';
    default:
      return 'null';
  }
}

/**
 * Callback to check relations
 */
function classification_checkterms(){

  $form = array(
    '#post' => $_POST,
  );

  //first, we make the associative array out of the original  data POSTed back as JSON
  $original = json_decode(urldecode(str_replace('\x','%',$_REQUEST['original'])),true);

  $missing = false;
  $vernacular_message = '';
  $list = '<ul class="classification_flags_missing">';

  //examine the relations
  foreach($original['relations'] as $relations) {
    $type = str_replace('_', ' ',$relations['type']);
    $vid = $relations['vid'];
    $tid = $relations['tid'];
    $value = $relations['value'];
    $dom_name = $relations['dom_name'];

    //get the posted value to compare with $value, then check it
    $posted_value = $_REQUEST['classification_flags'][$vid];

    if($posted_value && !is_numeric($posted_value)) {
        //vernaculars are special, so convert those & provide a different message
        if($vid == CLASSIFICATION_VERNS) {
          $result = db_fetch_object(db_query("SELECT td.tid FROM {term_data} td WHERE RIGHT(td.name,2)='%s' AND td.vid=%d",$posted_value,$vid));
          if(!$result) {
            $vernacular_message = t('%code was not found in the list of vernacular codes. Please choose another code.', array('%code' => $posted_value));
          }
        }
        else {
          $result = db_fetch_object(db_query("SELECT td.tid FROM {term_data} td WHERE td.name='%s' AND td.vid=%d",$posted_value,$vid));
          if(!$result) {
            $missing = true;
            $list .= '<li rel="' . $vid . '"><input type="checkbox" name="classification_flags_missing['.$vid.']"><span style="font-weight:bold">' . check_plain($posted_value) . '</span> (' . $type . ')</li>';
            $list .= '<input type="hidden" value="'.$posted_value.'" name="classification_flags_add['.$vid.'][name]">';
            $list .= '<input type="hidden" value="'.$vid.'" name="classification_flags_add['.$vid.'][vid]">';
          }
        }
    }
  }

  $list .= '</ul>';

  $message = $missing ? t('The following entries were missing in your collection of flags. Select those you would like to add.'): '';

  drupal_json(array('status' => TRUE, 'missing' => $missing, 'vernacular_message' => $vernacular_message, 'message' => $message, 'data' => $list));
}

/**
 * Callback to add term_relation terms
 */
function classification_addflags() {

    $txn = new Transaction();

    $form = array(
      '#post' => $_POST,
    );

    $flags = $_REQUEST['classification_flags_missing'];
    $flag_data = $_REQUEST['classification_flags_add'];

    $added = false;

    foreach ($flags as $vid => $val) {
      if(isset($vid)) {
          //let's make certain the term doesn't already exists(i.e. clicking twice)
          $result = db_fetch_object(db_query("SELECT td.tid FROM {term_data} td WHERE td.name='%s' AND td.vid=%d",$flag_data[$vid]['name'],$flag_data[$vid]['vid']));
          if(!$result) {
              $flag = array(
                'name' => $flag_data[$vid]['name'],
                'vid' => $flag_data[$vid]['vid'],
              );
              taxonomy_save_term($flag);
              $added = true;
          }
      }
    }

    if($added) {
      $message = t('Your new flags were successfully added. Please resubmit your form to save the changes.');
    }
    else {
      $message = t('Some of those flags may already exist, you did not check one to add, or something bad happened.');
    }

    drupal_json(array('status' => $added, 'message' => $message));
}

/**
 * Callback to insert bulk list of names under a parent
 */
function classification_bulkadd() {

  global $user;

  $txn = new Transaction();

  $form = array(
    '#post' => $_POST,
  );

  $vid = $_REQUEST['vid'];
  $parent = $_REQUEST['parent'];
  $children = $_REQUEST['children'];

  $parent_term = taxonomy_get_term($parent);

  if($vid == CLASSIFICATION_PRIMARY) {
    $vocab->name = t('!classification Classification', array('!classification' => variable_get('site_name', 'LifeDesk')));  
  }
  else {
    $vocab = taxonomy_vocabulary_load($vid);
  }

  $whole = trim($children);
  $temp = explode("\n",$whole);

  $limit = variable_get('classification_import_copypaste_limit', 1000);

  if(count($temp) <= $limit) {
    
      //set the status to busy
      _classification_lock_classification();
    
      foreach ($temp as $a) {
        $name = strip_tags(trim($a));
        if($name) {
            $term = array(
              'name' => $name,
              'parent' => $parent_term->tid,
              'vid' => $vid,
            );
            taxonomy_save_term($term);  
        }
      }
    
      //set the status back to ready
      _classification_unlock_classification();
    
      $status = 'bulkadded';

      $action = array(
        'tid' => 0,
        'parent' => $parent_term->tid,
        'vid' => $vid
      );
      $message = t('!count name(s) were bulk inserted into !classification under !parent', array('!count' => count($temp), '!classification' => $vocab->name, '!parent' => ($parent_term->name) ? $parent_term->name : t('root')));
      $classification_log = array(
        'uid' => $user->uid,
        'vid_from' => 0,
        'vid_to' => $parent_term->vid,
        'type' => 'added',
        'description' => $action['message'],
        'action' => $action,
        'created' => time(),
      );
      drupal_write_record('classification_log', $classification_log);
  }
  else {
    $status = 'failed';
    $message = t('Sorry, you attempted to paste more than the limit of !limit names.', array('!limit' => $limit));
  }

  drupal_json(array('status' => $status, 'message' => $message));

  if($status == 'bulkadded') {
    classification_push_message('bulkadded', $action, $message);
  }
}

/**
 * Callback to get parent tid for use in the editor
 */
function classification_get_parent(){
  $form = array(
    '#post' => $_POST,
  );
  $child = $_REQUEST['child'];
  $child_parent = $_REQUEST['child_parent'];

  $result = db_fetch_object(db_query("SELECT parent as tid FROM {term_hierarchy} WHERE tid = %d", $child_parent));

  drupal_json(array('child' => $child, 'dest' => $result->tid));
}

/**
 * Callback to get parent tids for use in autoexpanding the editor
 */
function classification_get_parents($tid){
  $current->tid = $tid;
  $ancestry = array();
  while ($parents = taxonomy_get_parents($current->tid)) {
     $current = array_shift($parents);
     $ancestry[] = 'n'.$current->tid;
  }
  $ancestry = array_reverse($ancestry);
  $ancestry = implode(",", $ancestry);

  drupal_json(array('ancestry' => $ancestry));
}

/**
 * Callback for inline editing of name
 */
function classification_edit_name() {

  global $user;

  $txn = new Transaction();

  $form = array(
    '#post' => $_POST,
  );

  $tid = $_REQUEST['tid'];
  $vid = $_REQUEST['vid'];
  $value = $_REQUEST['value'];

  $form_state = array('submitted' => FALSE);

  $parent = db_fetch_object(db_query("SELECT parent FROM {term_hierarchy} WHERE tid=%d", $tid));
  $parent_term = taxonomy_get_term($parent->parent);
  $parent_name = ($parent->parent == 0) ? 'root' : $parent_term->name;

  $term_array = taxonomy_get_term($tid);
  $value_db = $term_array->name;

  //check to see if name already exists
  $exists = false;
  $query = db_query("SELECT tid FROM {term_data} WHERE vid=%d AND name=LOWER('%s')",$vid,strtolower($value));
  while($tids = db_fetch_object($query)) {
    if($tids->tid && $tid != $tids->tid) {
      $exists = true;
    }
  }
  if($exists) {
    //perform the update but provide some feedback to later delete if requested
    $term = array(
      'tid' => $tid,
      'name' => $value,
      'parent' => $parent->parent,
      'vid' => $vid,
    );
    taxonomy_save_term($term);
    
    //log the edit
    $action = array(
        'tid' => $tid,
        'parent' => $parent->parent,
        'vid' => $vid
    );
    $message = t('A duplicate name !value was added under !parent', array('!value' => $value, '!parent' => $parent_name));
    $classification_log = array(
        'uid' => $user->uid,
        'type' => 'added',
        'vid_to' => $vid,
        'vid_from' => $vid,
        'description' => $message,
        'action' => $action,
        'created' => time(),
    );
    drupal_write_record('classification_log', $classification_log);

    drupal_json(array('status' => 'duplicate', 'message' => $message));
  }
  else {
      if($value !== $value_db) {
         //perform the update
         $term = array(
          'tid' => $tid,
          'name' => $value,
          'vid' => $vid,
          'parent' => $parent->parent,
         );
         taxonomy_save_term($term);
         //log the edit
         $action = array(
            'tid' => $tid,
            'parent' => $parent->parent,
            'vid' => $vid
         );
         $message = ($value_db == 'New taxon') ? t('!value added under !parent', array('!value' => $value, '!parent' => $parent_name)) : t('!value_db changed to !value', array('!value_db' => $value_db, '!value' => $value));
         $classification_log = array(
            'uid' => $user->uid,
            'type' => ($value_db == 'New taxon') ? 'added' : 'updated',
            'description' => $message,
            'vid_from' => $vid,
            'vid_to' => $vid,
            'action' => $action,
            'created' => time(),
         );
         drupal_write_record('classification_log', $classification_log);
        
         drupal_json(array('status' => 'edited', 'message' => $message));

         classification_push_message($classification_log['type'], $action, $message);
      }
      else {
         drupal_json(array('status' => '', 'message' => ''));
      }
  }

}

/**
 * Build form for name metadata using jQuery to adjust selections
 */
function classification_get_metadata($vid='',$tid='') {

  global $NAMES;

  $form_state = array('submitted' => FALSE);

  if(!$tid || !$vid) {
    return;
  }

  // Get the name of the taxon
  $taxon = taxonomy_get_term($tid);
  $parents = taxonomy_get_parents($tid);

  //we assume that we only have one parent
  $parent = array_pop($parents);

  //find the parent rank
  $rank = classification_get_rank($parent->tid);

  $parent_name = $parent->name ? $parent->name : '';

  //start the construction of what we know about this name, including the immediate parent
  $metadata[$tid] = array(
    'tid' => $tid,
    'vid' => $vid,
    'name' => $taxon->name,
    'description' => ($taxon->description) ? $taxon->description : "",
    'parent_tid' => ($parent->tid) ? $parent->tid : "",
    'parent_name' => classification_check_italicize($parent->tid) ? $NAMES->italicized_form($parent->name) : $parent_name
  );

  // Get all the relations that we know about, which include ranks, taxonomic relations, vernaculars and any user-defined flags

  $query = db_query("
    SELECT
     trt.name as relation_type,
     trt.vid,
     td.tid,
     td.name,
     tr.trid
   FROM
    {term_relation_types} trt
   LEFT JOIN
    (
      {term_relation_has_type} trht
      INNER JOIN
      {term_relation} tr ON (trht.trid=tr.trid AND tr.tid1=%d)
      INNER JOIN
      {term_data} td ON (tr.tid2=td.tid)
    ) ON (trht.rtid=trt.rtid)", $tid);

  $italicize = false;
  $relations = array();
  while($result = db_fetch_object($query)) {
    $relations[] = array(
      'type' => check_plain(str_replace(" ","_",$result->relation_type)),
      'trid' => ($result->trid) ? $result->trid : "",
      'tid' => ($result->tid) ? $result->tid : "",
      'vid' => ($result->vid) ? $result->vid : "",
      'dom_name' => 'classification_flags['.$result->vid.']',  //constructed to be exctly same as DOM name
      'value' => ($result->name) ? $result->name : ""
    );
    if($result->relation_type == 'taxonomic ranks' && classification_check_italicize($tid)) {
      $italicize = true;
    }
    if($result->tid && $result->relation_type == 'taxonomic relationships' && $result->name !== 'vernacular name') {
      $italicize = true;
    }
  }

  $metadata[$tid]['relations'] = $relations;

  if($italicize) {
    $metadata[$tid]['name'] = $NAMES->italicized_form($metadata[$tid]['name']);
  }

  $content = false;
  $result = db_result(db_query("SELECT COUNT(*) FROM {term_node} WHERE tid=%d",$tid));
  if($result>0) {
    $content = true;
  }

  $metadata[$tid]['content'] = $content;

  //pop the array because we don't care that it's all nested under a tid
  $metadata = array_pop($metadata);

  echo json_encode(array('status' => TRUE,'data' => $metadata));
}

/**
 * Callback to save name metadata
 */
function classification_update_metadata() {

    global $user;
    
    $txn = new Transaction();

    $form = array(
      '#post' => $_POST,
    );

    //first, we make the associative array out of the original data POSTed back as JSON so we can make the comparisons
    $original = json_decode(urldecode(str_replace('\x','%',$_REQUEST['original'])),true);

    $tid = $original['tid'];
    $name = $original['name'];

    $description = $_REQUEST['description'];

    $term = taxonomy_get_term($tid);

    if($description) {
      db_query("UPDATE {term_data} td SET td.description='%s' WHERE td.tid=%d",$description,$tid);
    }

    //examine the relations
    if(!$original['relations']) return;
    foreach($original['relations'] as $relations) {
      $type = str_replace('_', ' ',$relations['type']);
      $trid = $relations['trid'];
      $vid = $relations['vid'];
      $relation_tid = $relations['tid'];
      $value = $relations['value'];
      $dom_name = $relations['dom_name'];

      //get the posted value to compare with $tid or $value
      $posted_value = $_REQUEST['classification_flags'][$vid];

      //update an existing relation
      if($posted_value && $trid) {
        //if a drop-down selection
        if(is_numeric($posted_value)) {
          db_query("UPDATE {term_relation} tr SET tr.tid2=%d WHERE tr.trid=%d",$posted_value,$trid);
          //get the name for logging purposes
          $posted_value = db_result(db_query("SELECT td.name FROM {term_data} td WHERE td.tid=%d",$posted_value));
        }
        //if an autocomplete, we need the tid
        else {
          //vernaculars are special
          if($vid==CLASSIFICATION_VERNS) {
            $relation_tid = db_result(db_query("SELECT td.tid FROM {term_data} td WHERE td.vid=%d AND RIGHT(td.name,2)='%s'",$vid,$posted_value));
          }
          else {
            $relation_tid = db_result(db_query("SELECT td.tid FROM {term_data} td WHERE td.vid=%d AND td.name='%s'",$vid,$posted_value));
          }
          db_query("UPDATE {term_relation} tr SET tr.tid2=%d WHERE tr.trid=%d",$relation_tid,$trid);
        }
        //log the change only if the value has actually changed
        if($posted_value !== $value) {
          $classification_log = array(
            'uid' => $user->uid,
            'type' => 'flag',
            'vid_to' => $term->vid,
            'vid_from' => 0,
            'description' => t('!name changed from !value to !posted_value for !type', array('!name' => $name, '!value' => $value, '!posted_value' => $posted_value, '!type' => $type)),
            'created' => time(),
          );
          drupal_write_record('classification_log', $classification_log);
        }
      }
      //a new relation so we need to insert term_relation and term_relation_has_type values
      else if ($posted_value && !$trid) {
        //if a drop-down selection
        if(is_numeric($posted_value)) {
          db_query("INSERT INTO {term_relation} (tid1,tid2) VALUES (%d,%d)",$tid,$posted_value);
          $rtid = db_result(db_query("SELECT trt.rtid FROM {term_relation_types} trt WHERE trt.vid=%d",$vid));
          $trid = db_last_insert_id('term_relation','trid');
          db_query("INSERT INTO {term_relation_has_type} (trid,rtid) VALUES (%d,%d)",$trid,$rtid);
          //get the name for logging purposes
          $posted_value = db_result(db_query("SELECT td.name FROM {term_data} td WHERE td.tid=%d",$posted_value));
        }
        //if an autocomplete, we need the tid
        else {
          //vernaculars are special
          if($vid==CLASSIFICATION_VERNS) {
            $relation_tid = db_result(db_query("SELECT td.tid FROM {term_data} td WHERE td.vid=%d AND RIGHT(td.name,2)='%s'",$vid,$posted_value));
          }
          else {
            $relation_tid = db_result(db_query("SELECT td.tid FROM {term_data} td WHERE td.vid=%d AND td.name='%s'",$vid,$posted_value));
          }
          db_query("INSERT INTO {term_relation} (tid1,tid2) VALUES (%d,%d)",$tid,$relation_tid);
          $rtid = db_result(db_query("SELECT trt.rtid FROM {term_relation_types} trt WHERE trt.vid=%d",$vid));
          $trid = db_last_insert_id('term_relation','trid');
          db_query("INSERT INTO {term_relation_has_type} (trid,rtid) VALUES (%d,%d)",$trid,$rtid);
        }
        $classification_log = array(
            'uid' => $user->uid,
            'vid_to' => $term->vid,
            'vid_from' => 0,
            'type' => 'flag',
            'description' => t('!name given !posted_value for !type', array('!name' => $name, '!posted_value' => $posted_value, '!type' => $type)),
            'created' => time(),
        );
        drupal_write_record('classification_log', $classification_log);
      }
      //an existing relation and posted_value is blank (i.e. intention is to remove data)
      else if (!$posted_value && $trid) {
        db_query("DELETE FROM {term_relation} WHERE trid=%d",$trid);
        db_query("DELETE FROM {term_relation_has_type} WHERE trid=%d",$trid);
        
        $classification_log = array(
            'uid' => $user->uid,
            'vid_to' => $term->vid,
            'vid_from' => 0,
            'type' => 'flag',
            'description' => t('!name had flag removed for !type', array('!name' => $name, '!type' => $type)),
            'created' => time(),
        );
        drupal_write_record('classification_log', $classification_log);
      }
    }

    //if a taxonomic relationship is specified, delete any existing rank relationships for that name
    if($_REQUEST['classification_flags'][CLASSIFICATION_RELATIONS]) {
        db_query("DELETE FROM {term_relation} USING {term_relation} INNER JOIN {term_relation_has_type} ON ({term_relation}.trid={term_relation_has_type}.trid) INNER JOIN {term_relation_types} ON ({term_relation_has_type}.rtid={term_relation_types}.rtid) WHERE {term_relation_types}.vid=%d AND {term_relation}.tid1=%d",CLASSIFICATION_RANKS,$tid);
    }

    //if a rank or no relationships specified, delete any existing taxonomic relationships & vernaculars
    if($_REQUEST['classification_flags'][CLASSIFICATION_RANKS] || (!$_REQUEST['classification_flags'][CLASSIFICATION_RELATIONS] && !$_REQUEST['classification_flags'][CLASSIFICATION_VERNS] && !$_REQUEST['classification_flags'][CLASSIFICATION_RANKS])) {
        db_query("DELETE FROM {term_relation} USING {term_relation} INNER JOIN {term_relation_has_type} ON ({term_relation}.trid={term_relation_has_type}.trid) INNER JOIN {term_relation_types} ON ({term_relation_has_type}.rtid={term_relation_types}.rtid) WHERE ({term_relation_types}.vid=%d OR {term_relation_types}.vid=%d) AND {term_relation}.tid1=%d",CLASSIFICATION_RELATIONS,CLASSIFICATION_VERNS,$tid);
    }

    drupal_json(array('status' => 'edited', 'data' => '', 'message' => t('Nomenclature successfully updated for !name', array('!name' => $name))));
}

/**
 * Callback to add a new name
 */
function classification_add_name() {

  $txn = new Transaction();

  $form = array(
    '#post' => $_POST,
  );

  $vid = $_REQUEST['vid'];
  $parent_tid = $_REQUEST['parent_tid'];
  $name = $_REQUEST['name'];

  $term = array(
        'vid' => $vid,
        'parent' => $parent_tid,
        'name' => $name,
      );

  taxonomy_save_term($term);

  $parent = db_fetch_object(db_query("SELECT name FROM {term_data} WHERE tid = %d",$term['parent']));
  drupal_json(array('status' => 'added', 'message' => t('Name data successfully added'), 'tid' => $term['tid'], 'parent_name' => $parent->name));
}

/**
 * Callback to delete a name
 */
function classification_delete_name() {

  global $user;

  $txn = new Transaction();

  $form = array(
    '#post' => $_POST,
  );

  $tid = $_REQUEST['tid'];
  $vid = $_REQUEST['vid'];

  _classification_lock_classification();

  $classification_settings = variable_get('classification_settings',array());

  $tree = classification_get_tree($vid,$tid);

  $parent = db_result(db_query("SELECT parent FROM {term_hierarchy} WHERE tid = %d", $tid));

  /* Relations are preserved here with the mod to taxonomy.module (See readme.txt)!! */
  $term = array(
    'tid' => $tid,
    'name' => $_REQUEST['name'],
    'parent' => 0,
    'vid' => CLASSIFICATION_DELETIONS,
    'old_vid' => $vid,
    'action_status' => 'deleting',
  );

  taxonomy_save_term($term);

  foreach ($tree as $branch) {

    /* Relations are preserved here with the mod to taxonomy.module (See readme.txt)!! */
    $child_term = array(
      'tid' => $branch->tid,
      'name' => $branch->name,
      'parent' => ($branch->parents[0]) ? $branch->parents[0] : $branch->parent,
      'vid' => CLASSIFICATION_DELETIONS,
      'old_vid' => $vid,
      'action_status' => 'deleting',
    );
    taxonomy_save_term($child_term);
  }

  $term = array(
    'tid' => $tid,
    'name' => $_REQUEST['name'],
    'parent' => 0,
    'vid' => CLASSIFICATION_DELETIONS,
    'old_vid' => $vid,
    'action_status' => 'deleted',
  );

  taxonomy_save_term($term);

  //log the action
  $action = array(
    'tid' => $tid,
    'parent' => $parent,
    'vid' => $vid
  );
  $message = t('!name and its children (if any) deleted', array('!name' => $term['name']));
  $classification_log = array(
    'uid' => $user->uid,
    'type' => 'deleted',
    'vid_to' => CLASSIFICATION_DELETIONS,
    'vid_from' => $vid,
    'description' => $message,
    'action' => $action,
    'created' => time(),
  );
  drupal_write_record('classification_log', $classification_log);

  _classification_unlock_classification();

  drupal_json(array('status' => 'deleted', 'delete_vid' => CLASSIFICATION_DELETIONS, 'message' => $message));

  classification_push_message('deleted', $action, $message);

}


/**
 * Callback to discover if a term (or its children) has any content
 */
function classification_delete_name_data($vid,$tid) {

  $txn = new Transaction();

  $name = taxonomy_get_term($tid);
  $name = $name->name;

  $name_content = '';

  $tree = classification_find_nodes($vid,$tid);

  if($tree) {
    foreach($tree as $branch) {
      $name_content[] = array(
        'name' => $branch->name,
        'tid' => $branch->tid,
      );
    }
  }

  $media = classification_term_count_media($tid);
  if(!empty($media)) {
    $taxon = taxonomy_get_term($tid);
    $name_content[] = array(
      'name' => $taxon->name,
      'tid' => $tid,
    );
  }

  if(!empty($name_content)) {
    $message = t('Content was found associated with <strong>!name</strong> or its children. Are you sure you want to remove these name(s)?', array('!name' => $name));
  }
  else {
    $message = t('No content was found associated with <strong>!name</strong> or any of if its children, if any. Are you sure you want to remove these name(s)?', array('!name' => $name));
  }

  drupal_json(array('status' => 'ok', 'names' => $name_content, 'message' => $message,));
}

/**
 * Callback to delete an alternate classification
 */
function classification_delete_classification() {

  global $user;

  $txn = new Transaction();

  $form = array(
    '#post' => $_POST,
  );

  $vid = $_REQUEST['vid'];

  _classification_lock_classification();

  $vocab = taxonomy_vocabulary_load($vid);

  if($vid == CLASSIFICATION_PRIMARY) {
    $status = 'failed';
    $output = t('Sorry, that classification cannot be deleted.');
  }
  else {
    $status = 'ok';
    taxonomy_del_vocabulary($vid);
    $output =t('%vocab was deleted.', array('%vocab' => $vocab->name));
    
    //log the action
    $classification_log = array(
      'uid' => $user->uid,
      'type' => 'deleted',
      'vid_to' => CLASSIFICATION_DELETIONS,
      'vid_from' => $vid,
      'description' => t('Classification !classification deleted', array('!classification' => $vocab->name)),
      'action' => array('vid' => $vid),
      'created' => time(),
    );
    drupal_write_record('classification_log', $classification_log);
  }

  _classification_unlock_classification();

  drupal_json(array('status' => $status, 'message' => $output));
}


/**
 * Callback to delete a name
 */
function classification_purge_deletions($vid='') {

  global $user;

  $txn = new Transaction();

  $form = array(
    '#post' => $_POST,
  );

  if (!$vid) {
    $vid = $_REQUEST['vid'];    
  }

  _classification_lock_classification();

  $qry = db_query("SELECT tid, name FROM {term_data} WHERE vid=%d",$vid);
  while ($result = db_fetch_array($qry)) {
    $media = classification_term_count_media($result['tid']);
    if(empty($media)) {
        $res = db_query("SELECT trid FROM {term_relation} WHERE tid1 = %d",$result['tid']);
        while ($term = db_fetch_object($res)) {
          db_query("DELETE FROM {term_relation_has_type} WHERE trid = %d", $term->trid);
        }
        //delete the term
        taxonomy_del_term($result['tid']);
    }
    else {
        $term_content = array(
          'tid' => $result['tid'],
          'name' => $result['name'],
          'parent' => 0,
          'vid' => $vid
        );
        taxonomy_save_term($term_content);
    }   
  }

  _classification_unlock_classification();

  //log the action
    $classification_log = array(
    'uid' => $user->uid,
    'type' => 'purged',
    'vid_to' => CLASSIFICATION_DELETIONS,
    'vid_from' => CLASSIFICATION_DELETIONS,
    'description' => t('Deleted names purged'),
    'action' => array('vid' => CLASSIFICATION_DELETIONS),
    'created' => time(),
  );
  drupal_write_record('classification_log', $classification_log);

  drupal_json(array('status' => 'edited', 'message' => t('Deleted names (except those with content) permanently deleted')));
}

/**
 * Callback to check the status of the db
 */
function classification_check_status() {
    
    if(variable_get('classification_status',1) == 1) {
      $status = 'ready';
      $message = '';
    }
    else {
      $status = 'busy';
      $message = t('Sorry, either your or someone else\'s previous action is still in progress. Please try again momentarily.');
    }
    drupal_json(array('status' => $status, 'message' => $message));
}

/**
 * Callback function to check that parent destination is still in the vocabulary
 */
function classification_check_destination($vid, $new_parent) {
    $term = taxonomy_get_term($new_parent);
    $status = ($term->vid != $vid) ? 'fail': 'ready';
    $message = ($term->vid != $vid) ? t('The destination where you intended to put your selection has been moved or deleted. Please refresh your tree.') : '';
    
    drupal_json(array('status' => $status, 'message' => $message ));
}

/**
 * Callback to move a name during drag/drop and during cut/paste
 */
function classification_move_name() {

  global $user;

  $txn = new Transaction();

  $form = array(
    '#post' => $_POST,
  );

  //set the status to busy
  _classification_lock_classification();

  $child = $_REQUEST['child'];
  $new_parent = $_REQUEST['new_parent'];
  $vid = $_REQUEST['vid'];

  //get the old parent and new parent for eventual logging purposes
  $old_parent_term = db_fetch_object(db_query("SELECT t.name,t.tid FROM {term_data} t INNER JOIN {term_hierarchy} h ON h.parent = t.tid WHERE h.tid = %d", $child));
  $old_parent_name = $old_parent_term->name ? $old_parent_term->name : 'root';

  $new_parent_term = taxonomy_get_term($new_parent);
  $child_term = taxonomy_get_term($child);

  //get the original vocab name for logging purposes later
  $vocab = taxonomy_vocabulary_load($child_term->vid);

  //let's first check to see if the term is being moved from another classification
  $children = false;
  if($vid != $vocab->vid) {
    //Call taxonomy_get_tree here. If it fails, nothing will be updated & PHP memory errors will be thrown
    $children = classification_get_tree($child_term->vid,$child);
  }

  $action_status = ($vid != $vocab->vid) ? "restoring" : "moving";

  $term = array(
    'tid' => $child,
    'name' => $child_term->name,
    'parent' => $new_parent,
    'old_parent' => $old_parent_term->tid,
    'vid' => $vid,
    'old_vid' => $vocab->vid,
    'action_status' => $action_status,
  );
  taxonomy_save_term($term);

  if($children) {
      foreach($children as $offspring) {
        $child_term_update = array(
          'tid' => $offspring->tid,
          'name' => $offspring->name,
          'parent' => ($offspring->parents[0]) ? $offspring->parents[0] : $offspring->parent,
          'vid' => $vid,
          'old_vid' => $vocab->vid,
          'action_status' => $action_status,
        );
        taxonomy_save_term($child_term_update);
      }
  }

  $action_status = ($action_status == "moving") ? "moved" : "restored";

  $term = array(
    'tid' => $child,
    'name' => $child_term->name,
    'parent' => $new_parent,
    'old_parent' => $new_parent,
    'vid' => $vid,
    'old_vid' => $vocab->vid,
    'action_status' => $action_status,
  );
  taxonomy_save_term($term);

  //log the activity
  $action = array();
  if($child_term->vid != $new_parent_term->vid) {
    $new_parent_name = $new_parent_term->name ? $new_parent_term->name : 'root';
    $message = t('!name and its children (if any) moved from !old_parent in !classification to !new_parent', array('!name' => $child_term->name, '!old_parent' => $old_parent_name, '!classification' => $vocab->name, '!new_parent' => $new_parent_term->name));
    $action[] = array(
      'vid' => $vocab->vid,
      'tid' => $child,
      'parent' => ($old_parent_term->tid) ? $old_parent_term->tid : 0,
    );
    $action[] = array(
      'vid' => $vid,
      'tid' => $child_term->tid,
      'parent' => ($new_parent) ? $new_parent : 0,
    );
    $classification_log = array(
      'uid' => $user->uid,
      'type' => 'moved',
      'vid_to' => $new_parent_term->vid,
      'vid_from' => $vocab->vid,
      'description' => $message,
      'action' => $action,
      'created' => time(),
    );
    drupal_write_record('classification_log', $classification_log);
  }
  else {
    $new_parent_name = $new_parent_term->name ? $new_parent_term->name : 'root';
    $message = t('!name and its children (if any) moved from !old_parent to !new_parent', array('!name' => $child_term->name, '!old_parent' => $old_parent_name, '!new_parent' => $new_parent_term->name));
    if($vid != classification_get_vid('Taxa')) {
      $message .= t(' in !vocab_name', array('!vocab_name' => $vocab->name));
    }
    $action[] = array(
        'vid' => $new_parent_term->vid,
        'tid' => $child_term->tid,
        'parent' => $old_parent_term->tid
    );
    $action[] = array(
        'vid' => $new_parent_term->vid,
        'tid' => $child_term->tid,
        'parent' => $new_parent_term->tid,
    );
    $classification_log = array(
      'uid' => $user->uid,
      'type' => 'moved',
      'vid_to' => $new_parent_term->vid,
      'vid_from' => $new_parent_term->vid,
      'description' => $message,
      'action' => $action,
      'created' => time(),
    );
    drupal_write_record('classification_log', $classification_log);
  }

  //set the status to ready
  _classification_unlock_classification();

  drupal_json(array('status' => 'moved', 'message' => t('!name and its children (if any) successfully moved', array('!name' => $child_term->name))));

  classification_push_message('moved', $action, $message);
}

/**
 * Callback to copy a name during drag/drop from source tree into destination tree
 */
function classification_copy_name() {

  global $user;

  $txn = new Transaction();

  $form = array(
    '#post' => $_POST,
  );

  //set the status to busy
  _classification_lock_classification();

  $child = $_REQUEST['child'];
  $parent = $_REQUEST['new_parent'];
  $vid = $_REQUEST['vid'];

  $child_term = taxonomy_get_term($child);
  $child_parent = db_result(db_query("SELECT parent FROM {term_hierarchy} WHERE tid=%d", $child));
  $child_vocab = db_fetch_object(db_query("SELECT v.name FROM {vocabulary} v INNER JOIN {term_data} td ON (v.vid=td.vid) WHERE td.tid=%d", $child));

  //produce the tree first because it will be intensive
  $child_tree = classification_get_tree($child_term->vid,$child);

  $parent_term = taxonomy_get_term($parent);
  $parent_name = ($parent == 0) ? 'root' : $parent_term->name;
  $vid = ($parent == 0) ? $vid : $parent_term->vid;

  $new_term = array(
    'name' => $child_term->name,
    'parent' => $parent,
    'vid' => $vid,
    'action_status' => 'copying',
  );

  taxonomy_save_term($new_term);

  $relations = classification_get_flags($child);

  foreach ($relations as $relation) {
    db_query("INSERT INTO {term_relation} (tid1,tid2) VALUES (%d,%d)",$new_term['tid'],$relation->tid);
    $trid = db_last_insert_id('term_relation','trid');
    db_query("INSERT INTO {term_relation_has_type} VALUES (%d,%d)",$trid,$relation->rtid);
  }

  // Now do something with all the kids
  if($child_tree) {
    $depth = array();
    $depth[0] = $new_term['tid'];

    foreach($child_tree as $children) {
        $new_parent = __classification_copy_name_create($children->tid, $children->name,$depth[$children->depth],$vid);
        $depth[$children->depth+1] = $new_parent;
    }
  }

  //log the activity
  $action = array();
  $message = t('!name and all its children (if any) copied from !classification and placed under !parent', array('!name' => $child_term->name, '!classification' => $child_vocab->name, '!parent' => $parent_name));
  $action[] = array(
    'tid' => $child,
    'vid' => $child_term->vid,
    'parent' => ($child_parent) ? $child_parent : 0,
  );
  $action[] = array(
    'tid' => $new_term['tid'],
    'vid' => $vid,
    'parent' => ($parent) ? $parent : 0,
  );
  $classification_log = array(
    'uid' => $user->uid,
    'type' => 'copied',
    'vid_to' => $parent_term->vid,
    'vid_from' => $child_term->vid,
    'description' => $message,
    'action' => $action,
    'created' => time(),
  );
  drupal_write_record('classification_log', $classification_log);

  $last_term = array(
    'tid' => $new_term['tid'],
    'name' =>$new_term['name'],
    'parent' => $parent,
    'vid' => $vid,
    'action_status' => 'copied',
  );

  taxonomy_save_term($last_term);

  //set the status to ready
  _classification_unlock_classification();

  drupal_json(array('status' => 'copied', 'message' => t('!name and its children (if any) successfully copied into your classification', array('!name' => $child_term->name))));

  classification_push_message('copied', $action, $message);
}

/*
 * Helper function to copy children of a name during drag/drop from source tree into destination tree
*/
function __classification_copy_name_create($tid,$name,$parent,$vid) {

  //build the new term
  $term = array(
    'name' => $name,
    'vid' => $vid,
    'parent' => $parent,
    'action_status' => 'copying',
  );

  //save the new term
  taxonomy_save_term($term);

  $relations = classification_get_flags($tid);

  foreach ($relations as $relation) {
    db_query("INSERT INTO {term_relation} (tid1,tid2) VALUES (%d,%d)",$term['tid'],$relation->tid);
    $trid = db_last_insert_id('term_relation','trid');
    db_query("INSERT INTO {term_relation_has_type} VALUES (%d,%d)",$trid,$relation->rtid);
  }

  return $term['tid'];
}

function classification_push_message($status, $action, $message) {
  if (!extension_loaded("redis")) {
    watchdog('classification', t("redis.so PHP extension is not loaded"), array(), WATCHDOG_ERROR); 
    exit;
  }

  global $user;
  $editor = "<em>" . $user->givenname . " " . $user->surname . "</em>";

  $channel = variable_get("juggernaut_channel", "site.lifedesks.org");
  $jug_message = array(
    'channels' => array($channel),
    'data' => array(
      'status' => $status,
      'actions' => array_key_exists(0, $action) ? $action : array($action),
      'message' => $editor . ": " . $message,
      'sid' => $user->sid
    )
  );

  $redis_host = variable_get("redis_host", "127.0.0.1");
  $redis_port = variable_get("redis_port", 6379);

  try {
    $redis = new Redis;
    $redis->connect(''.$redis_host.'', $redis_port);
    $redis->publish("juggernaut", json_encode($jug_message));
    $redis->close();
  } catch(RedisException $e) {
    watchdog('classification', t('Messages could not be pushed to the Juggernaut server'), array(), WATCHDOG_ERROR);
  }

}