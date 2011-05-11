<?php
// $Id$
/**
 * @file DataMonitoringFeedbackHandler.inc
 * Contains feedback handler class which follows a singleton design pattern.
 * DataMonitoringFeedbackHandler manages error and status messages and reporting to
 * watchdog and displaying to user.
 */
class DataMonitoringFeedbackHandler  {

  private static $fbh_singleton;

  protected $feedback = array();

  private function __construct() { }
  private function __clone() { }
  public function __destruct() { }

  public static function getInstance() {
    if (!isset(self::$fbh_singleton)) {
      self::$fbh_singleton = new DataMonitoringFeedbackHandler();
    }
    return self::$fbh_singleton;
  }

  /**
   * Appends feedback string and replacement vars to feedback array.
   * @param $message
   *     String plain text, untranslated.
   * @param $type
   *     String type of message, use drupal_set_message types, i.e. 'error', 'status', 'warning'
   * @param $vars
   *     Array (optional) replacement variables for feedback string.
   */
  public function addFeedback($message, $type, $vars = array()) {
    return $this->feedback[$type][] = array('message' => $message, 'vars' => $vars);
  }
  public function getFeedback() {
    return $this->feedback;
  }

  /**
   * Send feedback to drupal_set_message for display on screen.
   */
  public function displayFeedback() {
    if (is_array($this->feedback)) {
      foreach ($this->feedback as $type => $messages) {
        foreach ($messages as $message) {
          drupal_set_message(t($message['message'], $message['vars']), $type, FALSE);
        }
      }
    }
  }
  /**
   * Send errors to drupal watchdog table.
   */
  public function logFeedback() {
    if (is_array($this->feedback)) {
      foreach ($this->feedback as $type => $messages) {
        foreach ($messages as $message) {
          if ($type == 'error') watchdog('data_monitoring', $message['message'], $message['vars'], WATCHDOG_ERROR);
        }
      }
    }
  }

}
