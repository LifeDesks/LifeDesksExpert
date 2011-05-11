<?php
// $Id$

/**
 * @file
 * Contains theme override functions and preprocess functions for the theme.
 *
 * ABOUT THE TEMPLATE.PHP FILE
 *
 *   The template.php file is one of the most useful files when creating or
 *   modifying Drupal themes. You can add new regions for block content, modify
 *   or override Drupal's theme functions, intercept or make additional
 *   variables available to your theme, and create custom PHP logic. For more
 *   information, please visit the Theme Developer's Guide on Drupal.org:
 *   http://drupal.org/theme-guide
 *
 * OVERRIDING THEME FUNCTIONS
 *
 *   The Drupal theme system uses special theme functions to generate HTML
 *   output automatically. Often we wish to customize this HTML output. To do
 *   this, we have to override the theme function. You have to first find the
 *   theme function that generates the output, and then "catch" it and modify it
 *   here. The easiest way to do it is to copy the original function in its
 *   entirety and paste it here, changing the prefix from theme_ to expert_.
 *   For example:
 *
 *     original: theme_breadcrumb()
 *     theme override: expert_breadcrumb()
 *
 *   where expert is the name of your sub-theme. For example, the
 *   zen_classic theme would define a zen_classic_breadcrumb() function.
 *
 *   If you would like to override any of the theme functions used in Zen core,
 *   you should first look at how Zen core implements those functions:
 *     theme_breadcrumbs()      in zen/template.php
 *     theme_menu_item_link()   in zen/template.php
 *     theme_menu_local_tasks() in zen/template.php
 *
 *   For more information, please visit the Theme Developer's Guide on
 *   Drupal.org: http://drupal.org/node/173880
 *
 * CREATE OR MODIFY VARIABLES FOR YOUR THEME
 *
 *   Each tpl.php template file has several variables which hold various pieces
 *   of content. You can modify those variables (or add new ones) before they
 *   are used in the template files by using preprocess functions.
 *
 *   This makes THEME_preprocess_HOOK() functions the most powerful functions
 *   available to themers.
 *
 *   It works by having one preprocess function for each template file or its
 *   derivatives (called template suggestions). For example:
 *     THEME_preprocess_page    alters the variables for page.tpl.php
 *     THEME_preprocess_node    alters the variables for node.tpl.php or
 *                              for node-forum.tpl.php
 *     THEME_preprocess_comment alters the variables for comment.tpl.php
 *     THEME_preprocess_block   alters the variables for block.tpl.php
 *
 *   For more information on preprocess functions and template suggestions,
 *   please visit the Theme Developer's Guide on Drupal.org:
 *   http://drupal.org/node/223440
 *   and http://drupal.org/node/190815#template-suggestions
 */

/**
 * Implementation of HOOK_theme().
 */
function expert_theme(&$existing, $type, $theme, $path) {
  $hooks = zen_theme($existing, $type, $theme, $path);
  // Add your theme hooks like this:
  /*
  $hooks['hook_name_here'] = array( // Details go here );
  */
  $hooks['user_login_block'] = array(
    'arguments' => array('form' => array()),
    'template' => 'user-login-block',
    'path' => drupal_get_path('theme', 'expert') .'/templates',
  );
  return $hooks;
}

/**
 * Override or insert variables into all templates.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered (name of the .tpl.php file.)
 */
function expert_preprocess(&$vars, $hook) {
  if (!theme_get_setting('expert_help')) unset($vars['help']);
}
// */

/**
 * Override or insert variables into the page templates.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("page" in this case.)
 */
function expert_preprocess_page(&$vars, $hook) {
  //$vars['sample_variable'] = t('Lorem ipsum.');
  // Hook into color.module
  if (module_exists('color')) {
    _color_page_alter($vars);
  }
}
// */

/**
 * Override or insert variables into the node templates.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("node" in this case.)
 */
/* -- Delete this line if you want to use this function
function expert_preprocess_node(&$vars, $hook) {
  $vars['sample_variable'] = t('Lorem ipsum.');

  // Optionally, run node-type-specific preprocess functions, like
  // expert_preprocess_node_page() or expert_preprocess_node_story().
  $function = __FUNCTION__ . '_' . $vars['node']->type;
  if (function_exists($function)) {
    $function($vars, $hook);
  }
}
// */

