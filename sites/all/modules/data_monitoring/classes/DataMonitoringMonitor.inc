<?php
// $Id$
/**
 * @file DataMonitoringObservable.inc
 * Provides the observable class Monitor.
 */

class DataMonitoringMonitor extends DataMonitoringObservable implements DataMonitoringVisitee {

  protected $cancel_monitor;

  public $dmid;
  public $dmlid;
  public $id;
  public $name;
  public $description;
  public $query;
  public $module;
  public $processed;
  public $result;
  public $result_count;
  public $log_file;
  public $responses_pending = 0;
  public $responses_implemented = 0;
  public $monitor_state;

  public function __construct($id = NULL, $module = NULL) {
    $this->setId($id);
    $this->setMonitorState(DATA_MONITORING_STATE_PENDING);
  }
  public function __destruct() { }
  public function __sleep() {
    $this->notifyObservers(DATA_MONITORING_EVENT_SLEEPING);
    return array_keys(get_object_vars($this));
  }
  public function __wakeup() {
    $this->notifyObservers(DATA_MONITORING_EVENT_WAKING);
  }
  public function accept(&$visitor) {
    $visitor->visitMonitor($this);
  }

  public function getCancelMonitor() {
    return $this->cancel_monitor;
  }
  protected function setCancelMonitor($cancel) {
    return $this->cancel_monitor = $cancel;
  }
  public function CancelMonitor() {
    $this->setCancelMonitor(TRUE);
    $this->setMonitorState(DATA_MONITORING_STATE_CANCELLED);
    $this->notifyObservers(DATA_MONITORING_EVENT_CANCELLING);
  }
  /**
   * Data monitoring monitor database ID.
   */
  public function setDmid($dmid) {
    return $this->dmid = $dmid;
  }
  public function getDmid() {
    return $this->dmid;
  }
  /**
   * Data monitoring monitor log database ID.
   */
  public function setDmlid($dmlid) {
    return $this->dmlid = $dmlid;
  }
  public function getDmlid() {
    return $this->dmlid;
  }

  /**
   * Unique identifier for monitor, allowed values [a-Z]_-
   */
  public function setId($id) {
    if (preg_match('/^([a-z]|[0-9]|_|-)*$/i', $id)) {
      return $this->id = $id;
    }
    else {
      DataMonitoringFeedbackHandler::getInstance()->addFeedback('Invalid identifier: %identifier. Identifiers should be plain text without special characters or spaces.', 'error', array('%identifier' => $id));
      return FALSE;
    }
  }
  public function getId() {
    return $this->id;
  }

  /**
   * Timestamp, time the monitor was processed.
   */
  public function setProcessed($time = NULL) {
    if (empty($time)) $time = time();
    return $this->processed = $time;
  }
  public function getProcessed() {
    return $this->processed;
  }

  /**
   * Plain text user friendly name summarising what is being monitored.
   */
  public function setName($name) {
    return $this->name = $name;
  }
  public function getName() {
    return $this->name;
  }

  /**
   * Plain text user friendly description of what is being monitored.
   */
  public function setDescription($description) {
    return $this->description = $description;
  }
  public function getDescription() {
    return $this->description;
  }

  /**
   * SELECT query that will be run against the database to give the monitor result.
   */
  public function setQuery($query) {
    $query = trim($query);
    if (preg_match('/^select/i', $query)) {
      return $this->query = $query;
    }
    else {
      // Invalid query, don't want to be modifying data with a monitoring query.
      DataMonitoringFeedbackHandler::getInstance()->addFeedback('Monitoring query was deemed invalid, and could not be set. Only SELECT queries are valid.', 'error');
      return NULL;
    }
  }
  public function getQuery() {
    if (isset($this->query)) {
      $q = @unserialize($this->query);
      if ($q !== FALSE) {
        return $q;
      }
      return $this->query;
    }
    return NULL;
  }

  /**
   * Name of module that defined this monitor.
   */
  public function setModule($module) {
    return $this->module = $module;
  }
  public function getModule() {
    return $this->module;
  }

  /**
   * Monitor query results as array.
   * Array of results
   *  - header array of columns - column array($column => $column)
   *  - rows array of rows - row => array($column => $value)
   *  - count
   */
  public function setResult($result) {
    return $this->result = $result;
  }
  public function getResult() {
    return $this->result;
  }
  /**
   * Count of records returned by monitor query.
   * @param Integer $count
   */
  public function setResultCount($count) {
    return $this->result_count = $count;
  }
  public function getResultCount() {
    return $this->result_count;
  }
  public function setLogFile($filepath) {
    return $this->log_file = $filepath;
  }
  public function getLogFile() {
    return $this->log_file;
  }

