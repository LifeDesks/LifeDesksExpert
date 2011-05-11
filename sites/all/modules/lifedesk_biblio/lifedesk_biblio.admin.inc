<?php

function lifedesk_biblio_admin_form() {
  $form['refparser'] = array(
    '#type' => 'fieldset',
    '#title' => t('Reference Parser'),
    '#description' => t('Configure the dynamic reference parsing tool on LifeDesk taxon pages'),
  );

  $form['refparser']['lifedesk_biblio_pid'] = array(
    '#type' => 'textfield',
    '#title' => t('CrossRef Personal Identifier'),
    '#description' => t('A CrossRef personal identifier (pid) may be obtained at !link', array('!link' => l('http://www.crossref.org/requestaccount/', 'http://www.crossref.org/requestaccount/', array('absolute' => true, 'attributes' => array('target' => '_blank'))))),
    '#default_value' => variable_get('lifedesk_biblio_pid', ''),
  );

  $form['refparser']['lifedesk_biblio_parserurl'] = array(
    '#type' => 'textfield',
    '#title' => t('Reference Parser Web Service'),
    '#default_value' => variable_get('lifedesk_biblio_parserurl', 'http://refman.eol.org/cgi-bin/refparser.cgi?')
  );

  return system_settings_form($form);
}