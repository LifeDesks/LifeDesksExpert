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

  $form['juggernaut_host'] = array(
    '#type' => 'textfield',
    '#title' => t('Juggernaut host'),
    '#default_value' => variable_get('juggernaut_host', 'www.lifedesks.org')
  );

  $form['juggernaut_port'] = array(
    '#type' => 'textfield',
    '#title' => t('Juggernaut port'),
    '#default_value' => variable_get('juggernaut_port', 8080)
  );
 
  $form['juggernaut_channel'] = array(
    '#type' => 'textfield',
    '#title' => t('Juggernaut channel'),
    '#default_value' => variable_get('juggernaut_channel', 'site.lifedesks.org')
  );

  $form['redis_host'] = array(
    '#type' => 'textfield',
    '#title' => t('Redis host'),
    '#default_value' => variable_get('redis_host', '127.0.0.1')
  );

  $form['redis_port'] = array(
    '#type' => 'textfield',
    '#title' => t('Redis port'),
    '#default_value' => variable_get('redis_port', 6379)
  );

  return system_settings_form($form);
}