/**
 * Override or insert variables into the comment templates.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("comment" in this case.)
 */
/* -- Delete this line if you want to use this function
function expert_preprocess_comment(&$vars, $hook) {
  $vars['sample_variable'] = t('Lorem ipsum.');
}
// */

/**
 * Override or insert variables into the block templates.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("block" in this case.)
 */
/* -- Delete this line if you want to use this function
function expert_preprocess_block(&$vars, $hook) {
  $vars['sample_variable'] = t('Lorem ipsum.');
}
// */

/**
 * Override or insert variables into the user login block template.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("user-login-block" in this case.)
 */
function expert_preprocess_user_login_block(&$vars, $hook) {
  $items = array();
  if (variable_get('user_register', 1)) {
    $items[] = l(t('Create new account'), 'user/register', array('attributes' => array('title' => t('Create a new user account.'), 'class' => 'register')));
  }
  $items[] = l(t('Request new password?'), 'user/password', array('attributes' => array('title' => t('Request new password via e-mail.'), 'class' => 'request-password')));
  $vars['form']['links']['#value'] = theme('item_list', $items);
}

/**
 * Implements theme_menu_item_link()
 */
function expert_menu_item_link($link) {
  if (empty($link['localized_options'])) {
    $link['localized_options'] = array();
  }

  if ($link['type'] & MENU_IS_LOCAL_TASK) { // If an item is a LOCAL TASK, render it as a tab
    $link['title'] = '<span class="tab">' . check_plain($link['title']) . '</span>';
    $link['localized_options']['html'] = TRUE;
  } else { // Not a LOCAL TASK
    // Add css class to action-links menu
    if ($link['menu_name'] == 'menu-action-links') {
      $link['localized_options']['attributes']['class'] = preg_replace('#[^A-z]#i', '-', $link['link_path']);
    }
  }
  return l($link['title'], $link['href'], $link['localized_options']);
}

/**
 * Implements theme_node_submitted
 */
function expert_node_submitted($node) {
  return t('@datetime &mdash; !username',
    array(
      '!username' => theme('username', $node),
      '@datetime' => format_date($node->created),
    ));
}
/**
 * Implements theme_comment_submitted
 */
function expert_comment_submitted($comment) {
  return t('@datetime &mdash; !username',
    array(
      '!username' => theme('username', $node),
      '@datetime' => format_date($node->created),
    ));
}
/**
 * Implements theme_filter_tips
 */
function expert_filter_tips($tips, $long = FALSE, $extra = '') {
  // Remove help tips if user doesn't have administer nodes permissions
  // This will not remove the format selection form, and is not a robust solution for managing input formats.
  // Recommend replacing this with Better Formats module or similar
  if (!user_access('administer filters')) return '';
      
  $output = '';
  
  $multiple = count($tips) > 1;
  if ($multiple) {
    $output = t('input formats') .':';
  }
  
  if (count($tips)) {
    if ($multiple) {
      $output .= '<ul>';
    }
    foreach ($tips as $name => $tiplist) {
      if ($multiple) {
        $output .= '<li>';
        $output .= '<strong>'. $name .'</strong>:<br />';
      }
  
      if (count($tiplist) > 0) {
        $output .= '<ul class="tips">';
        foreach ($tiplist as $tip) {
          $output .= '<li'. ($long ? ' id="filter-'. str_replace("/", "-", $tip['id']) .'">' : '>') . $tip['tip'] .'</li>';
        }
        $output .= '</ul>';
      }
  
      if ($multiple) {
        $output .= '</li>';
      }
    }
    if ($multiple) {
      $output .= '</ul>';
    }
  }
  
  return $output;
}
/**
 * Implements theme_filter_tips_more_info
 */
function expert_filter_tips_more_info() {
  // Remove link to more info.
  // This does not prevent access to filter/tips page and is not a robust solution.
  // Recommend replacing this with Better Formats module or similar 
  if (!user_access('administer filters')) return '';
  //return '<p>'. l(t('More information about formatting options'), 'filter/tips') .'</p>';
} 