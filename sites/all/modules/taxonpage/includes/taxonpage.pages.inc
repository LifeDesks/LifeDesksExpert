<?php
// $Id: taxonpage.pages.inc,v 1.9 2008/01/18 16:23:57 dshorthouse Exp $

/**
 * @file
 * Page callbacks for the taxonpage module.
 */

/**
 * Menu callback; displays all nodes associated with a term.
 */
function taxonpage_term_page($tid = '', $op = 'page', $options = '', $uid = '') {

    global $NAMES;

    $term = taxonomy_get_term($tid);
    
    //make sure we are using the correct vid
    if($term->vid != CLASSIFICATION_PRIMARY || $tid == '') {
      drupal_not_found();
      return;
    }

    $result = db_fetch_object(db_query("SELECT t.name FROM {term_data} t WHERE t.tid = %d",$tid));
    $name = $result->name;

    if ($name) {
      switch ($op) {
        case 'page':
          //reconcilition by HTTP 302
          $result = db_fetch_object(db_query("
            SELECT th.parent
            FROM {term_data} td 
            INNER JOIN {term_hierarchy} th ON (td.tid=th.tid)
            INNER JOIN {term_relation} tr ON ( td.tid = tr.tid1 )
            INNER JOIN {term_data} td2 ON ( tr.tid2 = td2.tid )
            WHERE td.tid = %d
            AND td2.vid = %d", $tid, CLASSIFICATION_RELATIONS));
          if($result) {
            $query = NULL;
            if(isset($_GET['lang'])) $query = "lang=" . $_GET['lang'];
            drupal_goto("pages/" . $result->parent, $query);
            return;
          }

          $title = check_plain($name);
          $canonical = $NAMES->canonical_form($name);
        
          $dc_output  = '<meta name="dc:title" content="' . $title . '" />' . "\n";
          $dc_output .= '<meta name="dc:subject" content="' . t('Species page for !taxon', array('!taxon' => $title )) . '" />' . "\n";
          $dc_output .= '<meta name="dc.format" scheme="IMT" content="text/html" />' . "\n";
          $dc_output .= '<meta name="dc.type.documentType" content="Web Page" />' . "\n";
        
          drupal_set_html_head($dc_output);
        
          drupal_set_title($canonical);
        
          global $user,$NAMES,$languages;

          $name = classification_check_italicize($tid) ? $NAMES->italicized_form($name) : $name;
          $ancestry = theme('taxonpage_ancestry',taxonpage_ancestry_data($tid));
          $children = theme('taxonpage_children',taxonpage_children_data($tid));
          $classification_biblio = theme('taxonpage_classification_biblio',taxonpage_classification_biblio($tid));
          $languages = taxonpage_languages($tid);
          $taxonomy = theme('taxonpage_taxonomy',taxonpage_taxonomy_data($tid));
          $taxonomic_notes = theme('taxonpage_taxonomic_notes', $term->description, $term->vid, $term->tid);
          $vernaculars = theme('taxonpage_vernaculars',taxonpage_vernacular_data($tid));
          $flags = theme('taxonpage_flags',taxonpage_flag_data($tid));
          $biblio = theme('taxonpage_biblio', taxonpage_biblio_data($tid));
          $authors = taxonpage_authors_data($tid);

          if(!$options) {
            $images = theme('taxonpage_images', taxonpage_image_data($tid));
            $simplemappr = theme('taxonpage_simplemappr', taxonpage_simplemappr_data($tid));
            $chapters = theme('taxonpage_chapters',taxonpage_chapter_data($tid));
            $attachments = theme('taxonpage_attachments', taxonpage_attachment_data($tid));
            return theme('taxonpage_entire', $tid, CLASSIFICATION_PRIMARY, $name, $canonical, $ancestry, $children, $images, $simplemappr, $chapters, $taxonomy, $taxonomic_notes, $vernaculars, $flags, $biblio, $classification_biblio, $attachments, $authors, $languages);
          }

          switch($options) {
            case 'print':
              $images_print = theme('taxonpage_images_print', taxonpage_image_data($tid));
              $simplemappr_print = theme('taxonpage_simplemappr_print', taxonpage_simplemappr_data($tid));
              $chapters = theme('taxonpage_chapters',taxonpage_chapter_data($tid));
              $html = theme('taxonpage_print', $tid, $name, $ancestry, $children, $images_print, $simplemappr_print, $chapters, $taxonomy, $taxonomic_notes, $vernaculars, $flags,  $biblio, $classification_biblio, $authors);
              $pattern = '!<(a\s[^>]*?)>!is';
              $html = preg_replace_callback($pattern, '_taxonpage_rewrite_urls', $html);
              return $html;
            break;
    
            case 'pdf':
              global $base_url;
              $GLOBALS['conf']['cache'] = FALSE;

              module_load_include('php', 'taxonpage', 'pdf/dompdf_config.inc');
              $path = getcwd() . '/' . file_create_path();

              $filename = strip_tags(str_replace(" ","_",strtolower($NAMES->canonical_form($name)))) . '.pdf';
              $images_print = theme('taxonpage_images_print', taxonpage_image_data($tid));
              $simplemappr_print = theme('taxonpage_simplemappr_print', taxonpage_simplemappr_data($tid));
              $chapters = theme('taxonpage_chapters',taxonpage_chapter_data($tid));
              $html = theme('taxonpage_pdf', $tid, $name, $ancestry, $children, $images_print, $simplemappr_print, $chapters, $taxonomy, $taxonomic_notes, $vernaculars, $flags, $biblio, $classification_biblio, $authors);

              //convert the image URLs
              $pattern = '!<(img\s[^>]*?)>!is';
              $html = preg_replace_callback($pattern, '_taxonpage_rewrite_urls', $html);

              //convert the links
              $pattern = '!<(a\s[^>]*?)>!is';
              $html = preg_replace_callback($pattern, '_taxonpage_rewrite_urls', $html);

              $dompdf = new DOMPDF();
              $dompdf->set_base_path($path);
              $dompdf->set_paper('letter', 'portrait');

              // REPLACE UTF-8 CHARS
              $html = utf8_char_replace($html);
              
              if (function_exists('utf8_decode')) {
                $html = utf8_decode($html);
              }
              elseif (function_exists('mb_convert_encoding')) {
                $html = mb_convert_encoding($html, 'ISO-8859-1', 'UTF-8');
              }
              elseif (function_exists('recode_string')) {
                $html = recode_string('UTF-8..ISO_8859-1', $html);
              }
              $html = htmlspecialchars_decode(htmlentities($html, ENT_NOQUOTES, 'ISO-8859-1'), ENT_NOQUOTES);
              $html = preg_replace('!<tbody[^>]*?>|</tbody>!i', '', $html);

              $dompdf->load_html($html);
            
              $dompdf->render();
              $dompdf->stream($filename,array('compress' => 0));

            break;

            case 'user':
              if($authors) {
               $profile = user_load(array('uid' => $uid));
               if(!$profile || $uid == 0) return drupal_not_found();
               $images_user = theme('taxonpage_images', taxonpage_image_data($tid,$uid));
               $simplemappr_user = theme('taxonpage_simplemappr', taxonpage_simplemappr_data($tid,$uid));         
               $chapters_user = theme('taxonpage_chapters',taxonpage_chapter_data($tid,$uid));
               $html = theme('taxonpage_user', $tid, CLASSIFICATION_PRIMARY, $profile, $name, $ancestry, $children,  $images_user, $simplemappr_user, $chapters_user, $taxonomy, $taxonomic_notes, $vernaculars, $flags, $biblio, $classification_biblio, $authors, $languages);
               return $html;
              }
              else {
                drupal_not_found();
              }
            break;

            default:
              drupal_not_found();
          }
          break;
        case 'feed':

// would be cool to have a feed but we don't yet know how to produce something useful :)~

          break;
        case 'rebuild':
          break;
        default:
          drupal_not_found();
      }
    }
    else {
      drupal_not_found();
    }
}

/**
 * Ancestry data
 */
function taxonpage_ancestry_data($tid) {
  global $NAMES;

  $classification_settings = variable_get('classification_settings',array());

  $current->tid = $tid;
  $ancestry = array();
  while ($parents = taxonomy_get_parents($current->tid)) {
     $current = array_shift($parents);
     $rank = classification_get_rank($current->tid);
     $name = classification_check_italicize($current->tid) ? $NAMES->italicized_form($current->name) : $current->name;
     $ancestry[] = array(
       'tid' => $current->tid,
       'vid' => CLASSIFICATION_PRIMARY,
       'link' => l($name, 'pages/'. $current->tid, array('html' => true)),
       'rank' => $rank[$current->tid]['rank'],
      );
  }
  $ancestry = array_reverse($ancestry);

  return $ancestry;
}

/**
 * Children data
 */
function taxonpage_children_data($tid) {
  global $NAMES;
  $taxa = array();
  $children = classification_get_valid_children($tid, CLASSIFICATION_PRIMARY);
  foreach($children as $child) {
	$rank = classification_get_rank($child->tid);
	$name = classification_check_italicize($child->tid) ? $NAMES->italicized_form($child->name) : $child->name;
    $taxa['taxa'][] = array(
	  'tid' => $child->tid,
	  'vid' => $child->vid,
      'name' => $name,
      'link' => l($name, 'pages/'. $child->tid, array('html' => true))
    );
  }
  $taxa['total'] = count($taxa['taxa']);
  return $taxa;
}

/**
 * Image data
*/
function taxonpage_image_data($tid,$uid='') {
  global $base_path, $user;

  if(user_access('access comments')) {
   $comment_link = true;
  }

  if(!$uid) {
      $query = db_query("
        SELECT
          il.credit,
          il.rights, 
          il.exemplar, 
          n.uid,
          n.nid,
          n.title,
          n.created,
          n.status,
          lang.language,
          td.tid,
          td.name,
          cl.license 
        FROM
          {image_lifedesk} il
        INNER JOIN 
          {image} i ON (i.nid = il.nid) 
        INNER JOIN 
          {node} n ON (i.nid = n.nid) 
        INNER JOIN 
          {term_node} cn ON (n.nid = cn.nid) 
        INNER JOIN 
          {term_hierarchy} th ON (cn.tid = th.tid) 
        INNER JOIN 
          {term_data} td ON (th.tid = td.tid)
        LEFT JOIN 
          {term_relation} tr ON (cn.tid=tr.tid1)
        INNER JOIN 
          {languages} lang ON (n.language=lang.language)
        INNER JOIN
          {creativecommons_lite} cl ON (cl.nid=n.nid)
        WHERE 
          (cn.tid = %d OR th.parent = %d) AND i.image_size = '_original'", $tid,$tid);  
  }
  else {
      $query = db_query("
        SELECT
          il.credit,
          il.rights, 
          il.exemplar, 
          n.uid,
          n.nid,
          n.title, 
          n.status,
          n.created,
          lang.language,
          td.tid,
          td.name,
          cl.license 
        FROM 
          {image_lifedesk} il
        INNER JOIN
          {image} i ON (il.nid = i.nid) 
        INNER JOIN 
          {node} n ON (i.nid = n.nid) 
        INNER JOIN 
          {term_node} cn ON (n.nid = cn.nid) 
        INNER JOIN 
          {term_hierarchy} th ON (cn.tid = th.tid) 
        INNER JOIN 
          {term_data} td ON (th.tid = td.tid)
        LEFT JOIN 
          {term_relation} tr ON (cn.tid=tr.tid1)
        INNER JOIN 
          {languages} lang ON (n.language=lang.language)
        INNER JOIN 
          {creativecommons_lite} cl ON (cl.nid = n.nid)
        WHERE 
          (cn.tid = %d OR th.parent = %d) AND i.image_size = '_original' AND f.uid = %d", $tid,$tid,$uid);  
  }

  $images = array();
  
  $cc_module_path = drupal_get_path('module', 'creativecommons_lite') .'/';

  while($result = db_fetch_object($query)) {
    $author = user_load($result->uid);
    $credit = $result->credit;
    $rights = $result->rights;
    $cclicense = l('<img src="' . base_path() . $cc_module_path . 'images/buttons_small/' . $result->license . '.png">', 'http://creativecommons.org/licenses/'.$result->license.'/3.0/', array('html' => true, 'attributes' => array('style' => 'width:80px;height:15px', 'title' => "Creative Commons: " . $result->license, 'alt' => "Creative Commons: " . $result->license, 'rel' => 'nofollow')));
 
    $exemplar = ($result->exemplar) ? ' class="active"' : '';
    $perms = (user_access('administer nodes') || (user_access('edit own images') && $request->uid == $user->uid)) ? true : false;

    $original = user_access('view original images') ? '/image/view/' . $result->nid . '/_original' : '';

    if($result->status == 1) {
      $images[$result->nid] = array(
        'node' => $result->nid,
        'original' => $original,
        'preview' => base_path() . 'image/view/' . $result->nid,
        'thumbnail' => base_path() . 'image/view/' . $result->nid . '/thumbnail',
        'author' => (variable_get('taxonpage_show_license',1) == 1) ? l($author->surname .', '.$author->givenname, 'user/' . $author->uid) : "",
        'credit' => addslashes($credit),
        'rights' => addslashes($rights),
        'license' => (variable_get('taxonpage_show_license',1) == 1) ? $result->license : "",
        'cclicense' => (variable_get('taxonpage_show_license',1) == 1) ? $cclicense : "",
        'title' => addslashes($result->title),
        'exemplar' => $exemplar,
        'created' => $result->created,
        'status' => $result->status,
        'language' => $result->language,
        'permission' => $perms,
        'taxon_tid' => $result->tid,
        'taxon_name' => $result->name,
        'comment' => $comment_link,
        'comment_count' => $comment_link ? comment_num_all($result->nid) : 0,
      );
    }
    else {
      if($perms) {
          $images[$result->nid] = array(
            'node' => $result->nid,
            'original' => $original,
            'preview' => base_path() . 'image/view/' . $result->nid,
            'thumbnail' => base_path() . 'image/view/' . $result->nid . '/thumbnail',
            'author' => (variable_get('taxonpage_show_license',1) == 1) ? l($author->surname .', '.$author->givenname, 'user/' . $author->uid) : "",
            'credit' => addslashes($credit),
            'rights' => addslashes($rights),
            'license' => (variable_get('taxonpage_show_license',1) == 1) ? $result->license : "",
            'cclicense' => (variable_get('taxonpage_show_license',1) == 1) ? $cclicense : "",
            'title' => addslashes($result->title),
            'exemplar' => $exemplar,
            'created' => $result->created,
            'status' => $result->status,
            'language' => $result->language,
            'permission' => $perms,
            'taxon_tid' => $result->tid,
            'taxon_name' => $result->name,
            'comment' => $comment_link,
            'comment_count' => $comment_link ? comment_num_all($result->nid) : 0,
          );    
      }
    }
  }

  $exemplar_image = array();
  $time_created = array();
  foreach($images as $key => $row) {
      $exemplar_image[$key] = ($row['exemplar']) ? 1 : 0; 
      $time_created[$key] = $row['created'];
  }

  array_multisort($exemplar_image, SORT_DESC, $time_created, SORT_ASC, $images);

  return $images;
}

/**
 * Shaded maps HTML
 */
function taxonpage_simplemappr_data($tid,$uid='') {
  global $base_path, $user;

  if(user_access('access comments')) {
   $comment_link = true;
  }

  if(!$uid) {
      $query = db_query("
        SELECT 
          n.uid,
          n.nid,
          n.title,
          n.created,
          n.status,
          lang.language,
          cl.license
        FROM 
          {simplemappr} sm 
        INNER JOIN 
          {node} n ON (sm.nid = n.nid) 
        INNER JOIN 
          {term_node} tn ON (n.nid = tn.nid)  
        INNER JOIN 
          {languages} lang ON (n.language=lang.language)
        INNER JOIN 
          {creativecommons_lite} cl ON (cl.nid = n.nid)
        WHERE 
          tn.tid = %d AND sm.map_size = '_original'", $tid);    
  }
  else {
      $query = db_query("
        SELECT
          n.uid,
          n.nid,
          n.title, 
          n.status,
          n.created,
          lang.language,
          cl.license 
        FROM 
          {simplemappr} sm 
        INNER JOIN 
          {node} n ON (sm.nid = n.nid) 
        INNER JOIN 
          {term_node} tn ON (n.nid = cn.nid)
        INNER JOIN 
          {languages} lang ON (n.language=lang.language) 
        INNER JOIN 
          {creativecommons_lite} cl ON (cl.nid = n.nid)
        WHERE 
          tn.tid = %d AND sm.map_size = '_original' AND n.uid = %d", $tid,$uid);    
  }

  $simplemappr = array();

  $cc_module_path = drupal_get_path('module', 'creativecommons_lite') .'/';

  while($result = db_fetch_object($query)) {
    $author = user_load($result->uid);
    $perms = (user_access('administer nodes') || (user_access('edit own simplemap') && $request->uid == $user->uid)) ? true : false;
    $cclicense = l('<img src="' . base_path() . $cc_module_path . 'images/buttons_small/' . $result->license . '.png">', 'http://creativecommons.org/licenses/'.$result->license.'/3.0/', array('html' => true, 'attributes' => array('style' => 'width:80px;height:15px', 'title' => "Creative Commons: " . $result->license, 'alt' => "Creative Commons: " . $result->license, 'rel' => 'nofollow')));

    $original = user_access('view original simplemap') ? '/simplemappr/view/' . $result->nid . '/_original' : '';

    if($result->status == 1) {
      $simplemappr[$result->nid] = array(
        'node' => $result->nid,
        'original' => $original,
        'preview' => base_path() . 'simplemappr/view/' . $result->nid,
        'thumbnail' => base_path() . 'simplemappr/view/' . $result->nid . '/thumbnail',
        'author' => l($author->surname .', '.$author->givenname, 'user/' . $author->uid),
        'title' => $result->title,
        'cclicense' => $cclicense,
        'created' => $result->created,
        'status' => $result->status,
        'language' => $result->language,
        'permission' => $perms,
        'comment' => $comment_link,
        'comment_count' => $comment_link ? comment_num_all($result->nid) : 0,
      );
    }
    else {
      if($perms) {
          $simplemappr[$result->nid] = array(
            'node' => $result->nid,
            'original' => $original,
            'preview' => base_path() . 'simplemappr/view/' . $result->nid,
            'thumbnail' => base_path() . 'simplemappr/view/' . $result->nid . '/thumbnail',
            'author' => l($author->surname .', '.$author->givenname, 'user/' . $author->uid),
            'title' => $result->title,
            'cclicense' => $cclicense,
            'created' => $result->created,
            'status' => $result->status,
            'language' => $result->language,
            'permission' => $perms,
            'comment' => $comment_link,
            'comment_count' => $comment_link ? comment_num_all($result->nid) : 0,
          );    
      }
    }
  }

  return $simplemappr;
}


/**
 * taxon_description HTML
 */
function taxonpage_chapter_data($tid_taxon,$uid='') {

  global $languages;

  $lang = (isset($_GET['lang']) && array_key_exists($_GET['lang'], $languages)) ? $_GET['lang'] : 'en';

  $tree = taxon_description_taxonomy_get_tree(TAXON_DESCRIPTION_CHAPTERS, $lang);

  foreach ($tree as $i){
    $name = $i->name; 
    $tid_chapter = $i->tid;
    $parent_tid = $i->parents[0];
    if($parent_tid) {
      $parent = taxonomy_get_term($parent_tid);
      $chapter[$parent->name][$name] = _taxonpage_get_chapter($tid_chapter,$tid_taxon,$uid);
      if(count($chapter[$parent->name][$name]) < 1) {
        unset($chapter[$parent->name][$name]);
        if(empty($chapter[$parent->name])) {
          unset($chapter[$parent->name]);
        }
      }
    }
    else {
      $chapter[$name]['term_en'] = $i->term_en;
    }
  }

  foreach($chapter as $key => $value) {
    if(count($value) == 1) unset($chapter[$key]);   
  }

  return $chapter;
}

function _taxonpage_get_chapter($tid_chapter,$tid_taxon,$uid='') {

  global $languages;

  $lang = (isset($_GET['lang']) && array_key_exists($_GET['lang'], $languages)) ? $_GET['lang'] : 'en';

  if(!$uid) {
      $query = db_query("
        SELECT 
          '' as tid,
          '' as name,
          tdesc.sid,
          tdesc.nid,
          u.uid as uid,
          n.status as pubstatus,
          cl.license as license,
          tdesc.body as body
        FROM 
         {users} u  
        INNER JOIN 
         {node} n ON (u.uid=n.uid) 
        INNER JOIN
         {taxon_description} tdesc ON (n.nid=tdesc.nid)
        INNER JOIN 
         {creativecommons_lite} cl ON (n.nid=cl.nid) 
        WHERE 
         n.language = '%s' AND tdesc.tid_chapter = %d AND tdesc.tid_taxon = %d
        UNION ALL
        SELECT 
          td.tid AS tid, 
          td.name AS name,
          tdesc.sid,
          tdesc.nid,
          u.uid AS uid, 
          n.status AS pubstatus,
          cl.license AS license,
          tdesc.body AS body
        FROM 
          {users} u
        INNER JOIN 
          {node} n ON (u.uid=n.uid)
        INNER JOIN 
          {taxon_description} tdesc ON (n.nid=tdesc.nid)
        INNER JOIN 
          {term_data} td ON (tdesc.tid_taxon=td.tid)
        INNER JOIN 
          {term_hierarchy} th ON (tdesc.tid_taxon=th.tid)
        INNER JOIN 
          {term_relation} tr ON (th.tid=tr.tid1)
        INNER JOIN 
          {term_data} td2 ON (tr.tid2=td2.tid)
        INNER JOIN 
          {creativecommons_lite} cl ON (n.nid=cl.nid)
        WHERE 
          n.language = '%s' AND tdesc.tid_chapter = %d AND th.parent = %d AND td2.vid = %d
        ", $lang,$tid_chapter,$tid_taxon,$lang,$tid_chapter,$tid_taxon,CLASSIFICATION_RELATIONS);   
  }

  else {
      $query = db_query("
        SELECT 
          '' as tid,
          '' as name,
          tdesc.sid,
          tdesc.nid,
          u.uid as uid,
          n.status as pubstatus,
          cl.license as license,
          tdesc.body as body
        FROM 
         {users} u  
        INNER JOIN 
         {node} n ON (u.uid=n.uid) 
        INNER JOIN
         {taxon_description} tdesc ON (n.nid=tdesc.nid)
        INNER JOIN 
         {creativecommons_lite} cl ON (n.nid=cl.nid) 
        WHERE 
         n.language = '%s' AND tdesc.tid_chapter = %d AND tdesc.tid_taxon = %d AND u.uid = %d
        UNION ALL
        SELECT 
          td.tid AS tid, 
          td.name AS name, 
          tdesc.sid,
          tdesc.nid,
          u.uid AS uid, 
          n.status AS pubstatus,
          cl.license AS license,
          tdesc.body AS body
        FROM 
          {users} u
        INNER JOIN 
          {node} n ON (u.uid=n.uid)
        INNER JOIN 
          {taxon_description} tdesc ON (n.nid=tdesc.nid)
        INNER JOIN 
          {term_data} td ON (tdesc.tid_taxon=td.tid)
        INNER JOIN 
          {term_hierarchy} th ON (tdesc.tid_taxon=th.tid)
        INNER JOIN 
          {term_relation} tr ON (th.tid=tr.tid1)
        INNER JOIN 
          {term_data} td2 ON (tr.tid2=td2.tid)
        INNER JOIN 
          {creativecommons_lite} cl ON (n.nid=cl.nid)
        WHERE 
          n.language ='%s' AND tdesc.tid_chapter = %d AND th.parent = %d AND td2.vid = %d AND u.uid = %d
        ",$lang,$tid_chapter,$tid_taxon,$uid,$lang,$tid_chapter,$tid_taxon,CLASSIFICATION_RELATIONS,$uid);
  }

  $module_path = drupal_get_path('module', 'creativecommons_lite') .'/';
  while($result = db_fetch_object($query)) {
    $cclicense = l('<img src="' . base_path() . $module_path . 'images/buttons_small/' . $result->license . '.png">', 'http://creativecommons.org/licenses/'.$result->license.'/3.0/', array('html' => true, 'attributes' => array('style' => 'width:80px;height:15px', 'title' => "Creative Commons: " . $result->license, 'alt' => "Creative Commons: " . $result->license, 'rel' => 'nofollow')));
    //get all the authors
    $authors = array();
    $authors_qry = db_query("
        SELECT 
          tdcd.drupal_uid, tdcd.name
        FROM 
          {taxon_description_contributor_data} tdcd 
        INNER JOIN 
          {taxon_description_contributor} tdc ON (tdcd.cntr_id = tdc.cntr_id)
        WHERE tdc.sid = %d
        ORDER BY
          tdc.rank", $result->sid);
    while($authors_result = db_fetch_object($authors_qry)) {
        $authors[] = ($authors_result->drupal_uid) ? l(t($authors_result->name), "user/".$authors_result->drupal_uid) : t($authors_result->name);
    }
    //get all the rights holders
    $rightsholders = array();
    if($result->license != 'publicdomain') {
      $rightsholders_qry = db_query("
          SELECT 
            tdrd.drupal_uid, tdrd.name
          FROM 
            {taxon_description_rightsholder_data} tdrd 
          INNER JOIN 
            {taxon_description_rightsholder} tdr ON (tdrd.rts_id = tdr.rts_id)
          WHERE tdr.sid = %d
          ORDER BY
            tdr.rank", $result->sid);
      while($rightsholders_result = db_fetch_object($rightsholders_qry)) {
          $rightsholders[] = ($rightsholders_result->drupal_uid) ? l(t($rightsholders_result->name), "user/".$rightsholders_result->drupal_uid) : t($rightsholders_result->name);
      }
    }
    $data[] = array(
      'taxon_tid' => $result->tid,
      'taxon_name' => $result->name,
      'nid' => $result->nid,
      'uid' => $result->uid,
      'status' => $result->pubstatus,
      'cc_license' => $result->license,
      'license_img' => $cclicense,
      'body' => $result->body,
      'authors' => implode('; ', $authors),
      'rightsholders' => implode('; ', $rightsholders),
      'comment_count' => comment_num_all($result->nid),
    );
  }

  return $data;
}

function _taxonpage_check_chapter($tid_chapter, $tid_taxon, $nid) {
    $content = db_result(db_query("SELECT body FROM {taxon_description} WHERE nid=%d AND tid_taxon=%d AND tid_chapter = %d", $nid, $tid_taxon, $tid_chapter));
    if(!$content) {
      return false;
    }
    return true;
}

/**
 *  Attachments
 */
function taxonpage_attachment_data($tid) {
  $attachments = array();
  $query = db_query("SELECT nid FROM {term_node} WHERE tid=%d", $tid);
  while ($result = db_fetch_object($query)) {
   $attachments = array_merge($attachments,_taxonpage_get_attachments($result->nid));
  }
  return $attachments;
}

/**
 * Taxonomy data
 */
function taxonpage_taxonomy_data($tid) {
  return classification_get_synonyms($tid);
}

/**
 * Vernacular data
 */
function taxonpage_vernacular_data($tid) {
  return classification_get_vernaculars($tid);
}

/**
 * Biological flag data
 */
function taxonpage_flag_data($tid) {
  $flags = classification_get_flags($tid);

  $vocabs = taxonomy_get_vocabularies();
  foreach($vocabs as $vocab) {
    if($vocab->name == 'Taxonomic Ranks' || $vocab->name == 'Taxonomic Relationships') {
      unset($flags[$vocab->vid]);
    }   
  }

  return $flags;
}

/**
 * Original description data
 */
function taxonpage_classification_biblio($tid) {

  //get the biblio style
  $style = biblio_get_style();

  $query = db_query("
    SELECT
      tn.nid
    FROM 
     {taxonomy_enhancer_value_nodereference} tn 
    WHERE 
      tn.tid = %d AND tn.fid = 'field_biblio_citation_original'",$tid);

  $biblio_classification = array();

  while ($result = db_fetch_object($query)) {
   $node = node_load($result->nid);
   $item = theme('taxonpage_biblio_entry',$node,'biblio',$style,false);

   $biblio_classification[$result->nid] = array(
     'clean' => strtolower(strip_tags($item)),
     'full' => $item,
   ); 
  }

  return $biblio_classification;
}

function utf8_to_ascii($string){
    // source code at http://us3.php.net/manual/en/function.iconv.php#93609
    
    $r = '';
    $s1 = @iconv('UTF-8', 'ASCII//TRANSLIT', $string);
    $j = 0;
    for ($i = 0; $i < strlen($s1); $i++) {
        $ch1 = $s1[$i];
        $ch2 = substr($string, $j++, 1);
        if (strstr('`^~\'"', $ch1) !== false) {
            if ($ch1 <> $ch2) {
                --$j;
                continue;
            }
        }
        $r .= ($ch1=='?') ? $ch2 : $ch1;
    }
    return $r;
}

function _build_attachments($icons) {
    global $module_path;
    foreach($icons as $icon) {
        switch($icon['filemime']) {
            case 'application/pdf':
              $class = 'pdf';
            break;
            case 'text/plain':
              $class = 'text';
            break;
            case 'image/gif':
            case 'image/jpeg':
            case 'image/png':
            case 'image/tiff':
              $class = 'image';
            break;
            case 'application/msword':
              $class = 'doc';
            break;
            case 'application/vnd.ms-excel':
              $class = 'xls';
            break;
        }
        $output .= '<li class="attached ' . $class . '"><a href="/' . $icon['filepath'] . '">' . $icon['filename'] . ' (' . ceil($icon['filesize']/1000) . ' kb)</a></li>';
    }
    return $output;
}

/**
 * Biblio data
*/
function taxonpage_biblio_data($tid) {
  //get biblio items for tid and for all its immediate children if tagged to vernacular or synonym

  global $user;

  //get the biblio style
  $style = biblio_get_style();

  $query = db_query("
     SELECT 
       n.nid as nid 
     FROM 
       {node} n 
     INNER JOIN
        {term_node} cn ON (n.nid=cn.nid)  
     INNER JOIN 
        {term_data} td ON (cn.tid=td.tid)
     INNER JOIN 
        {term_hierarchy} th ON (td.tid=th.tid) 
     LEFT JOIN 
        {term_relation} tr ON td.tid=tr.tid1 
     WHERE 
         td.vid = %d AND (th.parent = %d OR td.tid = %d) AND n.type = 'biblio'", CLASSIFICATION_PRIMARY, $tid, $tid); 

  $biblio = array();

  while ($result = db_fetch_object($query)) {
   $node = node_load($result->nid);
   $item = theme('taxonpage_biblio_entry',$node,'biblio',$style,false,biblio_access('update', $node, $user->uid));
   $biblio[$result->nid] = array(
     'clean' => strtolower(utf8_to_ascii(strip_tags($item))),
     'full' => $item
   );
  }

  $sort_name = array();

  foreach($biblio as $key => $row) {
    $sort_name[$key] = $row['clean'];
  }

  array_multisort($sort_name, SORT_ASC, $biblio);

  return $biblio;
}

function theme_taxonpage_biblio_entry($node, $base = 'biblio', $style = 'apa', $inline = false, $edit = false) {

  $output  = "\n".'<div class="biblio-entry">'."\n" ;

  // first add the styled entry...
  $output .= theme('biblio_style', $node, $base, $style, $inline);

  // now add the various links
  if ($node->biblio_abst_e) {
    $output .= '<span class="biblio-abstract-link">';
    $output .= l(" Abstract", "node/$node->nid") ."\n";
    $output .= '</span>';
  }
  $annotation_field = variable_get('biblio_annotations', 'none');
  if ($annotation_field != 'none' && $node-> $annotation_field) {
    $output .= '<div class="biblio-annotation">';
    $output .= check_markup($node->$annotation_field, $node->format, FALSE);
    $output .= '</div>';
  }
  $openurl_base = variable_get('biblio_baseopenurl', '');

  if ($openurl_base){
    $output .= theme('biblio_openurl', biblio_openurl($node));
  }

  if (biblio_access('export', $node)) {
    $base = variable_get('biblio_base', 'biblio');
    $output .= theme('taxonpage_biblio_export_links',$node);
  }

  if($edit) {
    $output .= " <span class=\"biblio-edit\">" . l(t('edit'), 'node/' . $node->nid . '/edit') . "</span>";  
  }

  $output .= "\n</div>";

  return $output;
}

function theme_taxonpage_biblio_export_links($node) {
  $attachments = _taxonpage_get_attachments($node->nid);
  if($attachments) {
    $output = '<div class="attachments" title="' . t('attachments') . '">&nbsp;</div><div class="attachments_popup""><ul>' . _build_attachments($attachments) . '</ul></div>';
  }
  return $output;   
}

function _taxonpage_get_attachments($nid) {
    $items = array();
    
    $result = db_query('
      SELECT 
        n.type,
        f.filepath, 
        f.filemime,
        f.filename,
        f.filesize,
        f.timestamp 
      FROM 
        {files} f 
      INNER JOIN 
        {upload} u ON (f.fid=u.fid) 
      INNER JOIN 
        {node} n ON (u.nid=n.nid) 
      WHERE 
        n.status=1 AND u.nid = %d',$nid);
    
    while ($attachment = db_fetch_object($result)) {
        $items[] = array(
          'type' => $attachment->type,
          'filepath' => $attachment->filepath,
          'filemime' => $attachment->filemime,
          'filename' => $attachment->filename,
          'filesize' => $attachment->filesize,
          'timestamp' => $attachment->timestamp
        );
    }
    
    return $items;
}

function _cmp($a, $b){
   return strcmp($a['surname'], $b['surname']);
}

/**
 * Return array of authors
 */
function taxonpage_authors_data($tid) {
    $query = db_query("
      SELECT DISTINCT 
       u.uid 
      FROM 
       {users} u 
      INNER JOIN 
        {node} n ON (n.uid=u.uid) 
      INNER JOIN 
        {term_node} cn ON (n.nid=cn.nid) 
      WHERE 
        cn.tid = %d and n.type='taxon_description'",$tid);
    
    while ($result = db_fetch_object($query)) {
      $authors[] = $result->uid;
    }
    
    if ($authors) 
        $authors = implode(",",$authors);
    
    return $authors;
}

/**
 * Return array of languages that have at least one taxon_description with content
 * TODO: will have to UNION ALL with $tid passed as parent in term_hierarchy table
 */
function taxonpage_languages($tid) {
    $langs = array();
    $query = db_query("
        SELECT 
          n.language, l.native 
        FROM 
          {node} n 
        INNER JOIN 
          {languages} l ON (n.language = l.language) 
        INNER JOIN 
           {term_node} tn ON (n.nid = tn.nid)
        INNER JOIN
           {taxon_description} tdesc ON (n.nid = tdesc.nid) 
        INNER JOIN 
           {term_data} td ON (tdesc.tid_chapter = td.tid)
        WHERE 
          n.type='taxon_description' AND tn.tid = %d AND td.vid = %d
        UNION ALL
        SELECT
          n.language, l.native
        FROM
          {node} n 
        INNER JOIN 
          {languages} l ON (n.language = l.language) 
        INNER JOIN 
          {term_node} tn ON (n.nid = tn.nid)
        INNER JOIN 
          {term_data} td1 ON (tn.tid = td1.tid)
        INNER JOIN 
          {term_hierarchy} th ON (td1.tid = th.tid)
        INNER JOIN 
          {term_relation} tr ON (td1.tid = tr.tid1)
        INNER JOIN 
          {term_data} td2 ON (tr.tid2 = td2.tid)
        INNER JOIN 
          {taxon_description} tdesc ON (n.nid = tdesc.nid) 
        INNER JOIN 
          {term_data} td3 ON (tdesc.tid_chapter = td3.tid)
        WHERE 
          n.type = 'taxon_description' AND th.parent = %d AND td2.vid = %d AND td3.vid = %d
        ", $tid, TAXON_DESCRIPTION_CHAPTERS, $tid, CLASSIFICATION_RELATIONS, TAXON_DESCRIPTION_CHAPTERS);
    while($result = db_fetch_array($query)) {
        $langs[$result['language']] = $result['native'];
    }

    return $langs;
}

/**
 * Return HTML of chapters without any content via menu callback
 */
function _taxonpage_empty_chapters($tid_taxon,$lang='en') {
  global $user;
  $tree = taxon_description_taxonomy_get_tree(TAXON_DESCRIPTION_CHAPTERS, $lang);
  $nid = db_fetch_object(db_query("SELECT n.nid as nid FROM {node} n INNER JOIN {term_node} cn ON (n.nid = cn.nid) WHERE type='taxon_description' AND cn.tid = %d AND n.uid = %d AND n.language='%s'", $tid_taxon, $user->uid, $lang));

  if(!$nid->nid) {
    $output = '<span class="taxonpage-tools taxonpage-add-description">' . l(t('Add Description'), 'node/add/taxon-description/' . $tid_taxon) . '</span>';
  }
  else {
      $chapters['node'] = $nid->nid;

      foreach ($tree as $i){
        $name = $i->name; 
        $tid_chapter = $i->tid;
        $parent_tid = $i->parents[0];
        if($parent_tid) {
          $parent = taxonomy_get_term($parent_tid);
          $chapters_has_content = _taxonpage_check_chapter($tid_chapter, $tid_taxon, $nid->nid);
          if(!$chapters_has_content) {
            $chapters['chapters'][$parent->name][$tid_chapter] = $name;
          }
        }  
      }

      $output = '<label for="taxonpage_chapter_select">' . t('Select Chapter:') . '</label>' . "\n";
      $output .= '<select id="taxonpage_chapter_select" name="taxonpage_chapter_select">' . "\n";
      $output .= '<optgroup label="">' . "\n";
      $output .= '<option value=""> --- ' . t('Select') . ' --- </option>' . "\n";

      foreach($chapters['chapters'] as $parent => $chapter) {
        $output .= '<optgroup label="' . htmlspecialchars($parent) . '">';
        foreach ($chapter as $value => $empty_chapter) {
            $output .= '<option value="' . $chapters['node'] . ':' . htmlspecialchars($empty_chapter) . '">' . htmlspecialchars($empty_chapter) . '</option>';
        }
        $output .= '</optgroup>';
      }

      $output .= '</select>';   
  }

  echo $output;
}

function _taxonpage_rewrite_urls($matches) {
  global $base_url, $base_root, $_print_urls;

  // first, split the html into the different tag attributes
  $pattern = '!\s*(\w+\s*=\s*"(?:\\\"|[^"])*")\s*|\s*(\w+\s*=\s*\'(?:\\\\\'|[^\'])*\')\s*|\s*(\w+\s*=\s*\w+)\s*|\s+!';
  $attribs = preg_split($pattern, $matches[1], -1, PREG_SPLIT_NO_EMPTY | PREG_SPLIT_DELIM_CAPTURE);
  foreach ($attribs as $key => $value) {
    $attribs[$key] = preg_replace('!(\w)\s*=\s*(.*)!', '$1=$2', $value);
  }

  $size = count($attribs);
  for ($i=1; $i < $size; $i++) {
    // If the attribute is href or src, we may need to rewrite the URL in the value
    if (preg_match('!^(?:href|src)\s*?=(.*)!i', $attribs[$i], $urls) > 0) { 
      $url = trim($urls[1], " \t\n\r\0\x0B\"'");

      if (strpos($url, '://') || preg_match('!^mailto:.*?@.*?\..*?$!iu', $url)) {
        // URL is absolute, do nothing
        $newurl = str_replace(" ","%20",urldecode($url));
      }
      else {
        if ($url[0] == '#') {
          // URL is an anchor tag
          if (!empty($_print_urls)) {
            $path = explode('/', $_GET['q']);
            unset($path[0]);
            $path = implode('/', $path);
            if (is_numeric($path)) {
              $path = "node/$path";
            }
            // Printer-friendly URLs is on, so we need to make it absolute
            $newurl = url($path, array('fragment' => substr(urlencode($url), 1), 'absolute' => TRUE));
          }
          // Because base href is the original page, change the link to
          // still be usable inside the print page
          $matches[1] = str_replace($url, $_GET['q'] . $url, $matches[1]);
        }
        else {
          // URL is relative, convert it into absolute URL
          if ($url[0] == '/') {
            // If it starts with '/' just append it to the server name
            $newurl = $base_root .'/'. trim(urldecode($url), '/');
          }
          elseif (preg_match('!^(?:index.php)?\?q=!i', $url)) {
            // If it starts with ?q=, just prepend with the base URL
            $newurl = $base_url .'/'. trim(urlencode($url), '/');
          }
          else {
            $newurl = url(trim(urldecode($url), '/'), array('absolute' => TRUE));
          }
          $matches[1] = str_replace($url, str_replace(" ","%20",$newurl), $matches[1]);
        }
      }
    }
  }

  $ret = '<'. $matches[1] .'>';
  if (count($matches) == 4) {
    $ret .= $matches[2] . $matches[3];
    if ((!empty($_print_urls)) && (isset($newurl))) {
      $ret .= ' <span class="print-footnote">['. _print_friendly_urls(trim(stripslashes($newurl))) .']</span>';
    }
  }

  return $ret;
}