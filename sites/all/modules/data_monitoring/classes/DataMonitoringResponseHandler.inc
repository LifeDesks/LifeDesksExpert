<?php

// $Id$
/**
 * @file DataMonitoringResponseHandler.inc
 * Provides the observer classes.
 */

class DataMonitoringResponseHandler implements DataMonitoringObserver, DataMonitoringVisitee {

  protected $cancel_process;

  public $dmrlid;
  public $dmlid;
  public $id;
  public $module;
  public $description;
  public $trigger_when;
  public $trigger_mode;
  public $processed;
  public $response_state;
  public $batch_progressive;
  public $batch_destination = NULL;
  public $process_log;
  public $process_log_columns;
  public $created_result_count;
  public $updated_result_count;
  public $deleted_result_count;
  public $remaining_result_count;
  public $log_file;

  public function __construct($module = NULL, $id = NULL, $description = NULL, $trigger_when = NULL, $trigger_mode = NULL) {
    $this->setId($id);
    $this->setDescription($description);
    $this->setTriggerWhen($trigger_when);
    $this->setTriggerMode($trigger_mode);
    $this->setModule($module);
  }
  public function __destruct() {  }
  /**
   * Allow another object to visit temporarily, and initialise the visitor by
   * running the corresponding method.
   * @see DataMonitoringVisitee::accept()
   */
  public function accept(&$visitor) {
    $visitor->visitResponseHandler($this);
  }

  /**
   * Notification has just been sent from subject under observation.
   * @see DataMonitoringObserver::update()
   */
  public function update(DataMonitoringObservable &$subject, $event = NULL) {

    if (get_class($subject) == 'DataMonitoringMonitor') {
      $this->setDmlid($subject->getDmlid());
      switch ($event) {
        case DATA_MONITORING_EVENT_ANALYSING:
          // Monitor has got results but not yet saved to database.
          // This is an opportunity to analyse monitor results and modify monitor before it is saved to database.
          if ($this->evaluateTriggerWhen($subject->getResult())) {
            $subject->responsePending();
            $this->setResponseState(DATA_MONITORING_STATE_PENDING);
          }
          break;

        case DATA_MONITORING_EVENT_SAVING:
          // Monitor has completed all processing and analysis and has been written to database with pending responses count.
          // Now we implement an automated responses only if automated allowed and monitor state is automated implementation.
          // If monitor has been cancelled monitor state should reflect this.
          if ($subject->getMonitorState() == DATA_MONITORING_STATE_AUTOMATED_IMPLEMENTATION &&
              $this->evaluateTriggerWhen($subject->getResult()) &&
              variable_get('data_monitoring_respond', DATA_MONITORING_AUTOMATED_DISALLOWED) == DATA_MONITORING_AUTOMATED_ALLOWED &&
              $this->getTriggerMode() == DATA_MONITORING_AUTOMATED_RESPONSE_TRIGGER) {
            // TODO: Replace evaluateTriggerWhen() with getResponseState() which should = DATA_MONITORING_STATE_PENDING if evaluateTriggerWhen = TRUE
            $this->setResponseState(DATA_MONITORING_STATE_AUTOMATED_IMPLEMENTATION);
            $this->preProcess($subject);
            $this->process($subject);
            $this->postProcess($subject);
          }
          break;

        case DATA_MONITORING_EVENT_DELETING:
          // Monitor log has been deleted, delete response log.
          $this->deleteResponseLogRecord();
          break;
      }
    }
  }
  protected function getCancelProcess() {
    return $this->cancel_process;
  }
  protected function setCancelProcess($cancel) {
    return $this->cancel_process = $cancel;
  }
  public function setDmlid($dmlid) {
    return $this->dmlid = $dmlid;
  }
  public function getDmlid() {
    return $this->dmlid;
  }
  public function setDmrlid($dmrlid) {
    return $this->dmlid = $dmrlid;
  }
  public function getDmrlid() {
    return $this->dmrlid;
  }
  /**
   * Optional identifier for use in processing.
   */
  public function setId($id) {
    return $this->id = $id;
  }
  public function getId() {
    return $this->id;
  }
  public function setModule($module) {
    return $this->module = $module;
  }
  public function getModule() {
    return $this->module;
  }
  /**
   * User friendly plain text description of expected behaviour of response procedure.
   */
  public function setDescription($description) {
    return $this->description = $description;
  }
  public function getDescription() {
    return $this->description;
  }

