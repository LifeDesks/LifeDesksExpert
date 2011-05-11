<?php
// $Id: both.inc,v 1.2.2.3 2008/12/03 18:43:45 nunoveloso18 Exp $

/**
 * @file
 * @brief Content management filter "both" operations file
 * 
 * This file contains all the "both" functions used by the module.
 *
 * @attention Copyright (C) 2008 Nuno Veloso <nunoveloso18 (at) gmail (dot) com>
 * @author Nuno Andr Veloso Matias (http://www.nunoveloso.org)
 *
 * @warning For more information on licensing, read the LICENCE.txt file.
 */


/**
 * Defines the form for mixed content administration filter results.
 *
 * @ingroup forms
 * @see cmf_admin_both_form_validate()
 * @see cmf_admin_both_form_submit()
 * 
 * @return array with forms properties 
 */
function cmf_admin_both_form() {
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
      '#prefix' => '<div class="container-inline">', '#suffix' => '</div>'
    );
    $form['options']['operation'] = array(
      '#type' => 'select', 
      '#options' => array(
                      'publish' => t('Publish'), 
                      'unpublish' => t('Unpublish'), 
                      'delete' => t('Delete'),
                    ), 
      '#default_value' => 'publish'
    );
    $form['options']['submit'] = array('#type' => 'submit', '#value' => t('Update'));
  }

  // load the objects that we want to display
  $form['header'] = array(
    '#type'  => 'value', 
    '#value' => cmf_build_header(),
  );
  

  $result = cmf_perform_query($form['header']['#value']);
  // build a table listing the appropriate objects
  while ($object = db_fetch_object($result)) {
    $taxon_id = db_fetch_object(db_query("SELECT tid FROM {term_node} WHERE nid = %d",$object->nid));
    $url = ($taxon_id->tid) ? 'pages/' . $taxon_id->tid : 'node/' . $object->nid;
    if(module_exists('classification')) {
        if($object->type == 'taxon_description' && classification_check_italicize($taxon_id->tid)) {
          $object->title = $taxon_name->italicized_form($object->title);
        }
    }
    $link = ($object->status || $user->uid == $object->uid || $user->uid == 1) ? l($object->title, $url, array('html' => TRUE)) : $object->title;
    if ($object->cid == 0) {
      $objects['n-'. $object->nid] = '';
      $form['kind']['n-'. $object->nid] = array('#value' => _cmf_get_img('node', t('node')));
      $form['title']['n-'. $object->nid] = array('#value' => $link .' '. theme('mark', node_mark($object->nid, $object->changed)));
      $form['type']['n-'. $object->nid] =  array('#value' => theme('cmf_type', $object->type));
      if (!(arg(0) == 'user' && is_numeric(arg(1)) && arg(1) > 0)) {
        $form['username']['n-'. $object->nid] = array('#value' => theme('cmf_user', $object->uid));
      }
      $form['status']['n-'. $object->nid] =  array('#value' =>  ($object->status ? '<span class="cmf_published">' . t('published') . '</span>' : '<span class="cmf_draft">' . t('draft') . '</span>'));
      $form['created']['n-'. $object->nid] = array('#value' => format_date($object->created, 
        'small'));
      if (user_access('filter and manage site content') || $user->uid == $object->uid) {
        switch($object->type) {
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
            case 'simplemappr':
              $cmf_edit_class = 'edit_simplemappr';
             break;
            case 'forum':
              $cmf_edit_class = 'edit_forum';
             break;
            case 'poll':
              $cmf_edit_class = 'edit_poll';
             break;
            default:
             $cmf_edit_class = 'edit_default';
        }
        $form['operations']['n-'. $object->nid] = array('#value' => '<ul class="cmf_links"><li class="' . $cmf_edit_class . '">' . l(t('edit'),'node/'. $object->nid .'/edit', array('html' => TRUE)) . '</li></ul>');
      }
    }
    else{
      $objects['c-'. $object->cid] = '';
      $form['kind']['c-'. $object->cid] = array('#value' => _cmf_get_img('comment', t('comment')));
      $form['title']['c-'. $object->cid] = array('#value' => l($object->title, 'pages/'. $url, 
        array('attributes' => array('title' => truncate_utf8($object->comment, 128)),
        'fragment' => 'comment-'. $object->cid)));
      $form['type']['c-'. $object->cid] = array('#value' => theme('cmf_type', $object->type));
      if (!(arg(0) == 'user' && is_numeric(arg(1)) && arg(1) > 0)) {
        $form['username']['c-'. $object->cid] = array('#value' => theme('cmf_user', $object->uid));
      }
      $form['status']['c-'. $object->cid] =  array('#value' =>  ($object->status ? 
        t('not published') : t('published')));
      $form['created']['c-'. $object->cid] = array('#value' => format_date($object->created, 
        'small'));
      if (user_access('filter and manage site content') || $user->uid == $object->uid) {
        switch($object->type) {
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
            case 'simplemappr':
              $cmf_edit_class = 'edit_simplemappr';
             break;
            case 'forum':
              $cmf_edit_class = 'edit_forum';
             break;
            case 'poll':
              $cmf_edit_class = 'edit_poll';
             break;
            default:
             $cmf_edit_class = 'edit_default';
        }
        $form['operations']['c-'. $object->cid] = array('#value' => '<ul class="cmf_links"><li class="' . $cmf_edit_class . '">' . l(t('edit'), 'comment/edit/'. $object->cid, array('html' => TRUE)) . '</li></ul>');
      }
    }
  }

  if (user_access('filter and manage site content')) {
    $form['objects'] = array('#type' => 'checkboxes', '#options' => $objects);
  }
  $form['pager'] = array('#value' => theme('pager', NULL, $_SESSION['cmf_max_rows'], 0));

  return $form;
}


