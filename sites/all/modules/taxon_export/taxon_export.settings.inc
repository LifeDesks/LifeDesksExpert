<?php

/**
 * Taxon Export settings form
 */
function taxon_export_settings_form(&$form_state) {

  $module_path = drupal_get_path('module', 'taxon_export') .'/';
  drupal_add_css($module_path .'css/taxon_export.css');

  $form = array(
    '#cache' => FALSE,
    '#tree' => TRUE,
  );

  $options[] = '-- Choose --';
  $result = db_query("SELECT * FROM {taxon_export_spm}");
  while ($eol_spm = db_fetch_object($result)) {
    $options[$eol_spm->sid] = $eol_spm->infoitem;
  }

  $chapters = taxon_description_taxonomy_get_tree(TAXON_DESCRIPTION_CHAPTERS, 'en');
  foreach($chapters as $chapter) {
    $tid = $chapter->tid;
    $name = $chapter->name;
    $parent = $chapter->parents[0];
    // remove all the grouping chapter headings
    if(!$parent) {
    }
    else {
      $default_result = db_fetch_object(db_query("SELECT sid FROM {taxon_export_map} WHERE tid = %d",$tid));
      if($default_result) {
        $default = $default_result->sid;
      }
      else {
        $default = array_search($name,$options);
      }

      $form['chapter_mappings'][$tid]['chapter'] = array(
        '#type' => 'markup',
        '#value' => '<strong>' . $name . '</strong>',
      );

      $form['chapter_mappings'][$tid]['eol_chapter'] = array(
        '#type' => 'select',
        '#options' => $options,
        '#default_value' => $default,
        '#required' => TRUE,
      );
    }
  }

  $form['submit'] = array(
  	'#type' => 'submit',
  	'#value' => t('Save'),
    '#prefix' => '<div class="centered_button">',
    '#suffix' => '</div>',
  );
    
  return $form;
}

function theme_taxon_export_settings_form($form) {
  $header = array(t('Your Sub Chapters'), t('Species Profile Model Chapters'));
  foreach (element_children($form['chapter_mappings']) as $key) {
    $row = array();
    $row[] = drupal_render($form['chapter_mappings'][$key]['chapter']);
    $row[] = drupal_render($form['chapter_mappings'][$key]['eol_chapter']);
    $rows[] = $row;
  }
  $output = '<div id="taxon-export-map">' . t('Map your taxon page chapters to an equivalent in the Species Profile Model, an emerging standard used by the Encyclopedia of Life. If you have translated chapters, these will be automatically mapped.') . '</div>';
  $output .= theme('table', $header, $rows, array('style' => 'width:400px;margin:0px auto;'));
  $output .= drupal_render($form);

  return $output;
}

/**
 * Taxon export settings form submit
 */
function taxon_export_settings_form_submit($form, &$form_state) {
  $enabled = locale_language_list();
  unset($enabled['en']);

  db_query("DELETE FROM {taxon_export_map}");
  foreach($form_state['values']['chapter_mappings'] as $tid => $eol_chapter) {
    db_query("INSERT INTO {taxon_export_map} (sid,tid) VALUES (%d,%d)",$eol_chapter['eol_chapter'],$tid);

    foreach($enabled as $code => $native) {
	  $translated_tid = db_result(db_query("SELECT tid FROM {term_data} WHERE trid = %d AND language = '%s'", $tid, $code));
	  db_query("INSERT INTO {taxon_export_map} (sid,tid) VALUES (%d,%d)", $eol_chapter['eol_chapter'], $translated_tid);
    }
  }
  drupal_set_message(t('Settings for configuring your export were saved.<br>If you have not already done so, you may create your EOL Content Partner endpoint !here', array('!here' => l(t('here'), 'admin/export/taxon_export/export'))),'status');
}