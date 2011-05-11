<?php

/**
 * @file
 *   Administrative pages for Taxon Search.
 */

function taxon_search_settings() {
  $form = array();

  // Perform a check to ensure the server is there
  if (empty($_POST)) {
    // update_help() loads install.inc via include_once, so not much
    // point in working around it.
    include_once './includes/install.inc';
    module_load_include('install', 'apachesolr');

    foreach (apachesolr_requirements('runtime') as $requirement) {
      $status = $requirement['severity'] == REQUIREMENT_ERROR ? 'error' : 'status';
      drupal_set_message($requirement['title'] . ': ' . $requirement['value'], $status);
    }
  }

  $form['taxon_search_solr_host'] = array(
    '#type' => 'textfield',
    '#title' => t('Solr host name'),
    '#default_value' => variable_get('taxon_search_solr_host', 'localhost'),
    '#description' => t('Host name of your Solr server, e.g. <code>localhost</code> or <code>example.com</code>.'),
    '#required' => TRUE,
  );
  $form['taxon_search_solr_port'] = array(
    '#type' => 'textfield',
    '#title' => t('Solr port'),
    '#default_value' => variable_get('taxon_search_solr_port', '8983'),
    '#description' => t('Port on which the Solr server listens. The Jetty example server is 8983, while Tomcat is 8080 by default.'),
    '#required' => TRUE,
  );
  $form['taxon_search_solr_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Solr path'),
    '#default_value' => variable_get('taxon_search_solr_path', '/solr'),
    '#description' => t('Path that identifies the Solr request handler to be used.'),
  );

  $options = drupal_map_assoc(array(5, 10, 15, 20, 25, 30, 40, 50, 60, 70, 80, 90, 100));
  $form['taxon_search_solr_rows'] = array(
    '#type' => 'select',
    '#title' => t('Results per page'),
    '#default_value' => variable_get('taxon_search_solr_rows', 10),
    '#options' => $options,
    '#description' => t('The number of results that will be shown per page.'),
  );

  return system_settings_form($form);
}

/**
 * Validation function for the taxon_search_settings form.
 */
function taxon_search_settings_validate($form, &$form_state) {
  if (isset($form['apachesolr_port'])) {
    $port = $form_state['values']['taxon_search_solr_port'];
    // TODO: Port range should be 0-65535, but 0 crashes apachesolr
    if (!ctype_digit($port) || $port < 1 || $port > 65535) {
      form_set_error('taxon_search_solr_port', t('The port has to be an integer between 1 and 65535.'));
    }
  }
}