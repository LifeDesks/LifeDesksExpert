<?php
// $Id$
/**
 * @file
 * Menu callback for Data monitoring administration settings form.
 */

/**
 * Menu callback for Data monitoring settings form.
 */
function data_monitoring_settings() {

  $form['#attributes'] = array('class' => 'data-monitoring');

  // Time intervals.
  $day = 86400;
  $week = $day * 7;
  $interval = array(0 => t('Every time'));
  $interval += drupal_map_assoc(array($day, 2 * $day, 3 * $day, 4 * $day, 5 * $day, 6 * $day, $week, 1 * $week, 2 * $week, 3 * $week, 4 * $week, 8 * $week, 16 * $week, 24 * $week), 'format_interval');

  $language_list = language_list();
  foreach ($language_list as $language) {
    $languages[] = $language->name;
  }

  // Intervals, reporting methods.
  $form['data_monitoring_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Monitoring and reporting settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['data_monitoring_settings']['data_monitoring_monitoring_interval'] = array(
    '#type' => 'select',
    '#title' => t('Select monitoring interval'),
    '#description' => t('The time between running the monitoring monitors. Monitors will be run on next cron run after interval has passed.'),
    '#options' => $interval,
    '#default_value' => variable_get('data_monitoring_monitoring_interval', DATA_MONITORING_INTERVAL),
  );
  $expire = array_slice($interval, 7, count($interval) - 7, TRUE);
  $expire[0] = t('Never');

  $form['data_monitoring_settings']['data_monitoring_log_expire'] = array(
    '#type' => 'select',
    '#title' => t('Select log expiration'),
    '#description' => t('Choose how long log files should be kept for.'),
    '#options' => $expire,
    '#default_value' => variable_get('data_monitoring_log_expire', DATA_MONITORING_LOG_EXPIRE),
  );
  $form['data_monitoring_settings']['data_monitoring_mail'] = array(
    '#type' => 'checkbox',
    '#title' => t('Send mail report'),
    '#description' => t('Check to send a report by mail.'),
    '#default_value' => variable_get('data_monitoring_mail', DATA_MONITORING_MAIL),
  );
  $form['data_monitoring_settings']['data_monitoring_mail_to'] = array(
    '#type' => 'textarea',
    '#title' => t('Send mail to'),
    '#description' => t('Comma separated list of email addresses to which reports should be sent.'),
    '#default_value' => variable_get('data_monitoring_mail_to', ''),
  );
  $form['data_monitoring_settings']['data_monitoring_mail_reply_to'] = array(
    '#type' => 'textfield',
    '#title' => t('Reply to'),
    '#description' => t('Enter an email address to which recipients of reports can reply to. If you do not specify an address the default site mail address will be used.'),
    '#default_value' => variable_get('data_monitoring_mail_reply_to', ''),
  );
  $form['data_monitoring_settings']['data_monitoring_reporting_interval'] = array(
    '#type' => 'select',
    '#title' => t('Select mail frequency.'),
    '#description' => t('The time between sending reports by mail. Reports will be logged to file each time monitor is run.'),
    '#options' => $interval,
    '#default_value' => variable_get('data_monitoring_reporting_interval', DATA_MONITORING_INTERVAL),
  );
  $form['data_monitoring_settings']['data_monitoring_language'] = array(
    '#type' => 'select',
    '#title' => t('Select report language.'),
    '#description' => t('The language in which reports should be logged and mailed.'),
    '#options' => $languages,
    '#default_value' => variable_get('data_monitoring_language', DATA_MONITORING_LANGUAGE),
  );
  $form['data_monitoring_settings']['data_monitoring_respond'] = array(
    '#type' => 'checkbox',
    '#title' => t('Trigger response procedures.'),
    '#description' => t('Check to automatically trigger response procedures on each monitoring run. Only monitors with response triggers set to "automatic" will implment attached response procedures.'),
    '#default_value' => variable_get('data_monitoring_respond', DATA_MONITORING_RESPOND),
  );

  return system_settings_form($form);
}
