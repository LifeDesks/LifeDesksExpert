<?php
// $Id$
/**
 * @file DataMonitoringLogHandler.inc
 * Contains log file handler class which follows a vistor design pattern.
 * DataMonitoringLogHandler class is responsible for generating, saving and deleting log files.
 */
class DataMonitoringLogHandler implements DataMonitoringVisitor {

  public $content;
  public $basename;
  public $filepath;
  public $filemime;
  public $filesize;
  public $charset;

  public function __construct() {

  }
  public function visitMonitor(DataMonitoringMonitor &$monitor) {
    $this->setContent(theme('data_monitoring_monitor_details_plain', $monitor));
    $this->setBasename('MONITOR_LOG_' . drupal_strtoupper($monitor->getId()) . '_' . date("Y-m-d_H-i-s_T", $monitor->getProcessed()) . '.txt');
  }
  public function visitResponseHandler(DataMonitoringResponseHandler &$response) {
    if ($response->getProcessLog()) {
      if ($response->getProcessLogColumns() && is_array($response->getProcessLogColumns())) {
        $content = implode("\t", $response->getProcessLogColumns()) . "\n";
      }
      $content .= file_get_contents($response->getProcessLog());
    }
    $this->setContent(theme('data_monitoring_response_summary_plain', $response) . "\n\n" . $content);
    $this->setBasename('RESPONSE_LOG_' . drupal_strtoupper($response->getId()) . '_' . date("Y-m-d_H-i-s_T", $response->getProcessed()) . '.txt');
  }


  protected function setContent($content) {
    $this->content = $content;
  }
  public function getContent() {
    return $this->content;
  }
  public function setBasename($basename) {
    $this->basename = $basename;
  }
  public function getBasename() {
    return $this->basename;
  }
  protected function setFilepath($filepath) {
    $this->filepath = $filepath;
  }
  public function getFilepath() {
    return $this->filepath;
  }
  protected function setFilemime($filemime) {
    $this->filemime = $filemime;
  }
  public function getFilemime() {
    return $this->filemime;
  }
  protected function setFilesize($filesize) {
    $this->filesize = $filesize;
  }
  public function getFilesize() {
    return $this->filesize;
  }
  protected function setCharset($charset) {
    $this->charset = $charset;
  }
  public function getCharset() {
    return $this->charset;
  }

  /**
   * Create a temporary file if content exists.
   * @return String
   *     Drupal file path if file was created, FALSE or NULL otherwise.
   */
  public function generateLogFile() {
    if (!$this->getContent()) return FALSE;
    // Generate file name and full path.
    $destination = file_create_filename($this->getBasename(), file_directory_temp());
    // Save file to temporary folder.
    $file = file_save_data($this->getContent(), $destination, FILE_EXISTS_RENAME);
    if ($file) {
      $this->calculateFileParameters($file);
      return $file;
    }
    return NULL;
  }

  /**
   * Given path to file calculate and set file parameters e.g. basename, size, mime type.
   * @param $file
   *    String Drupal file path.
   */
  public function calculateFileParameters($filepath) {
    $this->setFilepath($filepath);
    $this->setBasename(pathinfo($filepath, PATHINFO_BASENAME));
    $this->setFilemime(file_get_mimetype($filepath));
    $this->setFilesize(filesize($filepath));
    $this->setCharset(mb_detect_encoding($this->getContent()));
  }
  /**
   * Copy log file from temporary directory to data monitoring directory.
   */
  public function saveLogFile() {
    if (!$this->getFilepath()) return FALSE;
    if (!$this->getBasename()) $this->calculateFileParameters($this->getFilepath());
    if (!$this->getBasename()) return FALSE;
    if (!$directory = self::checkDirectory()) return FALSE;
    if (!$destination = file_create_filename($this->getBasename(), $directory)) return FALSE;
    if (file_move($this->filepath, $destination, FILE_EXISTS_RENAME)) {
      $this->calculateFileParameters($this->getFilepath());
      return $this->getFilepath();
    }
    return FALSE;
  }

  /**
   * Helper method to check data monitoring files directory exists, and if so, return path.
   * @return String
   *     Path to data monitoring files directory.
   */
  public static function checkDirectory() {
    // Check data monitoring file directory has been created.
    $files_dir_path = file_directory_path();
    if (file_check_directory($files_dir_path)) { // Check files directory exists.
      $directory = variable_get('data_monitoring_directory', '');
      if (empty($directory) || !file_check_directory($directory, FILE_MODIFY_PERMISSIONS)) {
        $directory = file_create_path(DATA_MONITORING_DIR);
        if (file_check_directory($directory, FILE_CREATE_DIRECTORY)) {
          variable_set('data_monitoring_directory', $directory);
          return $directory;
        }
        else {
          drupal_set_message(t('Data monitoring directory %dir could not be created, or is not writable. Log files cannot be saved.', array('%dir' => $directory)));
        }
      }
      else {
        return $directory;
      }
    }
    else {
      drupal_set_message(t('Files directory is not available. Files cannot be saved.'), 'error');
    }
    unset($files_dir_path);
    return FALSE;
  }

  /**
   * Delete log files that are older than the expire period specified in data monitoring settings.
   */
  public static function expireLogs() {
    if ($directory = self::checkDirectory()) {
      $expire = variable_get('data_monitoring_log_expire', DATA_MONITORING_LOG_EXPIRE);
      if (!empty($expire) && $expire > 0) {
        $logs = file_scan_directory($directory, '.*', array('.', '..', 'CVS'), 0, TRUE, 'basename');
        if (is_array($logs)) {
          return $this->deleteFiles($logs, $expire);
        }
      }
    }
    return;
  }
  /**
   * Given array of files objects and interval, delete files that have not been modified since the specified interval.
   * @param $files
   *     Array of file objects.
   * @param $interval
   *     Time passed in seconds after which an unmodified file can be deleted, i.e. delete if (now - last modified) > interval
   */
  public static function deleteFiles($files, $interval = NULL) {
    $deleted = 0;
    if (is_array($files)) {
      foreach ($files as $file) {
        if (empty($file->filepath)) {
          if (empty($file->filename)) continue;
          $file->filepath = $file->filename;
        }
        if (is_numeric($interval) && $interval > 0) {
          // Get last modified time of file.
          if ((time() - filemtime($file->filepath)) < $interval) continue;
        }
        if (file_delete($file->filepath)) {
            $deleted++;
        }
        else {
          watchdog('data_monitoring', "Data monitoring log handler could not delete file !filepath from the filesystem: "
                                    . "files_delete return FALSE.", array('!filepath' => $file->filepath), WATCHDOG_ERROR);
        }
      }
      return $deleted;
    }
    else {
      watchdog('data_monitoring', "Data monitoring log handler could not delete files from the filesystem. "
                                . "Files parameter is expected to be an array.", array(), WATCHDOG_ERROR);
    }
  }

  /**
   * Create a zip archive of files.
   */
  public function generateArchive($files) {

    if (empty($files)) return FALSE;

    $destination = file_create_filename($this->getBasename(), file_directory_temp());

    $zip = new ZipArchive();

    if ($zip->open($destination, ZipArchive::CREATE)) {
      foreach ($files as $file) {
        $zip->addFile($file, pathinfo($file, PATHINFO_BASENAME));
      }
      $zip->close();
      $this->calculateFileParameters($destination);
      return TRUE;
    }
    return FALSE;
  }

}
