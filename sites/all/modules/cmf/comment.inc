<?php
// $Id: comment.inc,v 1.2.2.3 2008/12/03 18:43:45 nunoveloso18 Exp $

/**
 * @file
 * @brief Content management filter comment operations file
 * 
 * This file contains all the comment functions used by the module.
 *
 * @attention Copyright (C) 2008 Nuno Veloso <nunoveloso18 (at) gmail (dot) com>
 * @author Nuno AndrŽ Veloso Matias (http://www.nunoveloso.org)
 *
 * @warning For more information on licensing, read the LICENCE.txt file.
 */


/**
 * Defines the form for comments administration filter results.
 *
 * @ingroup forms
 * @see cmf_admin_comments_form_validate()
 * @see cmf_admin_comments_form_submit()
 * 
 * @return array with forms properties 
 */
function cmf_admin_comments_form() {
  $module_path = drupal_get_path('module', 'cmf') .'/';
  drupal_add_css($module_path .'css/cmf.css');
  $destination = drupal_get_destination();

  // build an 'Update options' form
  if (user_access('filter and manage site content')) {
    $form['options'] = array(
      '#type' => 'fieldset', '#title' => t('Update options'),
      '#prefix' => '<div class="container-inline">', '#suffix' => '</div>'
    );
    $options = array();
    foreach (comment_operations() as $key => $value) {
      $options[$key] = $value[0];
    }
    $form['options']['operation'] = array(
      '#type' => 'select', 
      '#options' => $options, 
      '#default_value' => 'publish',
    );
    $form['options']['submit'] = array('#type' => 'submit', '#value' => t('Update'));
  }
  
    // load the comments that we want to display
    $form['header'] = array(
      '#type'  => 'value', 
      '#value' => cmf_build_header(),
    );

  $result = cmf_perform_query($form['header']['#value']);
  // build a table listing the appropriate comments
  while ($comment = db_fetch_object($result)) {
    $comments[$comment->cid] = '';
    $form['title'][$comment->cid] = array('#value' => l($comment->subject, 'node/'. $comment->nid, 
      array('attributes' => array('title' => truncate_utf8($comment->comment, 128)), 
      'fragment' => 'comment-'. $comment->cid)));
    $form['kind'][$comment->cid] = array('#value' => _cmf_get_img('comment', t('comment')));
    $form['type'][$comment->cid] = array('#value' => theme('cmf_type', $comment->type));
    if (!(arg(0) == 'user' && is_numeric(arg(1)) && arg(1) > 0)) {
      $form['username'][$comment->cid] = array('#value' => theme('cmf_user', $comment->uid));
    }
    $form['status'][$comment->cid] =  array('#value' =>  ($comment->status ? t('not published') : 
      t('published')));
    $form['created'][$comment->cid] = array('#value' => format_date($comment->created, 'small'));
    if (user_access('filter and manage site content') || $user->uid == $comment->uid) {
    	 switch($comment->type) {
			case 'page':
			  $cmf_edit_class = 'page';
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
      $form['operations'][$comment->cid] = array('#value' => '<ul class="cmf_links"><li class="' . $cmf_edit_class . '">' . l(t('edit'), 'comment/edit/'. $comment->cid, array('html' => TRUE)) . '</li></ul>');
    }
  }
  
  if (user_access('filter and manage site content')) {
    $form['comments'] = array('#type' => 'checkboxes', '#options' => $comments);
  }
  $form['pager'] = array('#value' => theme('pager', NULL, $_SESSION['cmf_max_rows'], 0));

  return $form;
}


/** 
 * Form validation before submit. \n
 * We can't execute any 'Update options' if no comments were selected.
 *
 * @ingroup forms
 * @see cmf_admin_comments_form()
 * @see cmf_admin_comments_form_submit()
 *
 * @param the ID of the passed form
 * @param array with the form properties values
 */ 
function cmf_admin_comments_form_validate($form, &$form_state) {
  $form_state['values']['comments'] = array_diff($form_state['values']['comments'], array(0));
  if (count($form_state['values']['comments']) == 0) {
    form_set_error('', t('No items selected.'));
  }
}


/** 
 * Handle post-validation form submission. \n
 * Execute the chosen 'Update option' on the selected comments, such as
 * publishing, unpublishing or deleting.
 *
 * @ingroup forms
 * @see cmf_admin_comments_form()
 * @see cmf_admin_comments_form_validate()
 *
 * @param the ID of the passed form
 * @param array with the form properties values
 */ 
function cmf_admin_comments_form_submit($form, &$form_state) {
  $operations = comment_operations();
  if ($operations[$form_state['values']['operation']][1]) {
    // extract the appropriate database query operation
    $query = $operations[$form_state['values']['operation']][1];
    foreach ($form_state['values']['comments'] as $cid => $value) {
      if ($value) {
        // perform the update action, then refresh node statistics
        db_query($query, $cid);
        $comment = _comment_load($cid);
        _comment_update_node_statistics($comment->nid);
        // Allow modules to respond to the updating of a comment.
        comment_invoke_comment($comment, $form_state['values']['operation']);
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
}


/** 
 * Theme results table.
 *
 * @ingroup themable
 *
 * @return table with filter results
 */ 
function theme_cmf_admin_comments_form($form) {
  global $user;
  $output = drupal_render($form['options']);
  if (isset($form['title']) && is_array($form['title'])) {
    foreach (element_children($form['title']) as $key) {
      $row = array();
      if (user_access('filter and manage site content')) {
        $row[] = drupal_render($form['comments'][$key]);
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
