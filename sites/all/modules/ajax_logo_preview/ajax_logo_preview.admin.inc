<?php

/**
* Implementation of hook_settings().
*/
function ajax_logo_preview_admin_settings() {
  $form['ajax_logo_dimensions'] = array(
    '#type' => 'textfield',
    '#title' => 'AJAX logo dimensions',
    '#description' => t('Set the default dimensions of the site\'s logo.'),
    '#default_value' => variable_get('ajax_logo_dimensions', '300x89'),
  );
  $form['ajax_logo_dimensions_nositename'] = array(
    '#type' => 'textfield',
    '#title' => 'AJAX logo dimensions without a site name (full banner)',
    '#description' => t('Set the default dimensions of the site\'s logo.'),
    '#default_value' => variable_get('ajax_logo_dimensions_nositename', '982x89'),
  );
  $form['ajax_logo_dimensions_preview'] = array(
    '#type' => 'textfield',
    '#title' => 'AJAX logo dimensions for preview',
    '#description' => t('Set the default dimensions of the site\'s logo.'),
    '#default_value' => variable_get('ajax_logo_dimensions_preview', '600x89'),
  );
  $form['ajax_logo_path'] = array(
    '#type' => 'textfield',
    '#title' => 'AJAX logo path',
    '#description' => t('Set the path for temporary ajax logos.'),
    '#default_value' => variable_get('ajax_logo_path', 'ajax_logos'),
  );
  
  return system_settings_form($form);
}