<?php

/*
 * Helper function to validate file upload
 */
function _taxon_description_upload_validators() {
  $validators = array(
    '_taxon_description_validator_extensions' => array(array_filter(variable_get('taxon_description_upload_extensions',array('txt', 'xls', 'xlsx')))),
    '_taxon_description_validator_filesize' => array( 2 ),
  );

  return $validators; 
}

function _taxon_description_validator_extensions($file, $extensions) {
  $errors = array();

  $ext = substr($file->filename, strrpos($file->filename, '.') + 1);

  // Bypass validation for uid  = 1.
  if ($user->uid != 1) {
    if (!in_array($ext, $extensions)) {
      $errors[] = t('Sorry, accepted file extensions are %extensions.', array('%extensions' => implode(', ', $extensions)));
    }
  }

  return $errors;
}

function _taxon_description_validator_filesize($file, $size) {
  global $user;

  $errors = array();
  // Bypass validation for uid  = 1.
  if ($user->uid != 1) {
    if ($file->filesize < $size) {
      $errors[] = t('Sorry, your file did not have any content.');
    }
  }

  return $errors;
}

function _taxon_description_validators() {
	$langs = array();
	foreach(language_list() as $iso => $language) {
	  $langs[$iso] = $iso;
	}
	$validators = array(
	  'licenses' => array_filter(creativecommons_lite_get_license_array()),
	  'states' => array(
		'Draft' => 'Draft',
		'Published' => 'Published'),
	  'languages' => $langs,
	);
	
	return $validators;
}

function taxon_description_import_form(&$form_state) {
	
  global $NAMES;

  $module = drupal_get_path('module', 'taxon_description');
  drupal_add_css($module . '/css/taxon_description_import.css');

  $form = array();

  //check for at least one name in the classification before we present the form
  $term = db_fetch_object(db_query("SELECT tid FROM term_data WHERE vid = %d LIMIT 1", CLASSIFICATION_PRIMARY));
  if(!$term->tid) {
	    $message = t('Sorry, before you import a template, you must !add at least one name to your classification.', array('!add' => l(t('add'),'admin/classification/biological/edit')));
	    drupal_set_message($message,'error');
	    return $form;
  }

  $form['#attributes'] = array('enctype' => "multipart/form-data");

  $form['taxon_description_import_preview'] = array(
    '#type' => 'markup',
    '#value' => $form_state['values']['taxon_description_import_preview'],
  );
  
  $form['step1'] = array(
	'#type' => 'markup',
	'#prefix' => '<div class="messages warning">',
	'#suffix' => '</div>',
	'#value' => '<p>' . t('!warning Specifying multiple contributing authors per chapter is not accommodated in these templates. If you re-upload a template, previously specified authors for each taxon-chapter combination will be overwritten.</p><p><em>There is a limit of 50 taxa per template.</em></p>', array('!warning' => '<strong>' . t('WARNING') . '</strong>')) . '<p>' . t('Allowed HTML tags: ') . check_plain(variable_get('allowed_html_1','')) . '</p>',
  );  
  
  $form['preview'] = array(
    '#type' => 'fieldset',
    '#title' => '<b>'.t('Step 1. Check your file').'</b>',
  ); 
    
  $form['preview']['taxon_description_check'] = array(
    '#type' => 'file',
    '#title' => t('File to check'),
    '#description' => t('Result(s) will appear in a colored box(es) at the top of the page.'),
  );

  $form['preview']['preview'] = array(
    '#type' => 'submit',
    '#value' => t('Check'),
  );

  $form['import'] = array(
    '#type' => 'fieldset',
    '#title' => '<b>'.t('Step 2. Import your file').'</b>',
  );
  
  $form['import']['taxon_description_upload'] = array(
    '#type' => 'file',
    '#title' => t('File to import'),
  );

    $form['import']['import'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
  );

return $form;
}

function taxon_description_import_form_submit($form, &$form_state) {
  // Bump up the limits
  ini_set('max_execution_time',360);
  
switch ($form_state['values']['op']) {
    case t('Check'):
      taxon_description_check($form, $form_state);
    break;
    case t('Import'):
      taxon_description_import($form, $form_state);
    break;
    default:
    break;
  }
}

function taxon_description_check($form, &$form_state) {
	
	$validators = _taxon_description_upload_validators();
	
	if ($file = file_save_upload('taxon_description_check', $validators)) {
	  $objPHPExcel = excel_reader_parse($file);
	  _taxon_description_check_parse($objPHPExcel);
	}
}

