<?php
// $Id$
/**
 * @file DataMonitoringManager.inc
 * Provides a class to manage multiple monitors.
 * DataMonitoringManager can process and delete multiple monitors and associated responses.
 * It also handles email reporting for automated monitoring.
 */
class DataMonitoringManager {

  protected $monitors;
  protected $monitors_processed_count;
  protected $monitors_cancelled_count;

  public function __construct() { }

  public function __destruct() {
    foreach ($this->monitors as &$monitor) {
      $monitor->__destruct();
      unset($monitor);
    }
    $fbh = DataMonitoringFeedbackHandler::getInstance();
    unset($fbh);
  }

  protected function setMonitors(&$monitors) {
    return $this->monitors = $monitors;
  }
  protected function getMonitors() {
    return $this->monitors;
  }
  public function addMonitor(&$monitor) {
    return $this->monitors[] = $monitor;
  }
  protected function setMonitorsProcessedCount($count) {
    return $this->monitors_processed_count = $count;
  }
  protected function getMonitorsProcessedCount() {
    return $this->monitors_processed_count;
  }
  protected function setMonitorsCancelledCount($count) {
    return $this->monitors_cancelled_count = $count;
  }
  protected function getMonitorsCancelledCount() {
    return $this->monitors_cancelled_count;
  }
  protected function monitorProcessed() {
    $this->setMonitorsProcessedCount($this->getMonitorsProcessedCount() + 1);
  }
  protected function monitorCancelled() {
    $this->setMonitorsCancelledCount($this->getMonitorsCancelledCount() + 1);
  }
  /**
   * Determine whether time to wait has passed since last monitoring run,
   * if so process monitors (will include and automated responses),
   * then auto send email report.
   */
  public function autoRunMonitors() {
    if ($this->hasMonitoringIntervalPassed()) {
      $this->processMonitors(DATA_MONITORING_STATE_AUTOMATED_IMPLEMENTATION);
      if ($this->getMonitorsProcessedCount() > 0) {
        // We don't send an email report if no monitors were processed successfully.
        $this->autoSendEmailReport();
      }
    }
    return array('monitors_total' => count($this->getMonitors()),
                 'monitors_cancelled' => $this->getMonitorsCancelledCount(),
                 'monitors_processed' => $this->getMonitorsProcessedCount());
  }

  /**
   * Run monitors, automated responses will be triggered if automated responses are allowed.
   */
  public function processMonitors($implementation_state) {
    $this->setMonitorsProcessedCount(0);
    $this->setMonitorsCancelledCount(0);

    $monitors = &self::discoverMonitors();
    $this->setMonitors($monitors);
    $this->writeMonitorRecords();
    foreach ($this->monitors as &$monitor) {
      $monitor->setMonitorState($implementation_state);
      $monitor->process();
      // If monitor was not cancelled then we assume successful process.
      ($monitor->getCancelMonitor()) ? $this->monitorCancelled() : $this->monitorProcessed();
    }
    if ($this->getMonitorsProcessedCount() > 0) {
      // We only update last monitoring run if at least one monitor process returns TRUE.
      $this->updateMonitoringLastRun();
    }
  }

  /**
   * Determine whether user configured time to wait since last monitoring run has passed.
   */
  public function hasMonitoringIntervalPassed() {
    $monitoring_interval = variable_get('data_monitoring_monitoring_interval', DATA_MONITORING_INTERVAL);
    $last_monitoring_run = variable_get('data_monitoring_last_monitoring_run', 0);
    return (time() - $last_monitoring_run) > ($monitoring_interval - 3600);
    // Subtract 1 hour from interval to ensure scheduling works when cron set to same interval, e.g. daily.
  }

  /**
   * Scan all modules for monitors.
   */
  public static function &discoverMonitors() {
    foreach (module_implements('data_monitoring_monitors') as $module) {
      $results = call_user_func($module . "_data_monitoring_monitors");
      if (!empty($results) && is_array($results)) {
        foreach ($results as &$monitor) {
          // Only monitors with query and id are allowed.
          if (!$monitor->id && !$monitor->getQuery()) {
            unset($monitor);
          }
          else {
            $monitor->setModule($module);
          }
        }
      }
    }
    return $results;
  }