/** 
 * Form validation before submit. \n
 * We can't execute any 'Update options' if no objects were selected.
 *
 * @ingroup forms
 * @see cmf_admin_both_form()
 * @see cmf_admin_both_form_submit()
 *
 * @param the ID of the passed form
 * @param array with the form properties values
 */ 
function cmf_admin_both_form_validate($form, &$form_state) {
  global $user;
  $form_state['values']['objects'] = array_diff($form_state['values']['objects'], array(0));
  if (count($form_state['values']['objects']) == 0) {
    form_set_error('', t('No items selected.'));
    drupal_goto('user/' . $user->uid . '/cmf');
  }
}


/** 
 * Handle post-validation form submission. \n
 * Execute the chosen 'Update option' on the selected objects, such as
 * publishing, unpublishing or deleting.
 *
 * @ingroup forms
 * @see cmf_admin_both_form()
 * @see cmf_admin_both_form_validate()
 *
 * @param the ID of the passed form
 * @param array with the form properties values
 */ 
function cmf_admin_both_form_submit($form, &$form_state) {
  
  // queries building
  switch ($form_state['values']['operation']) {
    case 'publish':
        $node_query    = 'UPDATE {node} SET status = 1 WHERE nid = %d';
        $comment_query = 'UPDATE {comments} SET status = '. COMMENT_PUBLISHED .' WHERE cid = %d';
      break;
    case 'unpublish':
        $node_query    = 'UPDATE {node} SET status = 0 WHERE nid = %d';
        $comment_query = 'UPDATE {comments} SET status = '. COMMENT_NOT_PUBLISHED .' WHERE cid = %d';
      break;
    case 'delete':
      break;
  }

  // perform queries
  foreach ($form_state['values']['objects'] as $flag) {
    if ($flag) {
      $object = explode('-', $flag);
      $kind   = $object[0];
      $value  = $object[1];
      
      if ($kind == 'n') {
        $node_type = db_fetch_object(db_query("SELECT type FROM {node} WHERE nid = %d",$value));
        if($node_type->type == 'taxon_description') {
            switch ($form_state['values']['operation']) {
                case 'publish':
                  db_query("UPDATE {taxon_description_status} SET status = 1 WHERE nid = %d",$value);
                 break;
                case 'unpublish':
                  db_query("UPDATE {taxon_description_status} SET status = 0 WHERE nid = %d",$value);
                 break;
                case 'delete':
                 break;
            }   
        }
        db_query($node_query, $value);
      }
      elseif ($kind == 'c') {
        // perform the update action, then refresh node statistics
        db_query($comment_query, $value);
        $comment = _comment_load($value);
        _comment_update_node_statistics($comment->nid);
        // Allow modules to respond to the updating of a comment.
        comment_invoke_comment($comment, $form_state['values']['operation']);

      }
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
function theme_cmf_admin_both_form($form) {
  global $user;
  $output = drupal_render($form['options']);

  if (isset($form['title']) && is_array($form['title'])) {
    foreach (element_children($form['title']) as $key) {
      $row = array();
      if (user_access('filter and manage site content')) {
        $row[] = drupal_render($form['objects'][$key]);
      }
      $row[] = drupal_render($form['kind'][$key]);
      $row[] = drupal_render($form['title'][$key]);
      $row[] = drupal_render($form['type'][$key]);
      if (!(arg(0) == 'user' && is_numeric(arg(1)) && arg(1) > 0)) {
        $row[] = drupal_render($form['username'][$key]);
      }
      $row[] = drupal_render($form['status'][$key]);
      $row[] = drupal_render($form['created'][$key]);
      if (user_access('filter and manage site content') || $user->uid) {
        $row[] = drupal_render($form['operations'][$key]);
      }
      $rows[] = $row;
    }
  }
  else {
    $rows[] = array(array('data' => t('Filter returned no results.'), 'colspan' => '7'));
  }

  $output .= theme('table', $form['header']['#value'], $rows);
  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $output .= drupal_render($form);

  return $output;
}
