<?php
// $Id: node.inc,v 1.2.2.3 2008/12/03 18:43:45 nunoveloso18 Exp $

/**
 * @file
 * @brief Content management filter node operations file
 * 
 * This file contains all the node functions used by the module.
 *
 * @attention Copyright (C) 2008 Nuno Veloso <nunoveloso18 (at) gmail (dot) com>
 * @author Nuno Andr� Veloso Matias (http://www.nunoveloso.org)
 *
 * @warning For more information on licensing, read the LICENCE.txt file.
 */


/**
 * Defines the form for nodes administration filter results.
 *
 * @ingroup forms
 * @see cmf_admin_nodes_form_validate()
 * @see cmf_admin_nodes_form_submit()
 * 
 * @return array with forms properties 
 */
function cmf_admin_nodes_form() {
  global $user;

  if(module_exists('classification')) {
	  module_load_include('php', 'classification', 'includes/classification_functions.class');
	  global $taxon_name;
	  $taxon_name = new Names;	
  }

  $module_path = drupal_get_path('module', 'cmf') .'/';
  drupal_add_css($module_path .'css/cmf.css');
  $destination = drupal_get_destination();

  // build an 'Update options' form
  if (user_access('filter and manage site content')) {
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
      '#default_value' => 'approve'
    );
    $form['options']['submit'] = array('#type' => 'submit', '#value' => t('Update'));
  }

  // load the nodes that we want to display
  $form['header'] = array(
    '#type'  => 'value', 
    '#value' => cmf_build_header(),
  );
  
  $result = cmf_perform_query($form['header']['#value']);
  // build a table listing the appropriate nodes
  while ($node = db_fetch_object($result)) {
    $nodes[$node->nid] = '';
  	$taxon_id = db_fetch_object(db_query("SELECT tid FROM {term_node} WHERE nid = %d",$node->nid));
  	$url = ($taxon_id->tid) ? 'pages/' . $taxon_id->tid : 'node/' . $node->nid;
    if(module_exists('classification')) {
	    if($node->type == 'taxon_description' && classification_check_italicize($taxon_id->tid)) {
		  $node->title = $taxon_name->italicized_form($node->title);
	    }
    }
  	$link = ($node->status || _cmf_edit_perms($node)) ? l($node->title, $url, array('html' => TRUE)) : $node->title;
    $form['title'][$node->nid] = array('#value' => $link .' '.
      theme('mark', node_mark($node->nid, $node->changed)));
    $form['kind'][$node->nid] = array('#value' => _cmf_get_img('node', t('node')));
    $form['type'][$node->nid] =  array('#value' => theme('cmf_type', $node->type));
    $form['language'][$node->nid] =  array('#value' => $node->language);
    if (!(arg(0) == 'user' && is_numeric(arg(1)) && arg(1) > 0)) {
      $form['username'][$node->nid] = array('#value' => theme('cmf_user', $node->uid));
    }
    $form['status'][$node->nid] =  array('#value' =>  ($node->status ? '<span class="cmf_published">' . t('published') . '</span>' : '<span class="cmf_draft">' . t('unpublished') . '</span>'));
    $form['created'][$node->nid] = array('#value' => format_date($node->created, 'small'));
    
    if (_cmf_edit_perms($node)) { 
    	 switch($node->type) {
			case 'page':
			  $cmf_edit_class = 'edit_page';
			 break;
      		case 'image':
      		  $cmf_edit_class = 'edit_image';
      		 break;
      		case 'taxon_description':
      		  $cmf_edit_class = 'edit_taxon_page';
      		 break;
      		case 'biblio':
      		  $cmf_edit_class = 'edit_biblio';
      		 break;
            case 'forum':
              $cmf_edit_class = 'edit_forum';
             break;
            case 'simplemappr':
              $cmf_edit_class = 'edit_simplemappr';
             break;
            case 'poll':
              $cmf_edit_class = 'edit_poll';
             break;
      		default:
      		 $cmf_edit_class = 'edit_default';
      }
      $form['operations'][$node->nid] = array('#value' => '<ul class="cmf_links"><li class="' . $cmf_edit_class . '">' . l(t('edit'),'node/'. $node->nid .'/edit', array('html' => TRUE)) . '</li></ul>');
    }
  }

  if (user_access('filter and manage site content')) {
    $form['nodes'] = array('#type' => 'checkboxes', '#options' => $nodes);
  }
  $form['pager'] = array('#value' => theme('pager', NULL, $_SESSION['cmf_max_rows'], 0));

  return $form;
}


/** 
 * Form validation before submit. \n
 * We can't execute any 'Update options' if no nodes were selected.
 *
 * @ingroup forms
 * @see cmf_admin_nodes_form()
 * @see cmf_admin_nodes_form_submit()
 *
 * @param the ID of the passed form
 * @param array with the form properties values
 */ 
function cmf_admin_nodes_form_validate($form, &$form_state) {
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
 * @see cmf_admin_nodes_form()
 * @see cmf_admin_nodes_form_validate()
 *
 * @param the ID of the passed form
 * @param array with the form properties values
 */ 
function cmf_admin_nodes_form_submit($form, &$form_state) {

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
	if($form_state['values']['operation'] == 'delete') {
		node_delete($node);
		db_query("DELETE FROM {term_node} WHERE nid = %d",$nid);
	}
	else {
		db_query($node_query, $node);
	}
  }
  cache_clear_all();
  drupal_set_message(t('The update has been performed.'));

  if (arg(0) == 'user' && is_numeric(arg(1)) && arg(1) > 0) {
    $form_state['redirect'] = 'user/'. arg(1) .'/cmf';
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
function theme_cmf_admin_nodes_form($form) {
  global $user;
  $output = drupal_render($form['options']);
  if (isset($form['title']) && is_array($form['title'])) {
    foreach (element_children($form['title']) as $key) {
      $row = array();
      if (user_access('filter and manage site content')) {
        $row[] = drupal_render($form['nodes'][$key]);
      }
      $row[] = drupal_render($form['kind'][$key]);
      $row[] = drupal_render($form['title'][$key]);
      $row[] = drupal_render($form['type'][$key]);
      $row[] = drupal_render($form['language'][$key]);
      if (!(arg(0) == 'user' && is_numeric(arg(1)) && arg(1) > 0)) {
        $row[] = drupal_render($form['username'][$key]);
      }
      $row[] = drupal_render($form['status'][$key]);
      $row[] = drupal_render($form['created'][$key]);
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
/**
 * Function added by lwalley LifeDesks
 * Check user permissions for editing node
 */
function _cmf_edit_perms($node) {
  global $user;
  if(user_access('administer nodes')) {
    return TRUE;
  } else { 
    switch ($node->type) {
      case 'biblio':
        return (user_access('edit all biblio entries') || (user_access('edit own biblio entries') && $node->uid == $user->uid));
        break;
      case 'image':
        return (user_access('edit any images') || (user_access('edit own images') && $node->uid == $user->uid));
        break;
      case 'page':
        return (user_access('edit any page content') || (user_access('edit own page content') && $node->uid == $user->uid));
        break;
      case 'story':
        return (user_access('edit any story content') || (user_access('edit own story content') && $node->uid == $user->uid));
        break;
      case 'taxon_description':
        return (user_access('edit any taxon description') || (user_access('edit own taxon description') && $node->uid == $user->uid));
        break;
      case 'simplemappr':
	    return (user_access('edit any simplemap') || (user_access('edit own simplemap') && $node->uid == $user->uid));
	    break;
    }
  }
}