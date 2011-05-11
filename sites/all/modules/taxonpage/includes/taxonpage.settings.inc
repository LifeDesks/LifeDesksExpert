<?php
// $Id: taxonpage.settings.inc,v 1.9 2008/01/18 16:23:57 dshorthouse Exp $

/**
 * @file
 * Admin form for the taxonpage module.
 */

function taxonpage_settings_form(&$form_state) {
  $module_path = drupal_get_path('module', 'taxonpage') .'/';
  drupal_add_css($module_path .'css/taxonpage.css');

  $form['#cache'] = FALSE;

  $form['taxonpage'] = array(
	'#type' => 'fieldset',
	'#title' => t('Settings'),
  );

  $form['taxonpage']['taxonpage_show_license'] = array(
	'#type' => 'checkbox',
	'#title' => t('Show authors and licensing attributes under every image, map, and chapter'),
	'#default_value' => variable_get('taxonpage_show_license',1),
  );

  $form['#submit'][] = 'taxonpage_settings_form_submit';
  
  return system_settings_form($form);
}

function taxonpage_settings_form_submit() {
	cache_clear_all();
}