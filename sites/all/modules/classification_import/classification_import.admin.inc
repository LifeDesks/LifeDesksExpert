<?php

/**
* Implementation of hook_settings().
*/
function classification_import_admin_settings() {
  $form['import_limits'] = array(
    '#type' => 'fieldset',
    '#title' => t('Classification import limits'),
    '#description' => t('Set the limits for the number of names or rows that will be permitted. These settings may be adjusted for better performance.')
  );

  $form['import_limits']['classification_import_copypaste_limit'] = array(
	'#type' => 'textfield',
    '#title' => t('Textarea box limit'),
    '#default_value' => variable_get('classification_import_copypaste_limit', 1000),
    '#element_validate' => array('classification_import_textfield_validate'),
    '#required' => true,
  );

  $form['import_limits']['classification_import_file_limit'] = array(
    '#type' => 'textfield',
    '#title' => t('Row limit for uploaded files'),
    '#default_value' => variable_get('classification_import_file_limit', 15000),
    '#element_validate' => array('classification_import_textfield_validate'),
    '#required' => true,
  );

  $form['import_limits']['classification_import_tcs_limit'] = array(
    '#type' => 'textfield',
    '#title' => t('Name limit for EOL import web service'),
    '#default_value' => variable_get('classification_import_tcs_limit', 15000),
    '#element_validate' => array('classification_import_textfield_validate'),
    '#required' => true,
  );

  $form['file_administration'] = array(
    '#type' => 'fieldset',
    '#title' => t('File upload administration'),
    '#description' => t('Adjust settings for the administration of uploaded files.'),
    '#collapsible' => true,
    '#collapsed' => true,
  );

  $form['file_administration']['classification_import_retain_copy'] = array(
    '#type' => 'radios',
    '#title' => t('Retain a copy of each uploaded file'),
	'#default_value' => variable_get('classification_import_retain_copy', 1),
	'#options' => array(1 => t('Yes'), 0 => t('No')),
  );

  $form['file_administration']['classification_import_extensions'] = array(
    '#type' => 'checkboxes',
    '#title' => t('File extensions'),
    '#description' => t('Set allowed file extensions for upload'),
    '#default_value' => variable_get('classification_import_extensions', array('txt', 'xls', 'xlsx')),
    '#options' => array(
      'txt' => t('Tab-delimited text (.txt)'),
      'xls' => t('Excel 97-2004 (.xls)'),
      'xlsx' => t('Excel 2007 (.xlsx)'),
    ),
    '#required' => true,
  );

  $form['file_administration']['classification_import_filesize'] = array(
    '#type' => 'textfield',
    '#title' => t('Minimum file size (bytes)'),
    '#default_value' => variable_get('classification_import_filesize', 2),
    '#element_validate' => array('classification_import_textfield_validate'),
    '#required' => true,
  );

  $form['file_administration']['classification_import_show_memory_consumption'] = array(
    '#type' => 'radios',
    '#title' => t('Show a warning message for memory consumption after parsing uploaded files'),
    '#default_value' => variable_get('classification_import_show_memory_consumption', 0),
    '#options' => array(1 => t('Yes'), 0 => t('No')),
  );
  
  return system_settings_form($form);
}

function classification_import_textfield_validate($element, &$form_state) {
   if (!(int)$element['#value'] || $element['#value'] < 0) {
     form_error($element, t('This field must be an integer greater than 0.'));
   }
}