  /**
   * Timestamp and flag that indicates, if set, that this response has been processed.
   */
  public function setProcessed($time = NULL) {
    if (empty($time)) $time = time();
    return $this->processed = $time;
  }
  public function getProcessed() {
    return $this->processed;
  }

  /**
   * Describes the state the subject's results should be in to trigger a response.
   * @param $property
   *    String property to test, can be column alias or 'num_rows'.
   * @param $operator
   *    String possible values > < = !.
   * @param $value
   *    Variant comparison value.
   */
  public function setTriggerWhen($trigger_when) {

    $allowed_operators = "/^(<|>|=|!)$/i";
    $operator = trim($trigger_when['operator']);
    if (!preg_match($allowed_operators, $trigger_when['operator'])) return FALSE;
    return $this->trigger_when = serialize($trigger_when);
  }
  public function getTriggerWhen() {
    if (isset($this->trigger_when)) return unserialize($this->trigger_when);
    return NULL;
  }

  /**
   * Flag to determine whether a response should be triggered automatically.
   * Response will not be triggered automatically if automated responses are globally disallowed.
   * @see DATA_MONITORING_MANUAL_RESPONSE_TRIGGER
   * @see DATA_MONITORING_AUTOMATED_RESPONSE_TRIGGER
   */
  public function setTriggerMode($trigger_mode) {
    return $this->trigger_mode = $trigger_mode;
  }
  public function getTriggerMode() {
    return $this->trigger_mode;
  }

  /**
   * Response procedure state, pending, preview, completed - use constants:
   * @see DATA_MONITORING_STATE_PENDING
   * @see DATA_MONITORING_STATE_PREVIEW
   * @see DATA_MONITORING_STATE_AUTOMATED_IMPLEMENTATION
   * @see DATA_MONITORING_STATE_MANUAL_IMPLEMENTATION
   * @see DATA_MONITORING_STATE_COMPLETED
   */
  public function setResponseState($state) {
    return $this->response_state = $state;
  }
  public function getResponseState() {
    return $this->response_state;
  }
  public function setBatchProgressive($batch_progressive) {
    return $this->batch_progressive = $batch_progressive;
  }
  public function getBatchProgressive() {
    return $this->batch_progressive;
  }
  public function setBatchDestination($batch_destination) {
    return $this->batch_destination = $batch_destination;
  }
  public function getBatchDestination() {
    return $this->batch_destination;
  }
  public function setProcessLog($filepath) {
    return $this->process_log = $filepath;
  }
  public function getProcessLog() {
    return $this->process_log;
  }
  public function setProcessLogColumns(Array $columns) {
    return $this->process_log_columns = $columns;
  }
  public function getProcessLogColumns() {
    return $this->process_log_columns;
  }
  public function setCreatedResultCount($count) {
    return $this->created_result_count = $count;
  }
  public function getCreatedResultCount() {
    return $this->created_result_count;
  }
  public function incrementCreatedResultCount() {
    return $this->setCreatedResultCount($this->getCreatedResultCount() + 1);
  }
  public function setUpdatedResultCount($count) {
    return $this->updated_result_count = $count;
  }
  public function getUpdatedResultCount() {
    return $this->updated_result_count;
  }
  public function incrementUpdatedResultCount() {
    return $this->setUpdatedResultCount($this->getUpdatedResultCount() + 1);
  }
  public function setDeletedResultCount($count) {
    return $this->deleted_result_count = $count;
  }
  public function getDeletedResultCount() {
    return $this->deleted_result_count;
  }
  public function incrementDeletedResultCount() {
    return $this->setDeletedResultCount($this->getDeletedResultCount() + 1);
  }
  public function setRemainingResultCount($count) {
    return $this->remaining_result_count = $count;
  }
  public function getRemainingResultCount() {
    return $this->remaining_result_count;
  }
  public function incrementRemainingResultCount() {
    return $this->setRemainingResultCount($this->getRemainingResultCount() + 1);
  }
  public function setLogFile($filepath) {
    return $this->log_file = $filepath;
  }
  public function getLogFile() {
    return $this->log_file;
  }