  /**
   * Count of pending responses - i.e. response deemed to be required
   * but not yet implemented for this monitor log.
   * Used in conjunction with responses implemented to help determine
   * what response links to show in monitor log interface.
   */
  public function setResponsesPending($count) {
    return $this->responses_pending = $count;
  }
  public function getResponsesPending() {
    return $this->responses_pending;
  }

  /**
   * Count of responses that have ben implemented for this monitor log.
   * Used in conjunction with responses pending to help determine
   * what response links to show in monitor log interface.
   */
  public function setResponsesImplemented($count) {
    return $this->responses_implemented = $count;
  }
  public function getResponsesImplemented() {
    return $this->responses_implemented;
  }

  public function setMonitorState($state) {
    return $this->monitor_state = $state;
  }
  public function getMonitorState() {
    return $this->monitor_state;
  }

  /**
   * Increment responses waiting count.
   */
  public function responsePending() {
    $this->setResponsesPending($this->getResponsesPending() + 1);
  }
  /**
   * Increment responses implemented count.
   * Decrement responses pending count.
   */
  public function responseImplemented() {
    $this->setResponsesPending($this->getResponsesPending() - 1);
    $this->setResponsesImplemented($this->getResponsesImplemented() + 1);
  }

  /**
   * Process monitor.
   * @return
   *    Boolean TRUE if monitor processed and result set, FALSE/NULL if error occurred.
   */
  public function process() {
    $this->preProcess();

    if ($this->getCancelMonitor()) return FALSE;

    $result = $this->runQuery();
    if (!$result) return FALSE;
    $this->setResult($result);
    $this->setResultCount($result['count']);
    $this->setProcessed();
    $this->postProcess();
  }
  /**
   * Allows modules to alter monitor object prior to it being processed
   * by calling hook_data_monitoring_monitor_pre_process().
   */
  public function preProcess() {
    $this->setCancelMonitor(FALSE);
    if (!$this->getObservers()) $this->discoverObservers();
    foreach (module_implements('data_monitoring_monitor_pre_process') as $module) {
      call_user_func($module .'_data_monitoring_monitor_pre_process', &$this);
    }
  }
  /**
   * Allows modules to alter monitor object after it has been processed
   * and before response procedures are called
   * by calling hook_data_monitoring_monitor_post_process().
   */
  public function postProcess() {
    if ($this->getCancelMonitor()) return FALSE;
    foreach (module_implements('data_monitoring_monitor_post_process') as $module) {
      call_user_func($module .'_data_monitoring_monitor_post_process', &$this);
    }
    $this->notifyObservers(DATA_MONITORING_EVENT_ANALYSING);

    // If monitor has returned results we want to temporarily store the details even if a response is not pending.
    // However user will only be able to see these temporary details when doing a manual preview of the monitor.
    // Permanent file is only saved if response is pending.
    if ($this->getResultCount()) {
      $log = &new DataMonitoringLogHandler();
      DataMonitoring::acceptVisitor($this, $log);
      $log->generateLogFile();

      // We only create a permanent file if we are in an implementation phase and a response is pending and
      // monitoring hasn't been cancelled.
      if (($this->getMonitorState() == DATA_MONITORING_STATE_AUTOMATED_IMPLEMENTATION ||
          $this->getMonitorState() == DATA_MONITORING_STATE_MANUAL_IMPLEMENTATION) &&
          !$this->getCancelMonitor() &&
          $this->getResponsesPending()) {
        $log_saved = $log->saveLogFile();
        if (!$log_saved) $this->CancelMonitor();
        unset ($log_saved);
      }
      $this->setLogFile($log->getFilepath());
      unset($log);
    }
    // We only write record to database if we are in implementation phase and if cancel monitoring is FALSE.
    if ($this->getMonitorState() == DATA_MONITORING_STATE_AUTOMATED_IMPLEMENTATION ||
        $this->getMonitorState() == DATA_MONITORING_STATE_MANUAL_IMPLEMENTATION &&
        !$this->getCancelMonitor()) {
      if ($this->getResponsesPending()) {
        $this->writeMonitorLogRecord();
        $this->notifyObservers(DATA_MONITORING_EVENT_SAVING);
      }
      $this->setMonitorState(DATA_MONITORING_STATE_COMPLETED);
    }
  }

