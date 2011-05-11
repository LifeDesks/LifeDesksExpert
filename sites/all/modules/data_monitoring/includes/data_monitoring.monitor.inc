<?php
// $Id$
/**
 * @file
 * Menu callback for data monitoring response.
 */

/**
 * Monitor and associated responses form.
 * Responses form allows response process to be viewed and run.
 *
 */
function data_monitoring_form_monitor(&$form_state, $monitor) {

  // Make sure data monitoring include files from other modules are available.
  _data_monitoring_module_include('data_monitoring.inc');

  if (!$monitor->getDmid()) {
    drupal_set_message(t("Sorry, I couldn't load the monitor. Please try selecting the monitor again. "
                       . "If the problem persists I'm afraid there might be a bug in the system. Sorry about that, "
                       . "if it doesn't fix itself after a couple of tries, get the development team to take a "
                       . "look under the hood.",
                       array('@dmid' => $dmid)));
    unset($monitor);
    (isset($_GET['referrer'])) ? drupal_goto($_GET['referrer']) : drupal_goto('admin/reports/data-monitoring/monitors');
  }
  $monitor->setMonitorState(DATA_MONITORING_STATE_PREVIEW);
  $monitor->process();
  $form['#attributes'] = array('class' => 'data-monitoring');
  $form['referrer'] = array(
    '#value' => ($_GET['referrer']) ? $_GET['referrer'] : 'admin/reports/data-monitoring/monitors',
  );
  $form['monitor']['name'] = array(
    '#prefix' => '<h2>',
    '#value' => $monitor->getName(),
    '#suffix' => '</h2>',
  );
  $form['monitor']['description'] = array(
    '#prefix' => '<p>',
    '#value' => $monitor->getDescription(),
    '#suffix' => '</p>',
  );
  $form['monitor']['module'] = array(
    '#value' => $monitor->getModule(),
  );
  $form['monitor']['result_count'] = array(
    '#value' => $monitor->getResultCount(),
  );

  // Observers.
  if ($monitor->getResponsesPending()) {
    $form['observers'] = array(
      '#type' => 'fieldset',
      '#title' => t('Pending responses'),
      '#description' => t('Preview and implement pending responses.'),
      '#collapsible' => TRUE,
      '#collpsed' => FALSE,
      '#access' => user_access('administer monitoring responses'),
    );
    $observers_header = array(
      '',
      array('data' => t('Response'), 'class' => 'description'),
      array('data' => t('State'), 'class' => 'response-state'),
    );
    $form['observers']['header'] = array('#value' => $observers_header);

    $responses = array();
    foreach ($monitor->getObservers() as $i => $response) {
      if ($response->getResponseState() == DATA_MONITORING_STATE_PENDING) {
        if (!isset($default_reponse)) $default_response = $i;
        $responses[$i] = '';
        $form['observers']['id'][$i] = array('#value' => $response->id);
        $form['observers']['description'][$i] = array('#value' => $response->description);
        $form['observers']['response_state'][$i] = array('#value' => $response->response_state);
      }
    }
    $form['observers']['responses'] = array('#type' => 'radios', '#options' => $responses, '#default_value' => $default_response);
    if (!empty($responses)) {
      $form['observers']['preview_response'] = array(
        '#type' => 'submit',
        '#value' => t('Preview response'),
        '#submit' => array('_data_monitoring_form_monitor_preview_response_submit'),
        '#validate' => array('_data_monitoring_form_monitor_preview_response_validate')
      );
    }
  }
  else {
    $form['observers'] = array(
      '#type' => 'fieldset',
      '#title' => t('Pending responses'),
      '#description' => t('This monitor does not currently have any pending responses.'),
      '#collapsible' => TRUE,
      '#collpsed' => FALSE,
      '#access' => user_access('administer monitoring responses'),
    );
  }

  // Logs.
  $form['logs_title'] = array('#value' => t('View history'));
  $form['logs_description'] = array(
    '#prefix' => '<p>',
    '#value' => t('Previously logged results and responses for this monitor.'),
    '#suffix' => '</p>',
  );
  $responses_header = array(
    array('data' => t('Response processed'), 'class' => 'processed'),
    array('data' => t('Created'), 'class' => 'created'),
    array('data' => t('Updated'), 'class' => 'updated'),
    array('data' => t('Deleted'), 'class' => 'deleted'),
    array('data' => t('Remaining'), 'class' => 'remaining'),
    array('data' => t('Log file'), 'class' => 'log-file'),
    array('data' => t('State'), 'class' => 'state'),
  );
  $form['responses_header'] = array('#value' => $responses_header);

  $logs_header = array(
    array('data' => t('Monitor processed'), 'field' => 'processed', 'sort' => 'desc', 'class' => 'processed'),
    array('data' => t('Result count'), 'field' => 'result_count', 'class' => 'result-count'),
    array('data' => t('Log file'), 'class' => 'log-file'),
    array('data' => t('Responses pending'), 'field' => 'responses_pending', 'class' => 'responses-pending'),
    array('data' => t('Responses implemented'), 'field' => 'responses_implemented', 'class' => 'responses-implemented'),
  );
  $form['logs_header'] = array('#value' => $logs_header);
  $form['logs_limit'] = array('#value' => 10);

  $sql = "SELECT dml.*, dmrl.dmrlid, dmrl.response_state, dmrl.processed AS r_processed, dmrl.created_result_count,
          dmrl.updated_result_count, dmrl.deleted_result_count, dmrl.remaining_result_count, dmrl.log_file AS r_log_file
          FROM {data_monitoring_log} dml
          LEFT JOIN {data_monitoring_response_log} dmrl ON dml.dmlid = dmrl.dmlid
          WHERE dmid = %d";
  $sql .= tablesort_sql($logs_header);
  $sql = db_rewrite_sql($sql, 'data_monitoring_log', 'dmlid', array('dmid' => $monitor->getDmid()));
  $sql_count = "SELECT COUNT(*) FROM {data_monitoring_log} dml WHERE dmid = %d";
  $result = pager_query($sql, $form['logs_limit']['#value'], 1, $sql_count, $monitor->getDmid());

  $form['logs'] = array();
  while ($log = db_fetch_object($result)) {
    if (!isset($form['logs']['processed'][$log->dmlid])) {
      $form['logs']['processed'][$log->dmlid] = array('#value' => $log->processed);
      $form['logs']['result_count'][$log->dmlid] = array('#value' => $log->result_count);
      $form['logs']['log_file'][$log->dmlid] = array('#value' => $log->log_file);
      $form['logs']['responses_pending'][$log->dmlid] = array('#value' => $log->responses_pending);
      $form['logs']['responses_implemented'][$log->dmlid] = array('#value' => $log->responses_implemented);
    }
    if (!empty($log->dmrlid)) {
      $form['logs']['responses'][$log->dmlid]['response_state'][$log->dmrlid] = array('#value' => $log->response_state);
      $form['logs']['responses'][$log->dmlid]['r_processed'][$log->dmrlid] = array('#value' => $log->r_processed);
      $form['logs']['responses'][$log->dmlid]['created_result_count'][$log->dmrlid] = array('#value' => $log->created_result_count);
      $form['logs']['responses'][$log->dmlid]['updated_result_count'][$log->dmrlid] = array('#value' => $log->updated_result_count);
      $form['logs']['responses'][$log->dmlid]['deleted_result_count'][$log->dmrlid] = array('#value' => $log->deleted_result_count);
      $form['logs']['responses'][$log->dmlid]['remaining_result_count'][$log->dmrlid] = array('#value' => $log->remaining_result_count);
      $form['logs']['responses'][$log->dmlid]['r_log_file'][$log->dmrlid] = array('#value' => $log->r_log_file);
    }
  }
  $form['#theme'] = 'data_monitoring_monitor';
  $form_state['storage']['monitor'] = &$monitor;
  return $form;

}
/**
 * Validation callback for response trigger.
 * @param $form
 * @param $form_state
 */
