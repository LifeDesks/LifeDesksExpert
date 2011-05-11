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
function data_monitoring_form_response(&$form_state, $response) {
  // Make sure data monitoring include files from other modules are available.
  _data_monitoring_module_include('data_monitoring.inc');

  $monitor = new DataMonitoringMonitor();
  $monitor->loadMonitorByDmlid($response->getDmlid());
  $form['destination'] = array(
    '#value' => 'admin/reports/data-monitoring/monitor/' . $monitor->getDmid(),
  );
  if (!$monitor->getDmlid()) {
    drupal_set_message(t("Oops, I can't seem to find the log record for the selected response. "
                       . "If the response is still pending you can try running it again."));
    drupal_goto($form['destination']['#value']);
  }
  $form['#attributes'] = array('class' => 'data-monitoring');
  $form['description'] = array(
    '#prefix' => '<p>',
    '#value' => $response->getDescription(),
    '#suffix' => '</p>',
  );
  $form['module'] = array('#value' => $response->getModule());
  $form['m_name'] = array('#value' => $monitor->getName());
  $form['processed'] = array('#value' => $response->getProcessed());
  $form['created_result_count'] = array('#value' => $response->getCreatedResultCount());
  $form['updated_result_count'] = array('#value' => $response->getUpdatedResultCount());
  $form['deleted_result_count'] = array('#value' => $response->getDeletedResultCount());
  $form['remaining_result_count'] = array('#value' => $response->getRemainingResultCount());
  $form['log_file'] = array('#value' => $response->getLogFile());
  $form['response_state'] = array('#value' => $response->getResponseState());
  $form['trigger_mode'] = array('#value' => $response->getTriggerMode());

  if ($response->getResponseState() == DATA_MONITORING_STATE_PREVIEW) {
    $form['implement_response'] = array(
      '#type' => 'submit',
      '#value' => t('Implement response'),
      '#submit' => array('_data_monitoring_form_monitor_implement_response_submit'),
      '#validate' => array('_data_monitoring_form_monitor_implement_response_validate')
    );
    $form['cancel_response'] = array(
      '#type' => 'submit',
      '#value' => t('Return to monitor'),
      '#submit' => array('_data_monitoring_form_monitor_cancel_response_submit'),
      '#validate' => array('_data_monitoring_form_monitor_cancel_response_validate')
    );
  }
  elseif ($response->getResponseState() == DATA_MONITORING_STATE_COMPLETED) {
    $form['completed_response'] = array(
      '#type' => 'submit',
      '#value' => t('Done'),
      '#submit' => array('_data_monitoring_form_monitor_completed_response_submit'),
      '#validate' => array('_data_monitoring_form_monitor_completed_response_validate')
    );
  }

  $form['#theme'] = 'data_monitoring_response';
  $form_state['storage']['monitor'] = &$monitor;
  $form_state['storage']['response'] = &$response;
  return $form;
}
/**
 * Validate callback for implement response button.
 * @param $form
 * @param $form_state
 */
function _data_monitoring_form_monitor_completed_response_validate($form, &$form_state) {

}
/**
 * Submit callback for implement response button.
 * @param $form
 * @param $form_state
 */
function _data_monitoring_form_monitor_completed_response_submit($form, &$form_state) {
  // Get data monitoring include files from other modules.
  _data_monitoring_module_include('data_monitoring.inc');
  $dmid = $form_state['storage']['monitor']->getDmid();
  unset($form_state['storage']);
  $form_state['redirect'] = 'admin/reports/data-monitoring/monitor/' . $dmid;
}
/**
 * Validate callback for implement response button.
 * @param $form
 * @param $form_state
 */
function _data_monitoring_form_monitor_implement_response_validate($form, &$form_state) {

}
/**
 * Submit callback for implement response button.
 * @param $form
 * @param $form_state
 */
function _data_monitoring_form_monitor_implement_response_submit($form, &$form_state) {

  $monitor = &$form_state['storage']['monitor'];
  $response = &$form_state['storage']['response'];

  $response->setResponseState(DATA_MONITORING_STATE_MANUAL_IMPLEMENTATION);
  $response->preProcess($monitor);
  $response->setBatchProgressive(TRUE);
  $response->setBatchDestination($_GET['q']); // TODO: Investigate use of q in multilanguage environments http://drupal.org/node/46088
  $response->process($monitor);
  // If response process includes batch processing then everything
  // after this will point will not be executed. Post process will be called by
  // $response->batchProcessComplete() provided modules correctly call
  // the batchProcessComplete method in their batch finished function.
  $response->postProcess($monitor);
  unset($form_state['storage']);
}
/**
 * Validate callback for cancel response button.
 * @param $form
 * @param $form_state
 */
function _data_monitoring_form_monitor_cancel_response_validate($form, &$form_state) {

}
/**
 * Submit callback for cancel response button.
 * @param $form
 * @param $form_state
 */
function _data_monitoring_form_monitor_cancel_response_submit($form, &$form_state) {
  if (isset($form_state['storage']['monitor']) &&
      $form_state['storage']['response']->getResponseState() == DATA_MONITORING_STATE_PREVIEW) {
    DataMonitoringManager::deleteLogs(array($form_state['storage']['monitor']->getDmlid() => TRUE));
  }
  unset($form_state['storage']);
  $form_state['redirect'] = $form['destination']['#value'];
}