  /**
   * Evaluate monitor results against trigger_when criteria.
   * For results with multiple rows TRUE will be returned if any row meets the trigger_when criteria.
   * @return
   *    Boolean TRUE if response should be triggered, FALSE if response should not be triggered and NULL if a problem occurred.
   */
  public function evaluateTriggerWhen($result) {
    $trigger_when = $this->getTriggerWhen();

    if (empty($trigger_when) || empty($trigger_when['property']) || empty($trigger_when['operator']) ||
          (!is_numeric($trigger_when['value']) && empty($trigger_when['value'])) ||
          (empty($result) ||
          (!is_numeric($result['count']) && empty($result['count']) && empty($result['rows']))) ||
          ($trigger_when['property'] == 'num_rows' && !is_numeric($trigger_when['value']))
       ) return NULL;

    $test = array();
    if ($trigger_when['property'] == 'num_rows') {
      $test[] = array($trigger_when['property'] => $result['count']);
    }
    elseif (!empty($result['rows'])) {
      $test = $result['rows'];
    }

    foreach ($test as $item) {
      switch ($trigger_when['operator']) {
        case '>':
          if ($item[$trigger_when['property']] > $trigger_when['value']) return TRUE;
        break;
        case '<':
          if ($item[$trigger_when['property']] < $trigger_when['value']) return TRUE;
        break;
        case '=':
          if ($item[$trigger_when['property']] === $trigger_when['value']) return TRUE;
        break;
        case '!':
          if ($item[$trigger_when['property']] !== $trigger_when['value']) return TRUE;
        break;
      }
    }
    return FALSE;
  }

  public function cancelProcess() {
    $this->setCancelProcess(TRUE);
  }
  /**
   * Calls hook_data_monitoring_response() for any module that implements it.
   * @param $monitor
   *     Object from DataMonitoringMonitor class.
   * @param $mode
   *     String flag to determine whether the response is a preview or implementation of response procedure
   */
  public function process(DataMonitoringMonitor &$monitor) {
    if ($this->getCancelProcess()) return FALSE;
    foreach (module_implements('data_monitoring_response_process') as $module) {
      call_user_func($module .'_data_monitoring_response_process', &$monitor, &$this);
    }
  }
  public function preProcess(DataMonitoringMonitor &$monitor) {
    // We create blank response database record here as the dmrlid is needed for the batch destination link
    // @see _data_monitoring_form_monitor_preview_response_submit()
    // We do this even if we are in preview mode. Later we remove the database record if it wasn't implemented.
    if (!$this->getDmrlid()) $this->writeResponseLogRecord();
    $this->setCancelProcess(FALSE);
    $this->setCreatedResultCount(0);
    $this->setUpdatedResultCount(0);
    $this->setDeletedResultCount(0);
    $this->setRemainingResultCount(0);
    foreach (module_implements('data_monitoring_response_pre_process') as $module) {
      call_user_func($module .'_data_monitoring_response_pre_process', &$monitor, &$this);
    }
  }
  public function postProcess(DataMonitoringMonitor &$monitor) {
    if ($this->getCancelProcess()) return FALSE;
    $this->setProcessed();

    foreach (module_implements('data_monitoring_response_post_process') as $module) {
      call_user_func($module .'_data_monitoring_response_post_process', &$monitor, &$this);
    }
    if ($this->getCancelProcess()) return FALSE;

    $log = new DataMonitoringLogHandler();
    DataMonitoring::acceptVisitor($this, $log);
    $log->generateLogFile();
    // We save the file to permanent file directory as it is going to get written to database record
    // regardless of whether its a preview or implementation phase. We later remove the file if response
    // isn't implemented.
    $log->saveLogFile();
    $this->setLogFile($log->getFilepath());
    unset($log);

    if ($this->getResponseState() == DATA_MONITORING_STATE_AUTOMATED_IMPLEMENTATION ||
        $this->getResponseState() == DATA_MONITORING_STATE_MANUAL_IMPLEMENTATION) {
      $monitor->responseImplemented();
      $monitor->writeMonitorLogRecord(); // Update monitor log record in database with pending/implemented count.
      $this->setResponseState(DATA_MONITORING_STATE_COMPLETED);
    }
    // Save response db record regardless of preview or implentation - we later delete preview if user cancels implementation.
    // TODO: May need some clean up for response db records in preview mode that haven't been deleted through normal user actions.
    $this->writeResponseLogRecord();
  }

