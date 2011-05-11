<?php
/**
 * MultiCurl class library is a PHP solution for work with MULTI CURL extension.
 * It provides to execute some parallel HTTP requests with limit of downloaded
 * size. Example: start 100 downloads with 2 parallel sessions, and get only
 * first 100 Kb per session.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * @author    Vadym Timofeyev <tvad@mail333.com> http://weblancer.net/users/tvv/
 * @copyright 2007 Vadym Timofeyev
 * @license   http://www.gnu.org/licenses/lgpl-3.0.txt
 * @version   1.06
 * @since     PHP 5.0
 * @example   examples/example.php How to use MultiCurl class library.
 */
abstract class MultiCurl {
    /**
     * Maximal number of CURL multi sessions, 10 sessions by default
     * @var integer
     */
    private $maxSessions = 10;

    /**
     * Maximal size limit for downloaded content, 10 Mb (10 * 1024 * 1024) by default
     * @var integer
     */
    private $maxSize = 10485760;

    /**
     * CURL options for all requests
     * @var array
     */
    private $curlOptions;

    /**
     * Current CURL multi sessions
     * @var array
     */
    private $sessions = array();

    /**
     * Class constructor. Setup primary parameters.
     * @param array $curlOptions CURL options for all URLs
     */
    public function __construct($curlOptions = array()) {
        // Set CURL options for all URLs
        $this->setCurlOptions($curlOptions);
    }

    /**
     * Class destructor. Close opened sessions.
     */
    public function __destruct() {
        foreach ($this->sessions as $i => $sess) {
            $this->destroySession($i);
        }
    }

    /**
     * Add new URL to query
     * @param mixed $url URL for downloading
     * @param array $curlOptions CURL options for current request
     */
    public function addUrl($url, $curlOptions = array()) {
        // Check URL
        if (!$url) {
            throw new Exception('URL is empty!');
        }

        // Check array of URLs
        if (is_array($url)) {
            foreach ($url as $s) {
                $this->addUrl($s, $curlOptions);
            }
            return;
        }

        // Check query
        while (count($this->sessions) == $this->maxSessions) {
            $this->checkSessions();
        }

        // Init new CURL session
        $ch = curl_init($url);
        foreach ($this->curlOptions as $option => $value) {
            curl_setopt($ch, $option, $value);
        }
        foreach ($curlOptions as $option => $value) {
            curl_setopt($ch, $option, $value);
        }

        // Init new CURL multi session
        $mh = curl_multi_init();
        curl_multi_add_handle($mh, $ch);
        $this->sessions[] = array($mh, $ch, $url);
        $this->execSession(array_pop(array_keys($this->sessions)));
    }

    /**
     * Wait CURL milti sessions
     */
    public function wait() {
        while (count($this->sessions)) {
            $this->checkSessions();
        }
    }

    /**
     * Execute all active CURL multi sessions
     */
    protected function checkSessions() {
        foreach ($this->sessions as $i => $sess) {
            if (curl_multi_select($sess[0]) != -1) {
                $this->execSession($i);
            }
        }
    }

    /**
     * Execute CURL multi session, ñheck session status and downloaded size
     * @param integer $i A session id from array $this->sessions
     */
    protected function execSession($i) {
        list($mh, $ch,) = $this->sessions[$i];

        while (($mrc = curl_multi_exec($mh, $active)) == CURLM_CALL_MULTI_PERFORM);
        if (!$active || $mrc != CURLM_OK || curl_getinfo($ch, CURLINFO_SIZE_DOWNLOAD) >= $this->maxSize) {
            $this->closeSession($i);
        }
    }

    /**
     * Close session
     * @param integer $i A session id from array $this->sessions
     */
    protected function closeSession($i) {
        list(, $ch, $url) = $this->sessions[$i];
        $content = !curl_error($ch) ? curl_multi_getcontent($ch) : null;
        $info = curl_getinfo($ch);    
        $this->destroySession($i);
        $this->onLoad($url, $content, $info);
    }

    /**
     * Destroy session
     * @param integer $i A session id from array $this->sessions
     */
    protected function destroySession($i) {
        list($mh, $ch,) = $this->sessions[$i];
        curl_multi_remove_handle($mh, $ch);
        curl_close($ch);
        curl_multi_close($mh);
        unset($this->sessions[$i]);
    }

    /**
     * Get maximal number of CURL multi sessions
     * @return integer Maximal number of CURL multi sessions
     */
    public function getMaxSessions() {
        return $this->maxSessions;
    }

    /**
     * Set maximal number of CURL multi sessions
     * @param integer $maxSessions Maximal number of CURL multi sessions
     */
    public function setMaxSessions($maxSessions) {
        if ((int)$maxSessions <= 0) {
            throw new Exception('Max sessions number must be bigger then zero!');
        }
        $this->maxSessions = (int)$maxSessions;
    }

    /**
     * Get maximal size limit for downloaded content
     * @return integer Maximal size limit for downloaded content
     */
    public function getMaxSize() {
        return $this->maxSize;
    }

    /**
     * Set maximal size limit for downloaded content
     * @param integer $maxSize Maximal size limit for downloaded content
     */
    public function setMaxSize($maxSize) {
        if ((int)$maxSize <= 0) {
            throw new Exception('Max size limit must be bigger then zero!');
        }
        $this->maxSize = (int)$maxSize;
    }

    /**
     * Get CURL options for all requests
     * @return array CURL options
     */
    public function getCurlOptions() {
        return $this->curlOptions;
    }

    /**
     * Set CURL options for all requests
     * @param array $curlOptions CURL options
     */
    public function setCurlOptions($curlOptions) {
        if (!array_key_exists(CURLOPT_FOLLOWLOCATION, $curlOptions)) {
            $curlOptions[CURLOPT_FOLLOWLOCATION] = 1;
        }
        $curlOptions[CURLOPT_RETURNTRANSFER] = 1;
        $this->curlOptions = $curlOptions;
    }

    /**
     * OnLoad callback event
     * @param string $url URL for downloading
     * @param string $content Downloaded content
     * @param array $info CURL session information
     */
    protected abstract function onLoad($url, $content, $info);

    /**
     * Check CURL extension, etc.
     */
    public static function checkEnvironment() {
        if (!extension_loaded('curl')) {
            throw new Exception('CURL extension not loaded');
        }
    }
}
?>