function _taxon_description_check_parse($objPHPExcel) {

  global $NAMES, $theme, $user;

  $creativecommons_lite_path = '/' . drupal_get_path('module', 'creativecommons_lite') .'/images/buttons_small/';

  $validators = _taxon_description_validators();

  $iscontent = false;

  //do a quick check on the number of rows
  $rowcount = $objPHPExcel->getActiveSheet()->getHighestRow();
  if($rowcount > 55) {
	drupal_set_message(t('Sorry, there is a limit of 50 taxa in your import file'),'warning');
    return;	
  }

  $output = '<h2>' . t('Preview') . '</h2>';

  $content = _taxon_description_build_content($objPHPExcel);

  $language_list = language_list();

  $bad_chapters = array();
  foreach($content['alluniquechapters'] as $singlechapter) {
    $name = db_result(db_query("SELECT name FROM {term_data} WHERE vid = %d AND name = '%s' LIMIT 1", TAXON_DESCRIPTION_CHAPTERS, $singlechapter));
	if ($name == "") {
	  $bad_chapters[] = $singlechapter;
	}
  }

  if($bad_chapters) {
    drupal_set_message(t('!warning The chapter(s) %chapters were not found in your database. Content will not be imported for these chapters.', array('!warning' => '<strong>' . t('WARNING!') . '</strong>', '%chapters' => implode(', ',$bad_chapters))), 'warning');	
  }

  unset($content['alluniquechapters']);

  // Now we can loop through the array and unset the bad chapters and get rid of the empty chapters
  foreach($content as $data) {
    foreach($data['chapters'] as $key => $chapter) {
	  if(!$data['chapters'][$key]) {
	    unset($data['chapters'][$key]);	
	  }
	  if(in_array($key,$bad_chapters)) {
	    unset($data['chapters'][$key]);	
	  }
    }

      //check to see if name/id pair was messed with, but only those with content
	  if($data['chapters']) {
	    $name = db_result(db_query("SELECT name FROM {term_data} WHERE vid = %d AND name = '%s' AND tid ='%s' LIMIT 1", CLASSIFICATION_PRIMARY, $data['taxon'], $data['identifier']));
		if ($name == "") {
		  drupal_set_message(t('!warning The Taxa -- Identifier pair <b>!taxon_name -- !taxon_identifier</b> is not present in the database. This row will not be imported.', array('!warning' => '<strong>' . t('WARNING!') . '</strong>', '!taxon_name' => strip_tags($data['taxon']), '!taxon_identifier' => strip_tags($data['identifier']))), 'warning');
		 unset($data);
		}
		
		//check to see if licensing, state, and language are recognized and accepted
		if($data && !array_key_exists($data['license'],$validators['licenses'])) {
			drupal_set_message(t('!warning The Creative Commons license %license is not accepted by LifeDesks. This row will not be imported.', array('!warning' => '<strong>' . t('WARNING!') . '</strong>', '%license' => $data['license'])), 'warning');
			unset($data);
		}
		if($data && !array_key_exists($data['status'],$validators['states'])) {
			drupal_set_message(t('!warning The published state %state is not accepted by LifeDesks. This row will not be imported.', array('!warning' => '<strong>' . t('WARNING!') . '</strong>', '%state' => $data['status'])), 'warning');
			unset($data);
		}
		if($data && !array_key_exists($data['language'],$validators['languages'])) {
			drupal_set_message(t('!warning The language %language is not accepted by LifeDesks. This row will not be imported.', array('!warning' => '<strong>' . t('WARNING!') . '</strong>', '%language' => $data['language'])), 'warning');
			unset($data);
		}
		
	  }
	  else {
	    unset($data);	
	  }

	  if($data) {
		$iscontent = true;
	    $output .= "\n\n" . "<div class='taxon-description-content'>" . "\n";
		$output .= '<h3>' . (classification_check_italicize($data['identifier']) ? $NAMES->italicized_form($data['taxon']) : $data['taxon']) . '</h3>' . "\n";
		foreach($data['chapters'] as $key => $value) {
	      $output .= '<h4 class="taxon-description-chapter-heading">' . $key . '</h4>' . "\n";
		  $output .= '<div class="taxon-description-chapter-content">' . $value . "\n";
		  $output .= '<div class="taxon-description-chapter-authors">' . "\n";
		  $output .= t('<strong>Contributing author:</strong> !author', array('!author' => $user->surname . ', ' . $user->givenname));
		  $output .= '</div>';
		  $output .= '<div class="taxon-description-chapter-extra">' . "\n";
		  $output .= '<span class="taxon-description-chapter-status ' . $data['status'] . '">' . $data['status'] . '</span>' . "\n";
		  $output .= '<span class="taxon-description-chapter-license"><img src="' . $creativecommons_lite_path . $data['license'] . '.png" alt="' . t('Creative Commons !license', array('!license' => $data['license'])) . '"></span>' . "\n"; 
		  $output .= '<span class="taxon-description-chapter-language">'.$language_list[$data['language']]->native . '</span>' . "\n";
		  $output .= '</div>' . "\n";
		  $output .= '</div>' . "\n";
		}
		$output .= '</div>' . "\n";
	  }
	}

  if($iscontent) {
    drupal_set_message($output,'preview');
  }
  else {
    drupal_set_message(t('Sorry, either there was no content in your file or something bad happened. Please try again.'),'error');	
  }

}