  /**
   * Call this when a batch response process is complete.
   * Monitor and response objects will be stored in session for use after redirect.
   * @param DataMonitoringMonitor $monitor
   */
  public function batchProcessComplete(DataMonitoringMonitor &$monitor) {
    $this->postProcess($monitor);
    DataMonitoringFeedbackHandler::getInstance()->displayFeedback();
    DataMonitoringFeedbackHandler::getInstance()->logFeedback();
  }


  /**
   * Write monitor log record to database.
   */
  protected function writeResponseLogRecord() {
    if (empty($this->dmlid)) {
      DataMonitoringFeedbackHandler::getInstance()->addFeedback('Missing monitor log id, response could not be logged.', 'error');
      return FALSE;
    }
    if (empty($this->dmrlid)) {
      // Insert new response log record.
      if (!drupal_write_record('data_monitoring_response_log', $this))
        DataMonitoringFeedbackHandler::getInstance()->addFeedback('Response log could not be inserted into the database.', 'error');
    }
    else {
      // Update existing response log record.
      if (!drupal_write_record('data_monitoring_response_log', $this, 'dmrlid'))
        DataMonitoringFeedbackHandler::getInstance()->addFeedback('Response log database record could not be updated.', 'error');
    }
  }
  /**
   * Delete response log record.
   */
  protected function deleteResponseLogRecord() {
    db_affected_rows(db_query("DELETE dmrl
                               FROM {data_monitoring_response_log} dmrl
                               WHERE dmrlid = %d", $this->getDmrlid()));
    $files = array();
    $files[]->filepath = $this->getLogFile();
    DataMonitoringLogHandler::deleteFiles($files);
  }

  /**
   * Get data from database and populate object with the data.
   * @param $dmrlid
   */
  public function loadResponseByDmrlid($dmrlid) {
    $record = db_fetch_object(db_query("SELECT * FROM {data_monitoring_response_log} WHERE dmrlid = %d", $dmrlid));
    foreach ($record as $key => $val) {
      $v = @unserialize($val);
      if ($v !== FALSE) $val = $v;
      $this->$key = $val;
    }
  }

  public function appendRecordToProcessLog(Array $record) {
    if (!$this->getProcessLog()) {
      $this->setProcessLog(tempnam(realpath(file_directory_temp()), 'file'));
    }
    if (!$fh = fopen($this->getProcessLog(), 'a')) {
      DataMonitoringFeedbackHandler::getInstance()->addFeedback('Processed record could not be added to log file.', 'error');
      // TODO: Candidate for transaction rollback, i.e. if this returns false bail on processing records.
      return;
    }
    $columns = $this->getProcessLogColumns();
    if (!$columns) $columns = array();
    // Here we are checking for new columns to add to the output and converting any sub arrays to json.
    foreach ($record as $key => $value) {
      $column = trim($key);
      if (!in_array($column, $columns)) {
        $columns[] = $column;
      }
      if (is_array($value)) $record[$key] = json_encode($value);
      $record[$key] = _data_monitoring_remove_control_characters($record[$key]);
      if (is_null($record[$key])) $record[$key] = "NULL";
    }
    // Here we are matching up values in $record to each line in the order of the columns.
    foreach ($columns as $column) {
      if (isset($record[$column])) {
        $line[] = $record[$column];
        unset($record[$column]);
      }
    }
    // $record should now be empty, if not we have extra columns we haven't captured
    // before - this should never happen - but just in case we dump them in error message for debugging.
    if (!empty($record)) DataMonitoringFeedbackHandler::getInstance()->addFeedback(
        'Some data from a response record could not be accurately added to the response processing log. '
      . 'Extra data as json: %json', 'error', array('%json' => json_encode($record))
    );

    $this->setProcessLogColumns($columns);
    $line = implode("\t", $line) . "\n";
    fwrite($fh, $line);
    fclose($fh);
  }

  public function addModifiedRecordToProcessLog(Array $record, $notes) {
    $record['processing notes'] .= $notes;
    $this->appendRecordToProcessLog($record);
  }
  public function addOriginalRecordToProcessLog(Array $record) {
    $record['processing notes'] .= t('Unmodified original record.');
    $this->appendRecordToProcessLog($record);
  }
}
