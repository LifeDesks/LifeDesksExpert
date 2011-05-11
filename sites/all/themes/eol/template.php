<?php
// $Id: template.php,v 1.16 2007/10/11 09:51:29 goba Exp $

/**
 * Override for: Theme user administration overview.
 * user.admin.inc
 *
 * @ingroup themeable
 */
function eol_user_admin_account($form) {
  // Overview table:
  $header = array(
    theme('table_select_header_cell'),
    array('data' => t('Username'), 'field' => 'u.name'),
    array('data' => t('Status'), 'field' => 'u.status'),
    t('Roles'),
    array('data' => t('Member for'), 'field' => 'u.created', 'sort' => 'desc'),
    array('data' => t('Last access'), 'field' => 'u.access'),
    t('Operations')
  );

  $output = drupal_render($form['options']);
  if (isset($form['name']) && is_array($form['name'])) {
    foreach (element_children($form['name']) as $key) {
      //adding conditional statement to remove uid=1 from /admin/user/user list
      if($key == 1 || $key == variable_get('lifedesk_support', 0)) {
          unset($form['accounts'][$key]);
          unset($form['name'][$key]);
          unset($form['status'][$key]);
          unset($form['roles'][$key]);
          unset($form['member_for'][$key]);
          unset($form['last_access'][$key]);
          unset($form['operations'][$key]);
      } else {
        $rows[] = array(
          drupal_render($form['accounts'][$key]),
          drupal_render($form['name'][$key]),
          drupal_render($form['status'][$key]),
          drupal_render($form['roles'][$key]),
          drupal_render($form['member_for'][$key]),
          drupal_render($form['last_access'][$key]),
          drupal_render($form['operations'][$key]),
        );
      }
    }
  }
  else {
    $rows[] = array(array('data' => t('No users available.'), 'colspan' => '7'));
  }

  $output .= theme('table', $header, $rows);
  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $output .= drupal_render($form);

  return $output;
}

function eol_body_class() {
  $class = '';
  $class .= eol_get_page_classes();

  if (isset($class)) {
    print ' class="'. $class .'"';
  }
}

/**
 * Return a themed breadcrumb trail.
 *
 * @param $breadcrumb
 *   An array containing the breadcrumb links.
 * @return a string containing the breadcrumb output.
 */
function phptemplate_breadcrumb($breadcrumb) {
  if (!empty($breadcrumb)) {
    return '<div class="breadcrumb">'. implode(' › ', $breadcrumb) .'</div>';
  }
}

/**
 * Allow themable wrapping of all comments.
 */
function phptemplate_comment_wrapper($content, $node) {
  if (!$content || $node->type == 'forum') {
    return '<div id="comments">'. $content .'</div>';
  }
  else {
    return '<div id="comments"><h2 class="comments">'. t('Comments') .'</h2>'. $content .'</div>';
  }
}

/**
 * Replace username with Given Name and Surname from profile
 */
function phptemplate_username($object) {

  if ($object->uid && $object->name) {
    $thisuser = user_load(array('uid'=>$object->uid));
    $fullname = $thisuser->surname . ', ' . $thisuser->givenname;
    // Shorten the name when it is too long or it will break many tables.
    if (drupal_strlen($fullname) > 20) {
      $name = drupal_substr($fullname, 0, 15) . '...';
    }
    else {
      $name = $fullname;
    }
    if (user_access('access user profiles')) {
      $output = l($name, 'user/'. $object->uid, array('absolute' => true, 'attributes' => array('title' => t('View user profile.'))));
    }
    else {
      $output = check_plain($name);
    }
  }
  else if ($object->name) {
    if ($object->homepage) {
      $output = l($object->name, $object->homepage, array('rel' => 'nofollow'));
    }
    else {
      $output = check_plain($object->name);
    }
  }
  else {
    $output = variable_get('anonymous', t('Anonymous'));
  }
  return $output;
}

/**
 * Override or insert PHPTemplate variables into the templates.
 */
function phptemplate_preprocess_page(&$vars) {
  $vars['tabs2'] = menu_secondary_local_tasks();
  // Hook into color.module
  if (module_exists('color')) {
    _color_page_alter($vars);
  }
}

/**
 * Returns the rendered local tasks. The default implementation renders
 * them as tabs. Overridden to split the secondary tasks.
 *
 * @ingroup themeable
 */
function phptemplate_menu_local_tasks() {
  return menu_primary_local_tasks();
}

function phptemplate_comment_submitted($comment) {
  return t('!datetime — !username',
    array(
      '!username' => theme('username', $comment),
      '!datetime' => format_date($comment->timestamp)
    ));
}

function phptemplate_node_submitted($node) {
  return t('!datetime — !username',
    array(
      '!username' => theme('username', $node),
      '!datetime' => format_date($node->created),
    ));
}

function eol_get_page_classes($path = NULL) {
  $classes = '';
  if (!isset($path) && isset($_GET['q'])) $path = $_GET['q'];
  if ($path) {
    $menu_item = explode('/', $path);
    if (count($menu_item)) {
      foreach ($menu_item as $key => $page) {
        $menu_item[$key] = strtr($page, '-', '_');
      }

      do {
        $classes .= ' '. implode('-', $menu_item);
        array_pop($menu_item);
      } while (count($menu_item));
    }
  }

  return $classes;
}

/**
 * Generates IE CSS links for LTR and RTL languages.
 */
function phptemplate_get_ie_styles() {

  $iecss = '<link type="text/css" rel="stylesheet" media="all" href="'. base_path() . path_to_theme() .'/fix-ie.css" />';

  return $iecss;
}

/**
 * Strip the link for formatting hints
 */
function phptemplate_filter_tips_more_info() {
  return '';
}