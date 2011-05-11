<?php
// $Id$
/**
 * @file
 * Menu callback for Data monitoring administration logs form.
 */

/**
 * Callback for data_monitoring_form_monitors() form.
 *
 * @param $form_state
 */
function data_monitoring_form_monitors(&$form_state) {

  $form['#attributes'] = array('class' => 'data-monitoring');

  $day = 86400;
  $week = 604800;
  $interval = drupal_map_assoc(array(0, $day, 2 * $day, 3 * $day, 4 * $day, 5 * $day, 6 * $day, $week, 1 * $week, 2 * $week, 3 * $week, 4 * $week, 8 * $week, 16 * $week, 24 * $week), 'format_interval');
  $interval[0] = t("Don't delete logs in bulk");

  $form['trigger_monitoring'] = array(
    '#access' => user_access('administer monitoring'),
    '#type' => 'fieldset',
    '#title' => t('Trigger monitoring'),
    '#description' => t('Run monitor queries for all monitors. If automated responses are allowed they will be '
                      . 'executed for applicable monitors. In addition any monitors that are deemed to require a response '
                      . 'procedure will be logged to the database - a log file containing more detailed results from the '
                      . 'monitor query may also be saved to the file system.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['trigger_monitoring']['run'] = array(
    '#type' => 'submit',
    '#value' => t('Run monitors'),
    '#submit' => array('_data_monitoring_form_monitors_run_submit'),
    '#validate' => array('_data_monitoring_form_monitors_run_validate'),
  );

  $form['delete_logs'] = array(
    '#access' => user_access('administer monitoring'),
    '#type' => 'fieldset',
    '#title' => t('Delete logs'),
    '#description' => t('Delete selected logs, and/or bulk delete all logs older than the age you specify. '
                      . 'Deletion <strong>permanently</strong> removes logs from the database and filesystem.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['delete_logs']['age'] = array(
    '#type' => 'select',
    '#title' => t('Bulk delete logs older than'),
    '#options' => $interval,
    '#default_value' => 0,
  );
  $form['delete_logs']['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete logs'),
    '#submit' => array('_data_monitoring_form_monitors_delete_submit'),
    '#validate' => array('_data_monitoring_form_monitors_delete_validate'),
  );

  $latest_processed = db_query("SELECT MAX(processed) AS latest, dmid FROM {data_monitoring_log} GROUP BY dmid");
  while ($latest = db_fetch_object($latest_processed)) {
    $form['latest_processed'][$latest->dmid] = array('#value' => $latest->latest);
  }

  $selected_filter = (isset($_SESSION['data_monitoring_filter']['show'])) ? $_SESSION['data_monitoring_filter']['show'] : 0;
  $form['filters']['latest'] = array(
    '#type' => 'submit',
    '#value' => t('Latest'),
    '#submit' => array('_data_monitoring_form_monitors_filter_latest_submit'),
    '#attributes' => array('class' => ($selected_filter == 0) ? 'on' : 'off'),
  );
  $form['filters']['all'] = array(
    '#type' => 'submit',
    '#value' => t('All'),
    '#submit' => array('_data_monitoring_form_monitors_filter_all_submit'),
    '#attributes' => array('class' => ($selected_filter == 1) ? 'on' : 'off'),
  );
  $header = array(
    '',
    array('data' => t('Monitor'), 'field' => 'name', 'class' => 'monitor'),
    array('data' => t('Processed'), 'field' => 'processed', 'sort' => 'desc', 'class' => 'processed'),
    array('data' => t('Result count'), 'field' => 'result_count', 'class' => 'result-count'),
    array('data' => t('Log file'), 'class' => 'log-file'),
    array('data' => t('Responses pending'), 'field' => 'responses_pending', 'class' => 'responses-pending'),
    array('data' => t('Responses implemented'), 'field' => 'responses_implemented', 'class' => 'responses-implemented'),
  );

  $form['headers'] = array('#value' => $header);
  $form['limit'] = array('#value' => 50);

  if ($selected_filter == 1) {
    // Show all monitor logs.
    $sql = "SELECT dml.*, dmm.id, dmm.module, dmm.name, dmm.description
            FROM {data_monitoring_log} dml
            JOIN {data_monitoring_monitors} dmm ON dmm.dmid = dml.dmid";
    $sql_count = "SELECT COUNT(*)
                  FROM {data_monitoring_log} dml
                  JOIN {data_monitoring_monitors} dmm ON dmm.dmid = dml.dmid";
  }
  else {
    // Show only the latest log for each monitor.
    $sql = "SELECT dml.*, dmm.id, dmm.module, dmm.name, dmm.description
            FROM {data_monitoring_log} dml
            JOIN {data_monitoring_monitors} dmm ON dmm.dmid = dml.dmid,
            (SELECT MAX(processed) AS latest, dmid FROM {data_monitoring_log} GROUP BY dmid) lm
            WHERE dml.dmid = lm.dmid AND dml.processed = lm.latest";
    $sql_count = "SELECT COUNT(DISTINCT(dmid)) FROM {data_monitoring_log}";
  }

  $sql .= tablesort_sql($header);
  $result = pager_query($sql, $form['limit']['#value'], 0, $sql_count);

  $logs = array();
  while ($log = db_fetch_object($result)) {
    $logs[$log->dmlid] = '';
    $form['dmid'][$log->dmlid] = array('#value' => $log->dmid);
    $form['name'][$log->dmlid] = array('#value' => t($log->name));
    $form['description'][$log->dmlid] = array('#value' => t($log->description));
    $form['processed'][$log->dmlid] = array('#value' => $log->processed);
    $form['result_count'][$log->dmlid] = array('#value' => $log->result_count);
    $form['log_file'][$log->dmlid] = array('#value' => $log->log_file);
    $form['responses_pending'][$log->dmlid] = array('#value' => $log->responses_pending);
    $form['responses_implemented'][$log->dmlid] = array('#value' => $log->responses_implemented);
  }
  $form['logs'] = array('#type' => 'checkboxes', '#options' => $logs, '#access' => user_access('administer monitoring'));

  $form['last_monitoring'] = array('#value' => variable_get('data_monitoring_last_monitoring_run', 0));

  $form['#theme'] = 'data_monitoring_monitors';
  return $form;
}
/**
 * Validation callback for trigger monitors button.
 */
function _data_monitoring_form_monitors_run_validate($form, &$form_state) {

}
/**
 * Submit callback for trigger monitors button.
 */
function _data_monitoring_form_monitors_run_submit($form, &$form_state) {
  // Get data monitoring include files from other modules.
  _data_monitoring_module_include('data_monitoring.inc');

  $manager = new DataMonitoringManager();
  $manager->processMonitors(DATA_MONITORING_STATE_MANUAL_IMPLEMENTATION);
}
/**
 * Validation callback for delete button on data_monitoring_form_monitors()
 * @see data_monitoring_form_monitors()
 * @param $form
 * @param $form_state
 */
function _data_monitoring_form_monitors_delete_validate($form, &$form_state) {
  if (!$form_state['values']['age'] > 0) {
    $error = TRUE;
    foreach ($form_state['values']['logs'] as $dmlid) {
      if ($dmlid > 0) $error = FALSE;
    }
  }
  if ($error) form_set_error('age', t("Sorry, I couldn't find any logs to delete. Please try reselecting the logs "
                                    . "you'd like me remove. To select logs for removal check the box "
                                    . "next to the log record, and/or select an age from the <em>older than</em> "
                                    . "drop down list. Both checked logs and logs older than the age you select "
                                    . "will be removed when you activate the <em>Delete logs</em> button."));
}
function _data_monitoring_form_monitors_delete_submit($form, &$form_state) {
  // Make sure data monitoring include files from other modules are available.
  _data_monitoring_module_include('data_monitoring.inc');
  DataMonitoringManager::deleteLogs($form_state['values']['logs'], $form_state['values']['age']);
  DataMonitoringFeedbackHandler::getInstance()->displayFeedback();
}
/**
 * Custom submit handler for the filter latest button.
 * @see data_monitoring_form_monitors().
 */
function _data_monitoring_form_monitors_filter_latest_submit($form, &$form_state) {
  $_SESSION['data_monitoring_filter']['show'] = 0;
}
/**
 * Custom submit handler for the filter all button.
 * @see data_monitoring_form_monitors().
 */
function _data_monitoring_form_monitors_filter_all_submit($form, &$form_state) {
  $_SESSION['data_monitoring_filter']['show'] = 1;
}

