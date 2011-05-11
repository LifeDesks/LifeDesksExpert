<?php
// $Id$
/**
 * @file DataMonitoring.inc
 * Contains base and shared classes for data monitoring.
 */
class DataMonitoring {

  public static function acceptVisitor(&$visitee, &$visitor) {
    $visitee->accept($visitor);
  }

}
abstract class DataMonitoringObservable {

  public $observers = array();

  public function getObservers() {
    return $this->observers;
  }
  public function attachObserver(DataMonitoringObserver $observer) {
    $this->observers[] = $observer;
  }

  public function detachObserver(DataMonitoringObserver $observer) {
    foreach ($this->observers as $i => $obs) {
      if ($obs == $observer) {
        unset($this->observers[$i]);
      }
    }
  }

  public function notifyObservers($event = NULL) {
    foreach ($this->observers as $observer) {
      $observer->update($this, $event);
    }
  }
  public function discoverObservers() {
    _data_monitoring_module_include('data_monitoring.inc');
    foreach (module_implements('data_monitoring_observers') as $module) {
      call_user_func($module . "_data_monitoring_observers", &$this);
    }
  }
}

interface DataMonitoringObserver {
  public function update(DataMonitoringObservable &$subject, $event = NULL);
}
interface DataMonitoringVisitee {
  public function accept(&$visitor);
}
interface DataMonitoringVisitor {
  public function visitMonitor(DataMonitoringMonitor &$monitor);
  public function visitResponseHandler(DataMonitoringResponseHandler &$response);
}

class DataMonitoringSessionHandler {

  function registerObject($object, $object_name) {
    if (!is_object($object)) return FALSE;
    if (empty($object_name)) return FALSE;
    return $_SESSION[$object_name] = $object;
  }
  function unregisterObject($object_name) {
    if (!$_SESSION[$object_name]) return FALSE;
    unset($_SESSION[$objName]);
  }
  function getObject($object_name) {
    if (!$_SESSION[$object_name]) return FALSE;
    return $_SESSION[$object_name];
  }
}