  /**
   * Update monitors in database with newly discovered monitors.
   */
  protected function writeMonitorRecords() {
    $result = db_query("SELECT dmid, id, module FROM {data_monitoring_monitors}");
    $db_monitors = array();
    while ($row = db_fetch_object($result)) {
      $db_monitors[$row->id] = $row;
    }
    foreach ($this->getMonitors() as $i => $monitor) {
      if (array_key_exists($monitor->getId(), $db_monitors) && $db_monitors[$monitor->getId()]->module == $monitor->getModule()) {
        // Update monitor record.
        $this->monitors[$i]->setDmid($db_monitors[$monitor->getId()]->dmid);
        if (!drupal_write_record('data_monitoring_monitors', $monitor, array('dmid'))) return FALSE;
        unset($db_monitors[$monitor->getId()]);
      }
      else {
        // Insert new monitor.
        if (!drupal_write_record('data_monitoring_monitors', $monitor)) return FALSE;
      }
    }
    foreach ($db_monitors as $id => $row) {
      // Monitor in database doesn't match a discovered monitor.
      $db_monitor = db_fetch_object(db_query("SELECT * FROM {data_monitoring_monitors} WHERE id = '%s'", $id));
      $db_monitor->name = t("WARNING: MONITOR MAY BE DEPRECATED") . ' - ' . $db_monitor->name;
      drupal_write_record('data_monitoring_monitors', $db_monitor, array('dmid'));
    }
  }
  protected function updateMonitoringLastRun() {
    variable_set('data_monitoring_last_monitoring_run', time());
  }



  /**
   * Determines whether settings indicate that an email report should be sent,
   * if so it triggers the email build and send process. If email is sent it
   * triggers updates to last sent variables and so on.
   */
  public function autoSendEmailReport() {
    if ($this->isEmailReportingOn() && $this->hasReportingIntervalPassed()) {
      $sent = $this->sendReportEmail();
      if (!empty($sent['success'])) { // At least one report was sent. Failed messages $sent['failed'] will be recorded in watchdog.
        $this->updateReportEmailLastSent();
        return TRUE;
      }
    }
    return FALSE;
  }
  /**
   * Generate email report and send it.
   * Only sends email if a monitor has responses pending or responses that were implemented.
   * @return Boolean
   *     TRUE if email sent, FALSE if not.
   */
  public function sendReportEmail() {

    $to = variable_get('data_monitoring_mail_to', NULL);
    if (!$to) return FALSE;
    if (!$reply_to = variable_get('data_monitoring_mail_reply_to', NULL)) {
      $reply_to = variable_get('site_mail', ini_get('sendmail_from'));
    }
    $to = explode(',', $to);
    $params['monitors'] = $this->getMonitors();
    $params['attachments'] = $this->findMonitorAttachments();

    $sent = array('success' => array(), 'failed' => array());
    foreach ($to as $address) {
      if ($account = $this->findUserAccountByEmail($to)) {
        $language = user_preferred_language($account);
      }
      else {
        $language = $this->findLanguage();
      }
      $message = drupal_mail('data_monitoring',
                           'monitoring_report',
                           $address,
                           $language,
                           $params,
                           $reply_to,
                           TRUE);
      if (!$message['result']) {
        $sent['failed'][] = $address;
      }
      else {
        $sent['success'][] = $address;
      }
    }
    return $sent;
  }
  /**
   * Return data monitoring send mail setting.
   * @return Boolean
   *      TRUE if email reporting is on, FALSE if email reporting is off.
   */
  public function isEmailReportingOn() {
    return variable_get('data_monitoring_mail', DATA_MONITORING_MAIL);
  }
  /**
   * Determine whether data monitoring setting for time to wait between sending email reports has passed.
   * @return Boolean
   *     TRUE if reporting interval has passed and mail should be sent, FALSE otherwise.
   */
  public function hasReportingIntervalPassed() {
    $interval = variable_get('data_monitoring_reporting_interval', DATA_MONITORING_INTERVAL);
    $last_sent = variable_get('data_monitoring_last_reporting_run', 0);
    return (time() - $last_sent) > ($interval);
  }
  /**
   * Update last sent time to now.
   * Last sent variable records when the last email report was sent.
   */
  public function updateReportEmailLastSent() {
    variable_set('data_monitoring_last_reporting_run', time());
  }
  /**
   * Find language, either set by data monitoring module ior default for email report.
   */
  public function findLanguage() {
    $dm_language = variable_get('data_monitoring_language', DATA_MONITORING_LANGUAGE);
    $language_list = language_list();
    foreach ($language_list as $language) {
      if ($language->name == $dm_language) {
        return $language;
      }
    }
    global $language;
    return $language;
  }
  public function findUserAccountByEmail($email) {
    return user_load(array('mail' => $to));
  }
  public function findMonitorAttachments() {
    $attachments = array();
    $files = array();
    foreach ($this->getMonitors() as $monitor) {
      if ($monitor->getResponsesImplemented() || $monitor->getResponsesPending()) {
        $files[] = $monitor->getLogFile();
      }
    }
    if (!empty($files)) {
      $lh = new DataMonitoringLogHandler();
      $lh->setBasename('DATA_MONITORING_REPORT_' . gmdate("Y-m-d_H-i-s_T") . '.zip');
      if ($lh->generateArchive($files)) {
        $attachments[] = &$lh;
      }
    }
    return $attachments;
  }

