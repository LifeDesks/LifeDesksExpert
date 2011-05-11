<?php

/**
 * chapter config page for adjusting chapters
 */
function taxon_description_config_page($arg) {  
  return theme('taxon_description_config', TAXON_DESCRIPTION_CHAPTERS, $arg);
}

/**
 * Get chapter callback function
 */
function taxon_description_get_chapter() {

  $form = array(
    '#post' => $_POST,
  );

  $tid = $_REQUEST['tid']; 

  $form_state = array('submitted' => FALSE);

  $output = '';
  $tids_qry = db_query("SELECT tid FROM {term_data} WHERE trid=%d AND vid=%d", $tid, TAXON_DESCRIPTION_CHAPTERS);

  $result = array();
  while ($tids = db_fetch_object($tids_qry)) {
    $qry = db_query("SELECT td.*,th.parent FROM {term_data} td INNER JOIN {term_hierarchy} th ON (td.tid=th.tid) WHERE td.tid=%d AND td.vid=%d",$tids->tid,TAXON_DESCRIPTION_CHAPTERS);
    $tid_data = db_fetch_array($qry);
    $result[$tid_data['language']] = $tid_data;  
  }

  drupal_json(array('status' => 'ok', 'data' => $result));
}

/**
 * Add chapter callback
 */
function taxon_description_add_chapter() {

  $form = array(
    '#post' => $_POST,
  );
  
  $description= $_REQUEST['taxon-description'];  
 
  $form_state = array('submitted' => FALSE);
  
  if($description) {
    $new_term = array();
    $new_tids = array();
    // Create new term(s)
    foreach($description as $desc) {
      if($desc['name']) {
        $term = array(
          'vid' => TAXON_DESCRIPTION_CHAPTERS,
          'name' => $desc['name'],
          'parent' => $desc['parent'],
          'description' => $desc['description'],
        );
        taxonomy_save_term($term);
        db_query("UPDATE {term_data} SET language = '%s' WHERE tid = %d", $desc['language'], $term['tid']);
        if($desc['language'] == 'en') {
          $new_term = $term;
        }
        $new_tids[] = $term['tid'];
        $export_file = file_scan_directory(file_create_path('taxon_export'), '.gz');
      }
    }
    foreach($new_tids as $new_tid) {
      db_query("UPDATE {term_data} SET trid = %d WHERE tid = %d", $new_term['tid'], $new_tid);
    }
    drupal_json(array('status' => 'added', 'term' => $new_term, 'message' => t('Chapter successfully added.'), 'fade' => 'true'));
  }
  else {
    drupal_json(array('status' => 'failed', 'message' => t('Failed to add the Chapter.')));
  }
}

/**
 * Update chapter callback
 */
