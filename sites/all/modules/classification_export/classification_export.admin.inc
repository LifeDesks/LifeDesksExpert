<?php

function classification_export_settings_form() {
	$form = array();
	
	$form['gna'] = array(
	  '#type' => 'fieldset',
	  '#title' => t('Global Names Architecture'),
	);
	
	$form['gna']['classification_export_gna_url'] = array(
	  '#type' => 'textfield',
	  '#title' => t('POST URL'),
	  '#description' => t('The URL to automatically POST published classifications'),
	  '#default_value' => variable_get('classification_export_gna_url', 'http://gnaclr.globalnames.org/classifications'),
	);
	
	$form['gna']['classification_export_gna_uuid'] = array(
		'#type' => 'textfield',
		  '#title' => t('UUID'),
		  '#description' => t('The UUID to identify the POSTed classification'),
		  '#default_value' => variable_get('classification_export_gna_uuid', ''),
	);
	
	return system_settings_form($form);
}