  /**
   * Bulk delete monitor and response logs from the database and filesystem.
   * @param $logs
   *     Array where key is data_monitoring_log.dmlid and value is TRUE (or dmlid)
   *     or FALSE (or 0) where FALSE (or 0) means do not delete.
   * @param $age
   *     Time in seconds, logs older than this time will be deleted.
   */
  public static function deleteLogs($logs = array(), $age = NULL) {

    // TODO: Test delete logs hook.
    foreach (module_implements('data_monitoring_manager_delete_logs') as $module) {
      call_user_func($module .'_data_monitoring_manager_delete_logs', $logs, $age);
    }
    $where = array();
    foreach ($logs as $dmlid => $remove) {
      if (!$remove) {
        unset($logs[$dmlid]);
      }
      else {
        $logs[$dmlid] = $dmlid;
      }
    }
    $dmlids = implode(',', $logs);
    if ($dmlids) $where[] = "dml.dmlid IN (" . $dmlids . ")";
    $date = NULL;
    if (is_numeric($age) && $age > 0) {
      $date = time() - $age;
      $where[] = "dml.processed < " . $date;
    }
    $where = implode(' OR ', $where);

    $deleted = NULL;
    if ($where) {
      $result = db_query("SELECT dml.log_file AS mlf, dmrl.log_file AS rlf
                          FROM {data_monitoring_log} dml
                          LEFT JOIN {data_monitoring_response_log} dmrl ON dml.dmlid = dmrl.dmlid
                          WHERE (" . $where . ")");
      // First delete removes response logs and monitors.
      $deleted = db_affected_rows(db_query("DELETE dml, dmrl
                                            FROM {data_monitoring_log} dml,
                                            {data_monitoring_response_log} dmrl
                                            WHERE dml.dmlid = dmrl.dmlid
                                            AND (". $where . ")"));
      // Second delete removes monitors that don't have response log.
      $deleted += db_affected_rows(db_query("DELETE dml
                                             FROM {data_monitoring_log} dml
                                             WHERE " . $where));

      if ($deleted) {
        $files = array();
        while ($file = db_fetch_array($result)) {
          if (!empty($file['mlf'])) {
            $files[md5($file['mlf'])] = new stdClass();
            $files[md5($file['mlf'])]->filepath = $file['mlf'];
          }
          if (!empty($file['rlf'])) {
            $files[md5($file['rlf'])] = new stdClass();
            $files[md5($file['rlf'])]->filepath = $file['rlf'];
          }
        }
        DataMonitoringLogHandler::deleteFiles($files);
      }
    }
    return $deleted;

  }
}

