<?php
// $Id: taxon_description.settings.inc,v 1.8.2.1 2008/10/16 12:43:08 goba Exp $
        
/**
 * Form builder to display a listing of all editable Chapters.
 *
 * @ingroup forms
 * @see ocmf_chapters_form_submit()
 */
function ocmf_chapters_form(&$form_state,$arg1,$arg2,$arg3) {

$taxon_tid = db_result(db_query("SELECT tid_taxon FROM {taxon_description} WHERE sid = %d",$arg2));

$taxon_data = taxonomy_get_term($taxon_tid);

$taxon_header = '<h1>' . $taxon_data->name . '</h1>';

$form['taxon_name'] = array(
   '#type' => 'markup',
   '#value' => $taxon_header,
);
  
  $chapters = array();
  $chapters[" "]["choose"] = t(" --- Choose --- ");
  $tree = taxon_description_taxonomy_get_tree(TAXON_DESCRIPTION_CHAPTERS,$arg3);
  foreach ($tree as $i){
    $tid = $i->tid;
    $name = $i->name;
    $weight = $i->weight;	
    $parent = $i->parents[0];
    if($parent == 0){
	$chapters[$name];
    }else{
	$parentname = taxonomy_get_term($parent)->name;
	$chapters[$parentname][$tid] = $name;
    }
  }

  $orphan = db_fetch_object(db_query("SELECT * FROM taxon_description WHERE sid = %d", $arg2));
  $orphan_chapter = taxonomy_get_term($orphan->tid_chapter);

    $form['old_chapter'] = array(
	   '#type' => 'textfield',
	   '#title' => t('Orphaned Chapter'),
	   '#value' => $orphan_chapter->name,
	   '#weight' => 0,
	   '#disabled' => TRUE,
	   '#attributes' => array('style' => 'color:red;font-weight:bold'),
	);
	$form['body'] = array(
	    '#type' => 'item',
	    '#tree' => TRUE,
	);
    $form['body']['content'] = array(
        '#type' => 'textarea',
        '#title' => t('Orphaned Chapter Content'),
        '#required' => TRUE,
        '#default_value' => $orphan->body,
        '#weight' => 1,
        '#parents' => array('body', 'content'),
    );
    $form['tid_chapter'] = array(
        '#type' => 'select',
        '#title' => t('New Chapter'),
        '#required' => TRUE,
        '#options' => $chapters,
        '#weight' => 2,
    );	
    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Move'),
        '#weight' => 10,
    );
	$form['#ocmf_edit_form_param'] = $orphan;
	$form['cancel'] = array(
        '#type' => 'submit',
        '#value' => t('Cancel'),
        '#weight' => 10,
		'#submit' => array('ocmf_chapter_form_cancel'),
    );

    $form['body']['format'] = filter_form(FILTER_FORMAT_DEFAULT, NULL, array('body', 'format'));

    return $form;
}

/**
 * Submit handler to update changed chapter weights and parents.
 *
 * @see ocmf_edit_form()
 */ 
function ocmf_chapters_form_submit($form, &$form_state) {		
  if($form_state['values']['tid_chapter'] != "choose"){
    $body = db_result(db_query("SELECT body FROM  {taxon_description} WHERE tid_chapter = %d AND nid = %d", $form_state['values']['tid_chapter'], $form['#ocmf_edit_form_param']->nid));
    if($body){
      $body = $body."<br /><br />".$form_state['values']['body']['content'];
      db_query("UPDATE {taxon_description} SET body = '%s' WHERE tid_chapter = %d AND nid = %d", $body, $form_state['values']['tid_chapter'], $form['#ocmf_edit_form_param']->nid);
      db_query("DELETE FROM {taxon_description} WHERE sid = %d", $form['#ocmf_edit_form_param']->sid);
	  drupal_set_message(t("Content appended to existing content for that chapter."));
      drupal_goto('user/'.$form['#parameters'][2]->uid.'/ocmf');
    } else {
      db_query("UPDATE {taxon_description} SET body = '%s', tid_chapter = %d WHERE sid = %d", $form_state['values']['body']['content'], $form_state['values']['tid_chapter'], $form['#ocmf_edit_form_param']->sid);
      drupal_set_message(t("Content moved successfully"));
      drupal_goto('user/'.$form['#parameters'][2]->uid.'/ocmf/chapters');
    }
  } else {
    form_set_error('tid_chapter',t('Please choose a New Chapter.'));
  }
}

function ocmf_chapters_form_cancel($form, &$form_state) {  
	drupal_goto('user/'.$form['#parameters'][2]->uid.'/ocmf/chapters');
}

/**
 * @see ocmf_admin_chapters_form_submit()
 * 
 * @return array with forms properties 
 */
function ocmf_chapters_admin_form($form, &$form_state) {

  global $user;
  $module_path = drupal_get_path('module', 'ocmf') .'/';
  drupal_add_css($module_path .'css/ocmf.css');
  $destination = drupal_get_destination();

  $form['header'] = array(
    '#type'  => 'value', 
    '#value' => ocmf_chapters_build_header(),
  );
  
  $qry = ocmf_chapters_perform_query($form['header']['#value']);

  while ($result = db_fetch_object($qry)) {
  	$taxon = db_result(db_query("SELECT name FROM {term_data} WHERE tid = %d",$result->tid_taxon));
    $form['title'][$result->sid] = array('#value' => $taxon);
    $form['chapter'][$result->sid] = array('#value' => $result->chapter);
    $form['language'][$result->sid] =  array('#value' => $result->language);
	$form['content'][$result->sid] = array('#value' => $result->body);
    $form['operations'][$result->sid] = array('#value' => '<ul class="ocmf_links"><li class="edit_taxon_page">' . l(t('reassign'),'user/'.$result->uid.'/ocmf/chapters/'. $result->sid .'/edit/'. $result->language, array('html' => TRUE)) . '</li></ul>');
  }

  $form['pager'] = array('#value' => theme('pager', NULL, $_SESSION['ocmf_chapters_max_rows'], 0));


  return $form;
}

/** 
 * Theme results table.
 *
 * @ingroup themable
 *
 * @return table with filter results
 */ 
function theme_ocmf_chapters_admin_form($form) {
  $output = drupal_render($form['options']);
  if (isset($form['title']) && is_array($form['title'])) {
    foreach (element_children($form['title']) as $key) {
      $row = array();
	  $row[] = drupal_render($form['title'][$key]);
      $row[] = drupal_render($form['chapter'][$key]);
      $row[] = drupal_render($form['language'][$key]);
	  $row[] = drupal_render($form['content'][$key]);
      $row[] = drupal_render($form['operations'][$key]);
      $rows[] = $row;
    }
  }
  else {
    $rows[] = array(array('data' => t('Filter returned no results.'), 'colspan' => '5'));
  }

  $output .= theme('table', $form['header']['#value'], $rows, array());
  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $output .= drupal_render($form);

  return $output;
}


