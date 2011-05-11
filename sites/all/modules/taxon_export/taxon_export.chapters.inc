<?php

/**
 * Taxon Export chapter definitions page
 */
function taxon_export_chapters() {
  $module_path = drupal_get_path('module', 'taxon_export') .'/';
  drupal_add_css($module_path .'css/taxon_export.css');

  $output = '<div class="messages status">' . t('The following chapters and definitions are developed by the Biodiversity Information Standards (TDWG) and are collectively called the Species Profile Model (SPM). Content partners to the Encyclopedia of Life like you and your fellow LifeDesk owners, map their chapters to SPM chapters to facilitate machine-machine dialog.') . '</div>';

  $result = db_query("SELECT * FROM {taxon_export_spm}");
  while ($spm = db_fetch_object($result)) {
      $chapter = $spm->infoitem;
      $definition = $spm->definition;

      $output .= '<h3>' . $chapter . '</h3>';
      $output .= '<p class="chapter_definition">' . $definition . '</p>';
  }

  return $output;
}