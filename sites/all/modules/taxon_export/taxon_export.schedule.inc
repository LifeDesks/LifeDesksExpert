<?php

/**
 * Scheduled Taxon Export export form
 */
function taxon_export_schedule() {
  $module_path = drupal_get_path('module', 'taxon_export') .'/';
  drupal_add_css($module_path .'css/taxon_export.css');

  $chapter = db_fetch_object(db_query("SELECT tid FROM {taxon_export_map} LIMIT 1"));
  if(!$chapter->tid) {
  	$message = t('Sorry, before you can export your content for the Encyclopedia of Life, you must first map your chapter headings.');
  	$form['message'] = array(
  	  '#type' => 'markup',
  	  '#value' => '<div class="messages custom_error">' . $message . '</div>',
  	);
  	return $form;
  }

  $form['schedule_export'] = array('#type' => 'fieldset', '#title' => t('Encyclopedia of Life Content Partner Schedule'));
  
  $form['schedule_export']['taxon_export_cron_enable'] = array(
    '#type' => 'select', 
    '#title' => t('Enable automated re-generation of your content partner endpoint'),
    '#default_value' => variable_get('taxon_export_cron_enable', 1),
    '#options' => array('1' => t('Yes'), '0' => t('No')),
  );

  $form['schedule_export']['taxon_export_cron_interval'] = array(
    '#type' => 'textfield', 
    '#title' => t('Time interval (days)'),
    '#default_value' => variable_get('taxon_export_cron_interval', 7),
    '#size' => 5, '#maxlength' => 10,
    '#description' => t('Minimum number of days between the times your content partner endpoint is re-generated. The endpoint will actually be generated upon the first page request once the interval has elapsed.')
  );

  return system_settings_form($form);

}