function _data_monitoring_form_monitor_preview_response_validate($form, &$form_state) {
  if ($form_state['values']['responses'] == '')
      form_set_error('', t('No response selected. Please select the radio button next to the response you would like to process.'));
}
/**
 * Submit callback for response trigger.
 * @param $form
 * @param $form_state
 */
function _data_monitoring_form_monitor_preview_response_submit($form, &$form_state) {
  // Get data monitoring include files from other modules.
  _data_monitoring_module_include('data_monitoring.inc');
  $monitor = &$form_state['storage']['monitor'];
  if (empty($monitor->observers[$form_state['values']['responses']])) {
    drupal_set_message(t("Sorry, I couldn't find the response you selected, which is weird because "
                       . "I'm sure it was here a minute ago. If you don't mind could you please try"
                       . "selecting and triggering the response again? If that doesn't work try going back "
                       . 'to the <a href="!url">monitoring logs list</a> and starting again. Sorry for being annoying.'
                       , array('!url' => 'admin/reports/data-monitoring/monitors')), 'error');
  }
  else {
    $response = &$monitor->observers[$form_state['values']['responses']];
    if ($response->getTriggerMode() != DATA_MONITORING_MANUAL_RESPONSE_TRIGGER &&
        $response->getTriggerMode() != DATA_MONITORING_AUTOMATED_RESPONSE_TRIGGER) {
      drupal_set_message(t("It looks like the response you selected doesn't have an automated procedure for me "
                         . "to run. That might be because some human intervention is required, or maybe tracking "
                         . "the monitor results is enough in this case... I'm not sure but I'm guessing if you "
                         . "take a look at the response description it will tell you a bit more. For now, I won't "
                         . "take any action."), 'status');
    }
    else {
      $monitor->clearResult();
      $monitor->setMonitorState(DATA_MONITORING_STATE_MANUAL_IMPLEMENTATION);
      $monitor->process();
      if (!$response->evaluateTriggerWhen($monitor->getResult())) {
        // TODO: We shouldn't have to evaluate again here, this should already have taken place during $monitor->process() analysis.
        // TODO: Replace use of evaluateTriggerWhen() with getResponseState() which should be DATA_MONITORING_STATE_PENDING if evaluateTriggerWhen = TRUE
        // TODO: Reverse conditional so that response is processed if ($response->getResponseState() == DATA_MONITORING_STATE_PENDING) else this status message
        drupal_set_message(t('Evaluation of monitor results indicates that the selected response '
                         . 'is no longer required. Response process was not initiated.'), 'status');
      }
      else {
        $response->setResponseState(DATA_MONITORING_STATE_PREVIEW);
        $response->preProcess($monitor);
        $response->setBatchProgressive(TRUE);
        $response->setBatchDestination('admin/reports/data-monitoring/response/' . $response->getDmrlid());
        $response->process($monitor);
        // If response process includes batch processing then everything
        // after this will point will not be executed. Post process will be called by
        // $response->batchProcessComplete() provided modules correctly call
        // the batchProcessComplete method in their batch finished function.
        $response->postProcess($monitor);
        unset($form_state['storage']);
        DataMonitoringFeedbackHandler::getInstance()->displayFeedback();
        $form_state['redirect'] = 'admin/reports/data-monitoring/response/' . $response->getDmrlid();
      }
    }
  }
}
