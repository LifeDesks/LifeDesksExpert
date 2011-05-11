<?php

/**
* Implementation of hook_settings().
*/
function classification_admin_settings() {

  $form['classification_status'] = array(
    '#type' => 'radios',
    '#title' => t('Classification status'),
    '#description' => t('Select state for global classification module status. "Busy" means names cannot be imported or edited.'),
    '#default_value' => variable_get('classification_status', 1),
    '#options' => array(1 => t('Ready'), 0 => t('Busy')),
  );

  $form['classification_pinger_freq'] = array(
    '#type' => 'select',
    '#title' => t('Classification pinger frequency'),
    '#description' => t('Adjust the frequency in seconds the classification polling mechanism executes.'),
    '#default_value' => variable_get('classification_pinger_freq', 5),
    '#options' => array(5 => 5, 6 => 6, 7 => 7),
  );

  return system_settings_form($form);
}