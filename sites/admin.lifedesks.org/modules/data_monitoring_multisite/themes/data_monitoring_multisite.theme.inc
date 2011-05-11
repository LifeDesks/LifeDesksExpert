<?php
// $Id$
/**
 * @file data_monitoring_multisite.theme.inc
 * Includes themes for data monitoring multisite forms and email reports.
 */

function theme_data_monitoring_multisite_email_report_plain_monitor($monitor, $language) {
  global $base_root;
  $shortname_url = preg_replace('#^http://[^.]*#i', 'http://' . $monitor->shortname, $base_root);
  return t('Site: !shortname (!shortname_url)', array('!shortname' => $monitor->shortname, '!shortname_url' => $shortname_url), $language->language) . "\r\n"
       . t('Monitor: @monitor_name', array('@monitor_name' => $monitor->name), $language->language) . "\r\n"
       . t('Result count: !count', array('!count' => $monitor->result_count), $language->language) . "\r\n"
       . t('Responses implemented: !implemented', array('!implemented' => $monitor->responses_implemented), $language->language) . "\r\n"
       . t('Responses pending: !pending', array('!pending' => $monitor->responses_pending), $language->language) . "\r\n\r\n";
}
/**
 * Returns array of rows with HTML styles for email report, rows are intended for table theme.
 * @param $monitor
 * @param $language
 */
function theme_data_monitoring_multisite_email_report_html_monitor($monitor, $language) {
  global $base_root;
  $site_url = preg_replace('#^http://[^.]*#i', 'http://' . $monitor->shortname, $base_root);
  $style_tr = 'background-color: #eee; border-bottom: 2px solid #fff;';
  $style_td = 'border-spacing: 0; padding: 3px; vertical-align: top;';
  $row = array(
    'data' => array(
      array('data' => l($monitor->shortname, $site_url, array('html' => TRUE)), 'style' => $style_td),
      array('data' => $monitor->name, 'style' => $style_td),
      array('data' => $monitor->result_count, 'style' => $style_td),
      array('data' => $monitor->responses_implemented, 'style' => $style_td),
      array('data' => $monitor->responses_pending, 'style' => $style_td),
    ),
    'style' => $style_tr,
  );
  return $row;
}
function theme_data_monitoring_multisite_email_report_attachment_header($language) {
  $header = array(
    t('Site', array(), $language->language),
    t('Site URL', array(), $language->language),
    t('Monitor', array(), $language->language),
    t('Description', array(), $language->language),
    t('Processed', array(), $language->language),
    t('Result count', array(), $language->language),
    t('Responses implemented', array(), $language->language),
    t('Responses pending', array(), $language->language),
    t('Log file', array(), $language->language),
  );
  return implode("\t", $header) . "\n";
}
function theme_data_monitoring_multisite_email_report_attachment_monitor($monitor, $language) {
  global $base_root;
  $site_url = preg_replace('#^http://[^.]*#i', 'http://' . $monitor->shortname, $base_root);
  $row = array(
    $monitor->shortname,
    $site_url,
    $monitor->name,
    $monitor->description,
    format_date($monitor->processed, 'medium', '', NULL, $language->language),
    $monitor->result_count,
    $monitor->responses_implemented,
    $monitor->responses_pending,
    "{$site_url}/{$monitor->log_file}",
  );
  return implode("\t", $row) . "\n";
}
/**
 * HTML themed aggregate monitor logs summary.
 * @see data_monitoring_multisite_form_monitors()
 *
 */
