<?php

function ocmf_taxa_form(&$form_state,$arg1,$arg2,$arg3) {
	$form = array();
	return $form;
}

function ocmf_taxa_form_submit($form, &$form_state) {
	
}

function ocmf_taxa_form_cancel($form, &$form_state) {  
	drupal_goto('user/'.$form['#parameters'][2]->uid.'/ocmf/taxa');
}

function ocmf_taxa_admin_form() {

  $module_path = drupal_get_path('module', 'ocmf') .'/';
  drupal_add_css($module_path .'css/ocmf.css');
  $destination = drupal_get_destination();

  $form = array();

  $form['options'] = array(
    '#type' => 'fieldset', '#title' => t('Update options'),
    '#prefix' => '<div class="container-inline">', '#suffix' => '</div>',
  );
  $options = array();
  $options['publish'] = 'Publish';
  $options['unpublish'] = 'Unpublish';
  $options['delete'] = 'Delete';

  $form['options']['operation'] = array(
    '#type' => 'select', 
    '#options' => $options,  
    '#default_value' => 'publish'
  );
  $form['options']['submit'] = array('#type' => 'submit', '#value' => t('Update'));

  $form['header'] = array(
    '#type'  => 'value', 
    '#value' => ocmf_taxa_build_header(),
  );

  $qry = ocmf_taxa_perform_query($form['header']['#value']);

  while ($result = db_fetch_object($qry)) {
	$nodes[$result->tid . '-' .$result->nid] = '';
	$form['taxon'][$result->tid . '-' . $result->nid] = array('#value' => $result->taxon);
    $form['title'][$result->tid . '-' . $result->nid] = array('#value' => $result->title);
    $form['type'][$result->tid . '-' . $result->nid] = array('#value' => theme('ocmf_type', $result->type));
    $form['language'][$result->tid . '-' . $result->nid] =  array('#value' => $result->language);
	$form['status'][$result->tid . '-' . $result->nid] = array('#value' =>  ($result->status ? '<span class="ocmf_published">' . t('published') . '</span>' : '<span class="ocmf_draft">' . t('unpublished') . '</span>'));
    $form['operations'][$result->tid . '-' . $result->nid] = array('#value' => '<ul class="ocmf_links"><li class="edit_' . $result->type . '">' . l(t('edit'),'node/'.$result->nid.'/edit/', array('html' => TRUE)) . '</li></ul>');
  }

  $form['nodes'] = array('#type' => 'checkboxes', '#options' => $nodes);
  $form['pager'] = array('#value' => theme('pager', NULL, $_SESSION['ocmf_taxa_max_rows'], 0));

  return $form;
}

/** 
 * Form validation before submit. \n
 * We can't execute any 'Update options' if no nodes were selected.
 *
 * @ingroup forms
 * @see cmf_taxa_admin_form()
 * @see cmf_taxa_admin_form_submit()
 *
 * @param the ID of the passed form
 * @param array with the form properties values
 */ 
function ocmf_taxa_admin_form_validate($form, &$form_state) {
  $nodes = array_filter($form_state['values']['nodes']);
  if (count($nodes) == 0) {
    form_set_error('', t('No items selected.'));
  }
}

/** 
 * Handle post-validation form submission. \n
 * Execute the chosen 'Update option' on the selected nodes, such as
 * publishing, unpublishing, promotion and stickness status or deleting.
 *
 * @ingroup forms
 * @see cmf_taxa_admin_form()
 * @see cmf_taxa_admin_form_validate()
 *
 * @param the ID of the passed form
 * @param array with the form properties values
 */ 
function ocmf_taxa_admin_form_submit($form, &$form_state) {

  // queries building
  switch ($form_state['values']['operation']) {
    case 'publish':
        $node_query    = 'UPDATE {node} SET status = 1 WHERE nid = %d';
      break;
    case 'unpublish':
        $node_query    = 'UPDATE {node} SET status = 0 WHERE nid = %d';
      break;
  }

  // perform queries
  foreach ($form_state['values']['nodes'] as $node) {
	$node = explode("-", $node);
	$node = $node[1];
	if($form_state['values']['operation'] == 'delete') {
		node_delete($node);
		db_query("DELETE FROM {term_node} WHERE nid = %d",$node);
	}
	else {
		db_query($node_query, $node);
	}
  }
  cache_clear_all();
  drupal_set_message(t('The update has been performed.'));

  if (arg(0) == 'user' && is_numeric(arg(1)) && arg(1) > 0) {
    $form_state['redirect'] = 'user/'. arg(1) .'/ocmf';
  }
  else {
    $form_state['redirect'] = '';
  }
}

/** 
 * Theme results table.
 *
 * @ingroup themable
 *
 * @return table with filter results
 */ 
function theme_ocmf_taxa_admin_form($form) {
  $output = drupal_render($form['options']);
  if (isset($form['title']) && is_array($form['title'])) {
    foreach (element_children($form['title']) as $key) {
      $row = array();
      $row[] = drupal_render($form['nodes'][$key]);
      $row[] = drupal_render($form['taxon'][$key]);
	  $row[] = drupal_render($form['title'][$key]);
      $row[] = drupal_render($form['type'][$key]);
      $row[] = drupal_render($form['language'][$key]);
	  $row[] = drupal_render($form['status'][$key]);
      $row[] = drupal_render($form['operations'][$key]);
      $rows[] = $row;
    }
  }
  else {
    $rows[] = array(array('data' => t('Filter returned no results.'), 'colspan' => '7'));
  }

  $output .= theme('table', $form['header']['#value'], $rows, array());
  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $output .= drupal_render($form);

  return $output;
}