function taxon_description_update_chapter() {

  $form = array(
    '#post' => $_POST,
  );

  $description = $_REQUEST['taxon-description'];
  $eng_trid = $description['en']['trid'];
  $eng_parent = $description['en']['parent'];
  $vid = $description['en']['vid'];

  if($description) {
    foreach($description as $lang => $desc) {
      //language might not have a parent declared in some circumstances, so we have to find it using English parent
      if(!$desc['parent']) {

            $desc['vid'] = $vid;

            $lang_parent = db_result(db_query("
                SELECT td.tid
                FROM term_data td
                INNER JOIN term_hierarchy th ON (th.parent = td.trid)
                INNER JOIN term_data td2 ON (td2.tid = th.tid)
                WHERE td.language = '%s' AND td2.trid = %d", $lang, $eng_trid));
            
            $desc['parent'] = ($lang_parent) ? $lang_parent : 0;
            
            taxonomy_save_term($desc);
            
            //now add the missing language and the trid
            db_query("UPDATE {term_data} SET language = '%s', trid = %d WHERE tid = %d", $lang, $eng_trid, $desc['tid']);   
      }
      elseif($desc['name']) {
        taxonomy_save_term($desc);
      }
      elseif($desc['tid']) {
        taxonomy_del_term($desc['tid']);
      }
      else {}
    }
  }

  drupal_json(array('status' => 'updated', 'message' => t('Chapter successfully updated.')));
} 

/**
 * Check if a chapter has associated content for a confirmation dialog
 */
function taxon_description_check_chapter_content() {

  $form = array(
    '#post' => $_POST,
  );

  $deltid = $_REQUEST['tid'];
  $chapter = $_REQUEST['name'];

  $form_state = array('submitted' => FALSE);

  //let's see if this is a main chapter, whose parent is 0
  $parent = db_result(db_query("SELECT parent FROM {term_hierarchy} WHERE tid=%d",$deltid));

  $content = '';
  $message = '';

  if($deltid && $parent != 0) {
    $content = taxon_description_check_chapter_content_per_language($deltid);
    if($content) {
      $message = '<strong>' . t('WARNING') . '</strong> ' . t('There is already some content associated with the chapter(s): ') . '<strong>' . $content . '</strong><br><br>';
    }
  }
  elseif ($deltid && $parent == 0) {
    $chapter_contents = array();
    $tree = taxonomy_get_tree(TAXON_DESCRIPTION_CHAPTERS,$deltid);
    foreach($tree as $branch) {
      $chapter_content = taxon_description_check_chapter_content_per_language($branch->tid);
          if ($chapter_content) {
            $chapter_contents[] = $chapter_content;
          }
      $content = implode(", ", $chapter_contents);
    }
    
    if($content) {
      $message = '<strong>' . t('WARNING') . '</strong> ' . t('There is already some content associated with the chapter(s): ') . '<strong>' . $content . '</strong><br><br>';
    }
  }
  
  drupal_json(array('status' => 'warning', 'message' => $message . t('Are you sure you would like to delete') . ' <strong>' . $chapter . '</strong>?'));
  
}

/*
 * Helper function for taxon_description_check_chapter_content()
 * to check chapter content per language.
 */
function taxon_description_check_chapter_content_per_language($tid) {
  $sub_content = array();
  $qry = db_query("SELECT tid, name, language FROM {term_data} WHERE trid = %d", $tid);
  while ($result = db_fetch_object($qry)) {
    $language = $result->language;
    $chapter_name = $result->name;
    $lang_chapter_tid = $result->tid;
    $chapter_content = chapter_content_exists($lang_chapter_tid, $language);
    if($chapter_content) {
      $sub_content[] = $chapter_name;
    }
  }
  $content = ($sub_content) ? implode(", ", $sub_content) : "";
  return $content;
}

function chapter_content_exists($lang_chapter_tid, $language) {
  $chapter_content = db_result(db_query("SELECT td.tid_chapter FROM {taxon_description} td INNER JOIN {node} n ON (n.nid=td.nid) WHERE td.tid_chapter=%d AND n.language = '%s' LIMIT 1",$lang_chapter_tid, $language));
  return $chapter_content;
}

/**
 * Delete chapter callback
 */
function taxon_description_delete_chapter() {
   
  $form_state = array('submitted' => FALSE);

  $form = array(
    '#post' => $_POST,
  );
  $deltid = $_REQUEST['tid'];

  if($deltid) {
    $qry = db_query("SELECT tid, language FROM {term_data} WHERE trid = %d", $deltid);
    while ($result = db_fetch_object($qry)) {
      $tid = $result->tid;
      $language = $result->language;

      if($tid) {
        // Delete term if it's a Heading and put Chapters into orphaned vocab if they have content
        $parent = db_result(db_query("SELECT parent FROM {term_hierarchy} WHERE tid = %d", $tid));
        if($parent == 0) {
          $children = taxonomy_get_children($tid, TAXON_DESCRIPTION_CHAPTERS);
          foreach($children as $child) {
            $child_result = chapter_content_exists($child->tid, $language);
            if($child_result) {
              db_query("UPDATE {term_data} SET vid = %d WHERE tid = %d", TAXON_DESCRIPTION_ORPHANED, $child->tid);
              db_query("UPDATE {term_hierarchy} SET parent = 0 WHERE tid = %d", $child->tid);
            }
            else {
              taxonomy_del_term($child->tid);
            }  
          }
          taxonomy_del_term($tid);
        }
        // Put Chapter into orphaned vocab if it has content
        else {
          $result = chapter_content_exists($tid, $language);
          if($result) {
            db_query("UPDATE {term_data} SET vid = %d WHERE tid = %d", TAXON_DESCRIPTION_ORPHANED, $tid);
            db_query("UPDATE {term_hierarchy} SET parent = 0 WHERE tid = %d", $tid);
          }
        }
        db_query("DELETE FROM {taxon_export_map} WHERE tid = %d", $tid);
      }
    
    }
    drupal_json(array('status' => 'deleted', 'message' => t('Chapter successfully deleted.'), 'fade' => 'true'));
   
 }

}

/**
 * Update chapter order callback
 */
function taxon_description_reorder_chapter() {

  $form = array(
    '#post' => $_POST,
  );
  
  $action = $_REQUEST['action'];
  $order = $_REQUEST['order'];
  
  $form_state = array('submitted' => FALSE);

  $replace = array('tid[]=');
  $replaceby = '';
  $order = str_replace($replace, $replaceby, $order);
  $updateRecordsArray = explode("&", $order);
  $tree = taxon_description_taxonomy_get_tree(TAXON_DESCRIPTION_CHAPTERS, 'en');
  if(in_array('0',$updateRecordsArray)){
    $updateRecordsArray = array_diff($updateRecordsArray, array('0') );
    foreach ($tree as $i){
      $tid = $i->tid;
      $name = $i->name;
      $description = $i->description;
      $parent = $i->parents[0];
      if($parent == 0) { 
        $t[$tid]['name']=$name;
        $t[$tid]['description']=$description;
      }
    }

    if ($action == "updateRecordsListings"){    
      $listingCounter = 1;
      foreach ($updateRecordsArray as $recordIDValue) {
        $term = array(
          'vid' => TAXON_DESCRIPTION_CHAPTERS,
          'tid' => $recordIDValue,
          'name' => $t[$recordIDValue]['name'],
          'weight' => $listingCounter,
          'parent' => 0,
          'description' => $t[$recordIDValue]['description'],
        );
        taxonomy_save_term($term);
        db_query("UPDATE {term_data} SET weight = %d WHERE trid = %d", $term['weight'], $term['tid']);
        $listingCounter = $listingCounter + 1;
      }
    }
  } else {
    foreach ($tree as $i){
      $tid = $i->tid;
      $name = $i->name;
      $description = $i->description;
      $parent = $i->parents[0];
      if($parent == 0) { 
        $p[]=$tid;
      } else {
        $t[$tid]['name']=$name;
        $t[$tid]['description']=$description;
      }
    }
    if ($action == "updateRecordsListings"){    
      $listingCounter = 1;
      $new_prnt = array();
      foreach ($updateRecordsArray as $recordIDValue) {
        if(in_array($recordIDValue,$p)){
          $prnt = $recordIDValue;
          $qry = db_query("SELECT tid, language FROM {term_data} WHERE trid = %d", $prnt);
          while($result = db_fetch_array($qry)) {
            $new_prnt[$result['language']] = $result['tid'];
          }
        } else {
          $term = array(
            'vid' => TAXON_DESCRIPTION_CHAPTERS,
            'tid' => $recordIDValue,
            'name' => $t[$recordIDValue]['name'],
            'weight' => $listingCounter,
            'parent' => $prnt,
            'description' => $t[$recordIDValue]['description'],
          );
          taxonomy_save_term($term);

          db_query("UPDATE {term_data} SET weight = %d WHERE trid = %d", $term['weight'], $term['tid']);
          
          $qry = db_query("SELECT tid, language, trid FROM {term_data} WHERE trid = %d", $term['tid']);
          while($result = db_fetch_array($qry)) {
            db_query("UPDATE term_hierarchy SET parent = %d WHERE tid = %d", $new_prnt[$result['language']], $result['tid']);
          }

          $listingCounter = $listingCounter + 1;
        }
      }
    }
  }
  drupal_json(array('status' => 'updated', 'message' => t("Updated all chapters."), "fade" => 'true'));
}