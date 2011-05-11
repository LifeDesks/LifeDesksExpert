<?php

/**
 * Block page for default view
*/
function classification_block_page() {
  $item = menu_get_item();
  $content = system_admin_menu_block($item);
  $output = theme('classification_block_page', $content);
  return $output;
}

/*
 * Themed classification block page
*/
function theme_classification_block_page($content) {

  if (system_admin_compact_mode()) {
    $output .= '<ul class="menu">';
    foreach ($content as $item) {
      $output .= '<li class="leaf">'. l($item['title'], $item['href'], $item['localized_options']) .'</li>';
    }
    $output .= '</ul>';
  }
  else {
    $output .= '<dl class="admin-list">';
    foreach ($content as $item) {
      $output .= '<dt>'. l($item['title'], $item['href'], $item['localized_options']) .'</dt>';
      $output .= '<dd>'. $item['description'] .'</dd>';
    }
    $output .= '</dl>';
  }

  return $output;
}

/**
 * Autocomplete callback for node types that require it
 */
function classification_autocomplete($vid,$string='') {

  $regexp = '%(?:^|,\ *)("(?>[^"]*)(?>""[^"]* )*"|(?: [^",]*))%x';
  preg_match_all($regexp, $string, $matches);
  $array = $matches[1];

  $last_string = trim(array_pop($array));
  $matches = array();
  $classification_autocomplete = variable_get('classification_autocomplete_size',3);
  if (strlen($last_string) >= $classification_autocomplete ) {
    //join to the term_hierarchy table merely to ensure that this particular type of ghost term is not chosen
    $result = db_query_range(db_rewrite_sql("SELECT t.name FROM {term_data} t INNER JOIN {term_hierarchy} th ON (t.tid = th.tid) WHERE t.vid = %d AND LOWER(t.name) LIKE LOWER('%%%s%%')", 't', 'tid'), $vid, $last_string, 0, 10);

    while ($tag = db_fetch_object($result)) {
      $n = $tag->name;
      $matches[$n] = check_plain($tag->name);
    }
  }
  classification_to_json($matches);
} 

/**
 * Autocomplete callback for node types that require it
 */
function classification_autocomplete_vernaculars($vid,$string='') {

  $regexp = '%(?:^|,\ *)("(?>[^"]*)(?>""[^"]* )*"|(?: [^",]*))%x';
  preg_match_all($regexp, $string, $matches);
  $array = $matches[1];

  $last_string = trim(array_pop($array));
  $matches = array();
  if (strlen($last_string) >= 2 ) {
    $result = db_query_range(db_rewrite_sql("SELECT t.name FROM {term_data} t WHERE t.vid = %d AND LOWER(t.name) LIKE LOWER('%%%s%%')", 't', 'tid'), $vid, $last_string, 0, 10);

    while ($tag = db_fetch_object($result)) {
	  //strip the ISO code from the language
	  $lang_array = explode(",",$tag->name);
      $n = $tag->name;
      $matches[$lang_array[1]] = check_plain($tag->name);
    }
  }
  classification_to_json($matches);
}

function classification_to_json($var = NULL) {
  drupal_set_header('Content-Type: text/javascript; charset=utf-8');
  if (isset($var)) {
    echo classification_default_to_js($var);
  }
  else {
  	drupal_json(array('status' => TRUE,'data' => ''));
  }
}

function classification_default_to_js($var) {
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
          $output[] = classification_default_to_js($v);
        }
        return '[ '. implode(', ', $output) .' ]';
      }
      // Otherwise, fall through to convert the array as an object.
    case 'object':
      $output = array();
      foreach ($var as $k => $v) {
        $output[] = classification_default_to_js(strval($k)) .': '. classification_default_to_js($v);
      }
      return '{ '. implode(', ', $output) .' }';
    default:
      return 'null';
  }
}

/**
 * Callback to check if term actually exists and if there are multiple parents
 */
 function classification_checkterm() {
	
  $form = array(
    '#post' => $_POST,
  );
  $vid = $_REQUEST['vid'];
  $string = $_REQUEST['name'];

  $output = array();
  $result = db_query("SELECT td.tid, td.name as name FROM {term_data} td WHERE td.vid = %d AND td.name = '%s'", $vid, $string);

  while($names = db_fetch_object($result)) {
	$parents = taxonomy_get_parents_all($names->tid);
	$parent_array = array();
	if(count($parents)>1){
      $parents = array_reverse($parents);
      array_pop($parents);
      foreach($parents as $parent){
        $parent_array[] = check_plain($parent->name);
      }
    }
    $output[] = array(
	  'tid' => $names->tid,
	  'name' => check_plain($names->name),
	  'ancestry' => implode(" &raquo; ",$parent_array)
	);
  }

  switch (count($output)) {
    case 0:
      $message = '<strong><em>' . $string . '</em></strong> ' . t('was not found in your classification.');
    break;

    case 1:
      $message = t('We have just the one hit');
    break;

    default:
     $message = t('More than one name was found in your classification for ' . $string);	
  }
  
  drupal_json(array('status' => TRUE,'message' => $message, 'data' => $output));
}

/**
 * Callback to check if rank or vernacular (or any other term) in that vocabulary actually exists.
 * In this case, we don't care if there are multiple parents.
 */
 function classification_checkterm_simple() {
	
  $form = array(
    '#post' => $_POST,
  );
  $vid = $_REQUEST['vid'];
  $term = $_REQUEST['term'];
  $type = $_REQUEST['type'];

  switch($type) {
    case 'vernacular':
      $result = db_result(db_query("SELECT td.tid FROM {term_data} td WHERE td.vid=%d AND td.name LIKE '%,%s'",$vid,$term));
    break;
    default:
      $result = db_result(db_query("SELECT td.tid FROM {term_data} td WHERE td.vid = %d AND td.name = '%s'", $vid,$term));
  }

  $vocab = taxonomy_vocabulary_load($vid);

  if($result) {
    $data = true;
  }
  else {
	$data = false;
    $message = '<strong><em>' . $term . '</em></strong> ' . t('was not found in your list of') . ' ' . $vocab->name . '.';	
  }
  
  drupal_json(array('status' => TRUE,'message' => $message, 'data' => $data));
}

//accepts a comma-separated value of tids and produces a JSON output of equivalent names
function classification_buildnames() {
  
  $form = array(
    '#post' => $_POST,
  );
  $tids = $_REQUEST['tids'];
  
  $output = array();
  $tids = explode(',',$tids);
  foreach($tids as $tid) {
    $result = db_query("SELECT td.name FROM {term_data} td WHERE td.tid=%d AND td.vid=%d",$tid, CLASSIFICATION_PRIMARY);
    while($name = db_fetch_object($result)) {
	  $output[] = array(
	    'tid' => $tid,
	    'name' => $name->name
	  );
    }	
  }
  drupal_json(array('status' => TRUE, 'data' => $output));
}

/**
 * Callback to add a new name
 */
function classification_add_term() {

  $form_state = array('submitted' => FALSE);
  $form = array(
    '#post' => $_POST,
  );

  $vid = $_REQUEST['vid'];
  $name = $_REQUEST['name'];

  $term = array(
	'vid' => $vid,
	'name' => $name,
	'parent' => 0,
  );  

  taxonomy_save_term($term);

  drupal_json(array('status' => 'added', 'message' => t('Term successfully added')));
 }