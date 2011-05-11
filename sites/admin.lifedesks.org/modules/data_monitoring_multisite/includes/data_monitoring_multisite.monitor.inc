<?php

/**
 * @file data_monitoring_multisite.analysis.inc
 * Menu callback for data monitoring multisite analysis form.
 */
/**
 * Callback for monitor page.
 * Shows monitor results from multiple sites over time.
 *
 * @return
 *    String
 */
function data_monitoring_multisite_monitor($module, $id = NULL) {

  global $base_root;

  $module_path = drupal_get_path('module', 'data_monitoring_multisite');
  drupal_add_css($module_path . '/themes/css/data_monitoring_multisite.css');

  $referrer = (!empty($_GET['referrer'])) ? $_GET['referrer'] : 'admin/reports/data-monitoring-multisite';
  if (empty($module) || empty($id)) {
    drupal_set_message(t("Sorry, I couldn't load the monitor. Please try selecting the monitor again. "
                       . "If the problem persists I'm afraid there might be a bug in the system. Sorry about that, "
                       . "if it doesn't fix itself after a couple of tries, get the development team to take a "
                       . "look under the hood."), 'error');
    unset($module);
    unset($id);
    drupal_goto($referrer);
  }

  $result = db_query("SELECT dmmlid, shortname, id, module, name, description, processed, result_count,
                      (FLOOR(processed / 3600) * 3600) AS processed_floor
                      FROM {data_monitoring_multisite_log} dmml
                      WHERE dmml.module = '%s'
                      AND dmml.id = '%s'", $module, $id);

  $last_processed = db_result(db_query("SELECT (FLOOR(MAX(processed) / 3600) * 3600) AS last_processed FROM {data_monitoring_multisite_log}"));

  $last_count_rows = array();
  while ($log = db_fetch_object($result)) {
    if (!isset($issue)) {
      $issue = array(
        'id' => $log->id,
        'module' => $log->module,
        'name' => $log->name,
        'description' => $log->description,
      );
    }
    $issue['data'][$log->shortname][$log->processed] = $log->result_count;
    if ($log->processed_floor == $last_processed) {
      $site_url = preg_replace('#^http://[^.]*#i', 'http://' . $log->shortname, $base_root);
      $last_count_rows[] = array(
        l($log->shortname, $site_url, array('absolute' => TRUE)),
        $log->result_count,
      );
    }
  }

  $table_headers = array(
    t('Site'),
    t('Count')
  );
  $last_count_table = theme('table', $table_headers, $last_count_rows, array('class' => 'data-monitoring-multisite'),
    t('Most recent counts from each site for this monitor. Counts were taken on !last_processed',
    array('!last_processed' => date('d F, Y', $last_processed))));
  drupal_set_title('Monitor summary: ' . $issue['name']);
  return theme('data_monitoring_multisite_monitor', $issue, $last_count_table, $referrer);
}