function theme_data_monitoring_multisite_monitors($form) {

  global $base_root;

  $module_path = drupal_get_path('module', 'data_monitoring_multisite');

  drupal_add_css($module_path . '/themes/css/data_monitoring_multisite.css');
  drupal_add_js($module_path . '/themes/js/data_monitoring_multisite.js');
  $image_path = "/" . drupal_get_path('module', 'data_monitoring_multisite') . "/themes/images";
  $output = '';
  $output .= drupal_render($form['filters']);

  $rows = array();
  if (isset($form['monitors']['shortname']) && is_array($form['monitors']['shortname'])) {
    foreach (element_children($form['monitors']['shortname']) as $key) {
      $shortname = drupal_render($form['monitors']['shortname'][$key]);
      $module = drupal_render($form['monitors']['module'][$key]);
      $id = drupal_render($form['monitors']['id'][$key]);
      $site_url = preg_replace('#^http://[^.]*#i', 'http://' . $shortname, $base_root);
      $r = array();
      $r[] = array('data' => l($shortname, $site_url, array('absolute' => TRUE)));
      $murl = url('admin/reports/data-monitoring-multisite/monitor/' . $module . '/' . $id, array('query' => str_replace('destination=', 'referrer=', drupal_get_destination())));
      $mname = '<a href="' .$murl . '">' . drupal_render($form['monitors']['name'][$key]) . '</a>';
      $r[] = array(
        'data' => '<span class="monitor-name">' . $mname . '</span>'
                . '<span class="monitor-description">' . drupal_render($form['monitors']['description'][$key]) . '</span>',
        'class' => 'monitor'
      );
      $processed = drupal_render($form['monitors']['processed'][$key]);
      $r[] = array(
        'data' => ($processed > 0) ? date('c', $processed) : t('Date missing'),
        'class' => 'processed',
      );
      unset($processed);
      $r[] = array(
        'data' => drupal_render($form['monitors']['result_count'][$key]),
        'class' => 'result-count',
      );
      $filepath = $site_url . '/' . drupal_render($form['monitors']['log_file'][$key]);
      $r[] = array(
        'data' => ($filepath) ? l("<img src=\"$image_path/table-sheet.png\" alt=\"" . t('Log file.') . '" />', $filepath, array('html' => TRUE, 'absolute' => TRUE)) : t('Log file missing'),
        'class' => 'log-file',
      );
      unset($filepath);
      $r[] = array(
        'data' => drupal_render($form['monitors']['responses_pending'][$key]),
        'class' => 'responses-pending'
      );
      $r[] = array(
        'data' => drupal_render($form['monitors']['responses_implemented'][$key]),
        'class' => 'responses-implemented'
      );
      $change = drupal_render($form['monitors']['change'][$key]);
      if ($change > 0) {
        $r[] = array(
          'data' => "<img src=\"$image_path/arrow_up.png\" alt=\""
                   . t('Result count has increased since last count by !change.', array('!change' => $change))
                   . '" />',
          'class' => 'change increase'
        );
      }
      elseif ($change < 0) {
        $r[] = array(
          'data' => "<img src=\"$image_path/arrow_down.png\" alt=\""
                   . t('Result count has decreased since last count by !change.', array('!change' => $change))
                   . '" />',
          'class' => 'change decrease'
        );
      }
      else {
        $r[] = array(
          'data' => "<img src=\"$image_path/equal.png\" alt=\""
                   . t('Result count has not changed since last count.')
                   . '" />',
          'class' => 'change no-change'
        );
      }
      $rows[] = array('data' => $r, 'data_dmmlid' => $key);
    }
  }
  else {
    $rows[] = array(array('data' => t('No logs found.'), 'colspan' => '7'));
  }

  $output .= theme('pager', NULL, $form['monitors']['limit']['#value'], 0);
  $output .= theme('table', $form['monitors']['header']['#value'], $rows, array('id' => 'data-monitoring-multisite-monitors', 'class' => 'data-monitoring-multisite'));

  $last_aggregated = drupal_render($form['last_aggregate_logging_run']);
  if ($last_aggregated > 0) {
    $output .= '<p>'
             . t('Monitors last aggregated on !date', array('!date' => format_date($last_aggregated, 'custom', 'r')))
             . '</p>';
  }
  unset($form['monitors']['header']);
  unset($form['monitors']['limit']);
  unset($form['monitors']['dmid']);
  $output .= drupal_render($form);
  return $output;

}