function taxon_description_import($form, &$form_state) {
	
	$validators = _taxon_description_upload_validators();
	
	if ($file = file_save_upload('taxon_description_upload', $validators)) {
	  $parsed = excel_reader_parse($file);
	  _taxon_description_import_parse($parsed);				
	}	
}

function _taxon_description_import_parse($objPHPExcel) {
  global $user;

  //get the cntr_id & name from taxon_description_contributor_data table
  $taxon_description_author = array();
  $qry = db_query("SELECT cntr_id, name FROM {taxon_description_contributor_data} WHERE drupal_uid=%d", $user->uid);
  while($result = db_fetch_array($qry)) {
  	$taxon_description_author = $result;
  }

  $timestamp = time();

  $validators = _taxon_description_validators();

  $content_exists = false;

  $content = _taxon_description_build_content($objPHPExcel);

  //do a quick check on the number of rows and limit for performance reasons
  $rowcount = $objPHPExcel->getActiveSheet()->getHighestRow();
  if($rowcount > 55) {
	drupal_set_message(t('Sorry, there is a limit of 50 taxa in your import file'),'warning');
    return;	
  }

  $good_chapters = array();
  $bad_chapters = array();

  foreach($content['alluniquechapters'] as $singlechapter) {
    $qry = db_query("SELECT td.tid, td.name FROM {term_data} td INNER JOIN {term_hierarchy} th ON (td.tid=th.tid) WHERE td.vid = %d AND td.name = '%s' AND th.parent <> 0", TAXON_DESCRIPTION_CHAPTERS, $singlechapter);
    while($result = db_fetch_object($qry)) {
	  $good_chapters[$result->name] = $result->tid;
    }
  }

  foreach($content['alluniquechapters'] as $singlechapter) {
    if(!$good_chapters[$singlechapter]) {
	  $bad_chapters[] = $singlechapter;
    }	
  }

  if($bad_chapters) {
    drupal_set_message(t('!warning The chapter(s) %chapters were not found in your database. Content will not be imported for these chapters.', array('!warning' => '<strong>' . t('WARNING') . '</strong>', '%chapters' => implode(', ',$bad_chapters))), 'warning');	
  }

  unset($content['alluniquechapters']);
	
  foreach($content as $data) {
	
	$node = '';
	
	foreach($data['chapters'] as $key => $value) {
	  if (in_array($key,$bad_chapters)) {
	    unset($data['chapters'][$key]);	
	  }
    }

    $tid_taxa = '';
	if($data['chapters']) {
		
	  // check to see if taxon -- id pair exists
	  $tid_taxa = db_result(db_query("SELECT tid FROM {term_data} WHERE vid = %d AND name = '%s' AND tid ='%s' LIMIT 1", CLASSIFICATION_PRIMARY, $data['taxon'], $data['identifier']));
	  if ($tid_taxa == "") {
        drupal_set_message(t('!warning The Taxa -- Identifier pair <b>!taxon_name -- !taxon_identifier</b> was not present in the database. This row was not imported.', array('!warning' => '<strong>' . t('WARNING') . '</strong>', '!taxon_name' => strip_tags($data['taxon']), '!taxon_identifier' => strip_tags($data['identifier']))), 'warning');
        unset($data);
	  }

	  //check to see if licensing and state are recognized and accepted
	  if($data && !array_key_exists($data['license'],$validators['licenses'])) {
	    drupal_set_message(t('!warning The Creative Commons license %license is not accepted by LifeDesks. This row will not be imported.', array('!warning' => '<strong>' . t('WARNING') . '</strong>', '%license' => $data['license'])), 'warning');
		unset($data);
	  }
	  if($data && !array_key_exists($data['status'],$validators['states'])) {
		drupal_set_message(t('!warning The published state %state is not accepted by LifeDesks. This row will not be imported.', array('!warning' => '<strong>' . t('WARNING') . '</strong>', '%state' => $data['status'])), 'warning');
		unset($data);
	  }
	  if($data && !array_key_exists($data['language'],$validators['languages'])) {
		drupal_set_message(t('!warning The language %language is not accepted by LifeDesks. This row will not be imported.', array('!warning' => '<strong>' . t('WARNING') . '</strong>', '%language' => $data['language'])), 'warning');
		unset($data);
	  }
	
	  if($data) {
		//let's first see if a node already exists for specified language
		$exists = db_result(db_query("SELECT n.nid FROM {node} n INNER JOIN {taxon_description} td ON (n.nid=td.nid) WHERE td.tid_taxon=%d AND n.language='%s' AND n.uid=%d",$tid_taxa,$data['language'],$user->uid));
		$node->nid = ($exists) ? $exists : '';
        $node->title = $data['taxon'];
        $node->cc_lite_license = $data['license'];
        $node->status = (strip_tags($data["status"]) == "Published") ? 1 : 0;
        $node->type = 'taxon_description';
        $node->language = $data['language'];
        $node->taxonomy = array(
	      CLASSIFICATION_PRIMARY => array(
		    $tid_taxa => $tid_taxa
		  ));
		$node->uid = $user->uid;

        //now build the chapter data
        $taxon_description = array();
        foreach($data['chapters'] as $key => $value) {
	      $tid_chapter = $good_chapters[$key];
		  if ($tid_chapter) {
			$taxon_description[$tid_chapter] = array(
			  'contributors' => array(
					array(
						'name' => $taxon_description_author['name'],
						'cntr_id' => $taxon_description_author['cntr_id'],
						'rank' => 0,
					),
			  ),
			  'cid' => $tid_chapter,
			  'content' => $value,
		    );
		  }	
        }

        $node->taxon_description = $taxon_description;

        if($taxon_description) {
	      node_save($node);
	      $content_exists = true;
        }	
	  }
	}
  }

  if($content_exists) {
	drupal_set_message(t('Your file was successfully imported.'));
  }
  else {
    drupal_set_message(t('Your file was successfully parsed, but there was no content inserted in the database'), 'error');	
  }

}

