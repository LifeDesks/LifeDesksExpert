<?php
// $Id$
/**
 * @file
 * Menu callbacks for Data monitoring multisite administration settings.
 */

/**
 * Menu callback for Data monitoring multisite settings form.
 */
function data_monitoring_multisite_settings() {

  // Time intervals.
  $day = 86400;
  $week = 604800;
  $interval = array(0 => t('Every time'));
  $interval += drupal_map_assoc(array($day, 2 * $day, 3 * $day, 4 * $day, 5 * $day, 6 * $day, $week, 1 * $week, 2 * $week, 3 * $week, 4 * $week), 'format_interval');

  $language_list = language_list();
  foreach ($language_list as $language) {
    $languages[] = $language->name;
  }

  $form = array();

  // TODO: Configure interface to change settings across all sites?

  // Configure aggregate reporting settings.
  $form['data_monitoring_multisite_aggregate_logging'] = array(
    '#type' => 'fieldset',
    '#title' => t('Aggregate logging settings'),
    '#description' => t('Configure logging settings for multisite data monitoring'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['data_monitoring_multisite_aggregate_logging']['data_monitoring_multisite_aggregate_logging_interval'] = array(
    '#type' => 'select',
    '#title' => t('Select aggregate logging interval'),
    '#description' => t('The time between aggregating monitoring logs. Aggregate logging will be executed on next cron run after interval has passed.'),
    '#options' => $interval,
    '#default_value' => variable_get('data_monitoring_multisite_aggregate_logging_interval', DATA_MONITORING_MULTISITE_INTERVAL),
  );
  $form['data_monitoring_multisite_aggregate_reporting'] = array(
    '#type' => 'fieldset',
    '#title' => t('Aggregate reporting settings'),
    '#description' => t('Configure reporting settings for multisite data monitoring.'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['data_monitoring_multisite_aggregate_reporting']['data_monitoring_multisite_aggregate_mail'] = array(
    '#type' => 'checkbox',
    '#title' => t('Send mail for aggregate reporting'),
    '#description' => t('Check to send mail reports of aggregate logs.'),
    '#default_value' => variable_get('data_monitoring_multisite_aggregate_mail', DATA_MONITORING_MULTISITE_MAIL),
  );
  $form['data_monitoring_multisite_aggregate_reporting']['data_monitoring_multisite_aggregate_mail_to'] = array(
    '#type' => 'textarea',
    '#title' => t('Send mail to'),
    '#description' => t('Comma separated list of email addresses to which aggregate reports should be sent.'),
    '#default_value' => variable_get('data_monitoring_multisite_aggregate_mail_to', ''),
  );
  $form['data_monitoring_multisite_aggregate_reporting']['data_monitoring_multisite_aggregate_mail_reply_to'] = array(
    '#type' => 'textfield',
    '#title' => t('Reply to'),
    '#description' => t('Enter an email address to which recipients of aggregate reports can reply to. If you do not specify an address the default site mail address will be used.'),
    '#default_value' => variable_get('data_monitoring_multisite_aggregate_mail_reply_to', ''),
  );
  $form['data_monitoring_multisite_aggregate_reporting']['data_monitoring_multisite_aggregate_reporting_interval'] = array(
    '#type' => 'select',
    '#title' => t('Select mail frequency.'),
    '#description' => t('The time between sending reports by mail. Reports will be logged to file each time aggregate reporting is run, even if email is not sent.'),
    '#options' => $interval,
    '#default_value' => variable_get('data_monitoring_multisite_aggregate_reporting_interval', DATA_MONITORING_MULTISITE_INTERVAL),
  );
  $form['data_monitoring_multisite_aggregate_reporting']['data_monitoring_multisite_aggregate_reporting_language'] = array(
    '#type' => 'select',
    '#title' => t('Select report language.'),
    '#description' => t('The language in which reports should be logged and mailed.'),
    '#options' => $languages,
    '#default_value' => variable_get('data_monitoring_multisite_aggregate_language', DATA_MONITORING_MULTISITE_LANGUAGE),
  );

  return system_settings_form($form);

}