  /**
   * Return array of results from the monitor query.
   */
  public function runQuery() {
    $result = array();
    $records = db_query($this->getQuery());
    if (!$records) {
      DataMonitoringFeedbackHandler::getInstance()->addFeedback('Monitor %name query returned error, check watchdog for errors.', 'error', array('%name' => $this->getName()));
      return NULL;
    }
    while ($row = db_fetch_array($records)) {
      if (empty($result['header'])) {
        foreach ($row as $head => $value) {
          $result['header'][$head] = $head;
        }
      }
      $result['rows'][] = $row;
    }
    // Monitor queries can be counts or return full datasets.
    // The following regex determines whether the result is a count or dataset and sets result count accordingly.
    if (preg_match("/^SELECT\s+(\(?COUNT\([^\)]+\)(?:\s+-\s+COUNT\([^\)]+\))?\)?)\s(?:AS\s+)?([^\s]+)?\s*FROM .+$/i", $this->getQuery(), $matches)) {
      // TODO: Use preg_match results to set count instead of running query again e.g. $result['row'][0][$matches[1]]
      $result['count'] = db_result(db_query($this->getQuery()));
    }
    else {
      $result['count'] = (isset($result['rows']) && is_array($result['rows'])) ? count($result['rows']) : 0;
    }
    return $result;
  }

  /**
   * Delete monitor log record from database and filesystem.
   */
  public function deleteMonitorLogRecord() {

    $this->notifyObservers(DATA_MONITORING_EVENT_DELETING);
    $deleted = db_affected_rows(db_query("DELETE dml
                                          FROM {data_monitoring_log} dml
                                          WHERE dmlid = %d", $this->getDmlid()));
    if ($deleted) {
      $log = new DataMonitoringLogHandler();
      $log->calculateFileParameters($this->getLogFile());
      $log->deleteFiles(array($log));
      $message = format_plural($deleted, '1 monitor log was deleted.', '@count monitor logs were deleted.');
      DataMonitoringFeedbackHandler::getInstance()->addFeedback($message, 'status');
      unset($log);
    }
    else {
      DataMonitoringFeedbackHandler::getInstance()->addFeedback("Sorry, I was unable to remove the monitor log record with id (dmlid) !dmlid. "
                                                              . "Please, try again. If the problem persists you might need to get a developer "
                                                              . "to take a look at my delete query for the monitor log record, sorry about that.",
                                                              'error', array('!dmlid' => $this->getDmlid()));
    }
  }

  /**
   * Write monitor log record to database.
   */
  public function writeMonitorLogRecord() {
    if (empty($this->dmid)) $this->setDmid(db_result(db_query("SELECT dmid FROM {data_monitoring_monitors} WHERE id = '%s' AND module = '%s'", $this->getId(), $this->getModule())));
    if (!empty($this->dmid)) {
      if (empty($this->dmlid)) {
        // Insert new monitor log record.
        drupal_write_record('data_monitoring_log', $this);
      }
      else {
        // Update existing monitor log record.
        drupal_write_record('data_monitoring_log', $this, 'dmlid');
      }
    }
    else {
      DataMonitoringFeedbackHandler::getInstance()->addFeedback('Could not find matching monitor record in database. Results could not be logged. Suggests sync between monitors described in code and those stored in database is not working as expected.', 'error');
    }
  }

  /**
   * Populate object with monitor log data.
   * Used to populate the object with a complete, already processed monitor log record.
   * @param $dmlid
   *     Integer
   */
  public function loadMonitorByDmlid($dmlid) {
    $dml = db_fetch_object(db_query("SELECT dml.*, dmm.id, dmm.module, dmm.name, dmm.description, dmm.query
                FROM {data_monitoring_log} dml
                LEFT JOIN {data_monitoring_monitors} dmm ON dmm.dmid = dml.dmid
                WHERE dml.dmlid = %d", $dmlid));
    foreach ($dml as $key => $val) {
      $v = @unserialize($val);
      if ($v !== FALSE) $val = $v;
      $this->$key = $val;
    }
    if ($this->getProcessed()) $this->setMonitorState(DATA_MONITORING_STATE_COMPLETED);
  }
  /**
   * Populate object with monitor data from database.
   * Used to populate the object with the monitor metadata and query ready for processing.
   * @param $dmid
   *     Integer
   */
  public function loadMonitorByDmid($dmid) {
    $dmm = db_fetch_object(db_query("SELECT * FROM {data_monitoring_monitors} dmm WHERE dmm.dmid = %d", $dmid));
    if (!$dmm) return FALSE;
    foreach ($dmm as $key => $val) {
      $v = @unserialize($val);
      if ($v !== FALSE) $val = $v;
      $this->$key = $val;
    }
    return TRUE;
  }

  public function clearResult() {
    unset($this->result);
    unset($this->result_count);
    unset($this->responses_implemented);
    unset($this->responses_pending);
    unset($this->log_file);
    unset($this->processed);
  }
}