/*
 * Helper function to build the $content array from a $objPHPExcel object
 */
function _taxon_description_build_content($objPHPExcel) {

    $content = array();
	
	foreach ($objPHPExcel->getWorksheetIterator() as $worksheet) {
	
	  foreach ($worksheet->getRowIterator() as $row) {
		
		$rowindex = $row->getRowIndex();
		
		if($rowindex == 2) {
		  foreach ($row->getCellIterator() as $cell) {
			if($cell->columnIndexFromString($cell->getColumn()) > 5) {
				$content['alluniquechapters'][$cell->getCalculatedValue()] = $cell->getCalculatedValue();
			}	
		  }
		}

		$cellIterator = $row->getCellIterator();
		$cellIterator->setIterateOnlyExistingCells(false); // Loop all cells, even if it is not set

		foreach ($cellIterator as $cell) {
		  $column = $cell->getColumn();
		  $identifier = $worksheet->getCell('B'.$rowindex)->getCalculatedValue();
		  $language = $worksheet->getCell('E'.$rowindex)->getCalculatedValue();
		  if (!is_null($cell) && $rowindex > 2) {
		    switch($column) {
			  case 'A':
			    $content[$identifier.$language]['taxon'] = $cell->getCalculatedValue();
			    break;
			  case 'B':
			    $content[$identifier.$language]['identifier'] = $cell->getCalculatedValue();
				break;
			  case 'C':
				$content[$identifier.$language]['status'] = $cell->getCalculatedValue();
				break;
			  case 'D':
			    $content[$identifier.$language]['license'] = $cell->getCalculatedValue();
				break;
			  case 'E':
			    $content[$identifier.$language]['language'] = $cell->getCalculatedValue();
				break;
			  default:
                  $content[$identifier.$language]['chapters'][$worksheet->getCell($column.'2')->getCalculatedValue()] = ($cell->getCalculatedValue()) ? trim(check_markup($cell->getCalculatedValue(),FILTER_FORMAT_DEFAULT)) : '';
			 }		
		  }
	    }	
	 }
	}

	return $content;
}

