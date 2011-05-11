<?php
// $Id$
/**
 * @file data_monitoring_multisite.monitors.inc
 * Menu callback for data monitoring multisite monitors form.
 */

/**
 * Callback for monitoring form.
 * Shows aggregate monitors and results from multiple site.
 *
 * @param $form_state
 * @return
 *     Form array.
 */
function data_monitoring_multisite_form_monitors(&$form_state) {

  $form['#attributes'] = array('class' => 'data-monitoring-multisite');
  $filter_show_options = array(0 => t('Latest'), 1 => t('All'));
  $form['filters'] = array(
    '#type' => 'fieldset',
    '#attributes' => array('id' => 'monitor-filter'),
    '#title' => t('Filter results'),
    '#description' => t('Choose <em>latest</em> to restrict display to latest results for each monitor in each site. '
                      . 'Note, only monitors with responses pending or responses implemented are aggregated here.'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['filters']['show'] = array(
    '#type' => 'select',
    '#options' => $filter_show_options,
    '#default_value' => (isset($_SESSION['data_monitoring_multisite_filter']['show'])) ? $_SESSION['data_monitoring_multisite_filter']['show'] : 0,
  );
  $form['filters']['filter_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Show'),
    '#submit' => array('_data_monitoring_multisite_form_monitors_filter_submit'),
  );

  $header = array();
  $header[] = array('data' => t('Site'), 'field' => 'shortname', 'class' => 'site');
  $header[] = array('data' => t('Monitor'), 'field' => 'name', 'class' => 'monitor');
  $header[] = array('data' => t('Processed'), 'field' => 'processed', 'sort' => 'desc', 'class' => 'processed');
  $header[] = array('data' => t('Result count'), 'field' => 'result_count', 'class' => 'result');
  $header[] = array('data' => t('Log file'), 'class' => 'log-file');
  $header[] = array('data' => t('Responses pending'), 'field' => 'responses_pending', 'class' => 'responses-pending');
  $header[] = array('data' => t('Responses implemented'), 'field' => 'responses_implemented', 'class' => 'responses-implemented');
  $header[] = array('data' => t('Change'), 'class' => 'change', 'title' => 'Change in result count since last result.');

  $form['monitors'] = array();
  $form['monitors']['header'] = array('#value' => $header);
  $form['monitors']['limit'] = array('#value' => 50);

  $show_filter = array(
    '0' => array( // latest
      'sql' => "SELECT dmml.*
                FROM {data_monitoring_multisite_log} dmml,
                (SELECT MAX(processed) AS latest, shortname, dmid FROM {data_monitoring_multisite_log} GROUP BY shortname, dmid) mr
                WHERE dmml.shortname = mr.shortname AND dmml.dmid = mr.dmid AND dmml.processed = mr.latest",
      'sql_count' => "SELECT COUNT(DISTINCT(CONCAT(shortname, dmid))) FROM {data_monitoring_multisite_log}",
    ),
    '1' => array( // all
      'sql' => "SELECT dmml.*
                FROM {data_monitoring_multisite_log} dmml",
      'sql_count' => "SELECT COUNT(*) FROM {data_monitoring_multisite_log}",
    ),
  );
  $selected_filter = $form['filters']['show']['#default_value'];
  $sql = $show_filter[$selected_filter]['sql'];
  $sql .= tablesort_sql($header);
  $sql = db_rewrite_sql($sql);
  $sql_count = $show_filter[$selected_filter]['sql_count'];
  $result = pager_query($sql, $form['monitors']['limit']['#value'], 0, $sql_count);

  while ($log = db_fetch_object($result)) {
    _data_monitoring_multisite_compare($log);
    $form['monitors']['shortname'][$log->dmmlid] = array('#value' => $log->shortname);
    $form['monitors']['id'][$log->dmmlid] = array('#value' => $log->id);
    $form['monitors']['dmid'][$log->dmmlid] = array('#value' => $log->dmid);
    $form['monitors']['module'][$log->dmmlid] = array('#value' => $log->module);
    $form['monitors']['name'][$log->dmmlid] = array('#value' => $log->name);
    $form['monitors']['description'][$log->dmmlid] = array('#value' => $log->description);
    $form['monitors']['processed'][$log->dmmlid] = array('#value' => $log->processed);
    $form['monitors']['result_count'][$log->dmmlid] = array('#value' => $log->result_count);
    $form['monitors']['log_file'][$log->dmmlid] = array('#value' => $log->log_file);
    $form['monitors']['responses_pending'][$log->dmmlid] = array('#value' => $log->responses_pending);
    $form['monitors']['responses_implemented'][$log->dmmlid] = array('#value' => $log->responses_implemented);
    $form['monitors']['change'][$log->dmmlid] = array('#value' => $log->change);
  }

  $form['last_aggregate_logging_run'] = array('#value' => variable_get('data_monitoring_multisite_last_aggregate_logging_run', 0));

  $form['aggregate_monitors'] = array(
    '#type' => 'submit',
    '#value' => t('Aggregate latest results'),
    '#access' => user_access('administer multisite monitoring'),
    '#submit' => array('_data_monitoring_multisite_form_monitors_aggregate_submit'),
  );
  $form['email_report'] = array(
    '#type' => 'submit',
    '#value' => t('Email latest results'),
    '#access' => user_access('administer multisite monitoring'),
    '#submit' => array('_data_monitoring_multisite_form_monitors_email_submit'),
  );
  $form['#theme'] = 'data_monitoring_multisite_monitors';
  return $form;
}
/**
 * Submit callback for data_monitoring_multisite_form_monitors().
 * @see data_monitoring_multisite_form_monitors()
 * @param $form
 * @param $form_state
 */
function _data_monitoring_multisite_form_monitors_aggregate_submit($form, &$form_state) {
  _data_monitoring_multisite_aggregate_results();
}
function _data_monitoring_multisite_form_monitors_email_submit($form, &$form_state) {
  _data_monitoring_multisite_send_report();
}
/**
 * Custom submit handler for the filter button.
 * @see data_monitoring_form_monitors().
 */
function _data_monitoring_multisite_form_monitors_filter_submit($form, &$form_state) {
  $_SESSION['data_monitoring_multisite_filter']['show'] = $form_state['values']['show'];
}
/**
 * Helper function to compare monitor result count with previous monitor result.
 * @param $log
 *     Multisite monitor standard object.
 * @return Integer
 *     The difference between the $log result count and the previous result count or 0 if no previous result count.
 */
function _data_monitoring_multisite_compare(&$log) {
  static $logs = array();
  if (!count($logs) > 0) {
    $result = db_query("SELECT dmmlid, shortname, id, module, result_count, processed, responses_pending, responses_implemented
                        FROM {data_monitoring_multisite_log}");
    while ($row = db_fetch_object($result)) {
      $logs[$row->shortname][$row->module][$row->id][$row->processed] = array(
        'count' => $row->result_count,
        'responses_implemented' => $row->responses_implemented,
        'responses_pending' => $row->responses_pending,
      );
    }
  }
  $before = 0;
  $log->change = $log->result_count;
  foreach ($logs[$log->shortname][$log->module][$log->id] as $processed => $row) {
    if ($processed < $log->processed && $processed > $before) {
      $before = $processed;
      // This is not going to be 100% accurate, we are assuming that if response was implemented then
      // count of results affected has returned to 0, and therefore we leave change as the current result count
      // this is not always the case, but we don't have enough data in multisite monitor to be any more accurate.
      if ($row['responses_pending'] > 0 and $row['responses_implemented'] == 0) {
        $log->change = $log->result_count - $row['count'];
      }
    }
  }



}
