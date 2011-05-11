<?php

function taxon_description_export_form(&$form_state) {

  $classification_settings = variable_get('classification_settings',array());

  $form = array();

  $term = db_fetch_object(db_query("SELECT COUNT(*) AS sum FROM term_data WHERE vid = %d", CLASSIFICATION_PRIMARY));
  if($term->sum < 1) {
	    $message = t('Sorry, before you export a template, you must !add at least one name to your classification.', array('!add' => l(t('add'),'admin/classification/biological/edit')));
	    drupal_set_message($message,'error');
	    return $form;
  }
  else {
	  drupal_add_css(drupal_get_path('module', 'taxon_description') .'/css/taxon_description_export.css');
	  drupal_add_js(drupal_get_path('module', 'taxon_description') .'/js/taxon_description_export.js');
	  drupal_add_js(array('classification_tags' => array('vid' => CLASSIFICATION_PRIMARY)), 'setting');	
  }
	
  $form['template'] = array(
	'#prefix' => '<div class="node-form">',
	'#suffix' => '</div>',
	'#value' => '',
  );

  $form['classification_tree'] = array(
	'#type' => 'classification_tree'
  );

  $form['template']['warning'] = array(
	'#type' => 'markup',
	'#prefix' => '<div class="messages warning">',
	'#suffix' => '</div>',
	'#value' => '<p>' . t('!warning Specifying multiple contributing authors per chapter is not accommodated in these templates. If you re-upload a template, previously specified authors for each taxon-chapter combination will be overwritten. <em>There is a limit of 50 taxa per template.</em></p>', array('!warning' => '<strong>' . t('WARNING') . '</strong>')) . '<p>' . t('Allowed HTML tags: ') . check_plain(variable_get('allowed_html_1','')) . '</p>',
  );

  $form['template']['file_export'] = array(
    '#type' => 'fieldset',
    '#title' => t('File Format'),
  );
  
  $opt[''] = t('--Choose--');
  $opt[1] = t('Microsoft Excel 2007');
  $opt[2] = t('Microsoft Excel 97-2004');
  $opt[3] = t('Tab-delimited Text');
 
  $form['template']['file_export']['export_format'] = array(
    '#type' => 'select',
    '#title' => t('File format'),
    '#options' => $opt,
    '#required' => TRUE,
  );

  $form['template']['taxonomy_classification_multitag_' . CLASSIFICATION_PRIMARY] = array(
    '#type' => 'hidden'
  );

  $langs = locale_language_list();

  if(count($langs) > 1) {
	$form['template']['language'] = array(
	  	'#type' => 'radios',
	    '#title' => t('Language'),
	    '#options' => $langs,
	    '#required' => TRUE,
	  );
  }
    
  $form['template']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Export'),
  );

  return $form;
}

function taxon_description_export_form_validate($form, &$form_state) {
    if(empty($form_state['values']['taxonomy_classification_multitag_1'])) {
	    form_set_error('classification_tree',t('At least one taxon name must be selected.'));
	    return;
    }
}

function taxon_description_export_form_submit($form, &$form_state) {

  $file_format = $form_state['values']['export_format'];

  $shortname = variable_get('shortname','lifedesks');

  switch($file_format) {

    // Excel 2007 file
	case '1':
      $objPHPExcel = _taxon_description_build_output($form_state);

	  // Redirect output to a client’s web browser
	  $filename = $shortname . '_chapter_template_'.date("Ymd_Gi").'.xlsx';

	  header('Content-Type: application/vnd.openxmlformats-officedocument.spreadsheetml.sheet');
	  header('Content-Disposition: attachment;filename="' . $filename . '"');
	  header('Cache-Control: max-age=0');

	  $objWriter = PHPExcel_IOFactory::createWriter($objPHPExcel, 'Excel2007');
	  $objWriter->setOffice2003Compatibility(true);
	  $objWriter->save('php://output');
	break;
	
	// Excel 97-2004 file
	case '2':
	  $objPHPExcel = _taxon_description_build_output($form_state);

      // Redirect output to a client’s web browser
	  $filename = $shortname . '_chapter_template_'.date("Ymd_Gi").'.xls';

	  header('Content-Type: application/vnd.ms-excel');
	  header('Content-Disposition: attachment; filename='.$filename);
	  header('Cache-Control: max-age=0');

	  $objWriter = PHPExcel_IOFactory::createWriter($objPHPExcel, 'Excel5');
	  $objWriter->save('php://output');
	break;
	
	// tab-delimited text file
	case '3':
	  $objPHPExcel = _taxon_description_build_output($form_state);

      // Redirect output to a client’s web browser
	  $filename = $shortname . '_chapter_template_'.date("Ymd_Gi").'.txt';

	  header('Content-Type: text/tab-separated-values');
	  header('Content-Disposition: attachment; filename='.$filename);
	  header('Cache-Control: max-age=0');

	  $objWriter = PHPExcel_IOFactory::createWriter($objPHPExcel, 'CSV');
	  $objWriter->setDelimiter("\t")->setEnclosure('');
	  $objWriter->save('php://output');
	break;
  }

}

/**
 * Helper function to build a properly formatted output for any file format the PHPExcel class has a writer for
 */
function _taxon_description_build_output($form_state) {

  global $user;

  $taxa = explode(",",$form_state['values']['taxonomy_classification_multitag_1']);

  $shortname = variable_get('shortname','LifeDesk');

  foreach($taxa as $tid) {
    $taxon = taxonomy_get_term($tid);
    $taxon_name[$tid] = array(
	  'name' => $taxon->name,
	  'identifier' => $tid 
	);
  }
 
  $lang = ($form_state['values']['language']) ? $form_state['values']['language'] : 'en';

  $chapter_tree = taxon_description_taxonomy_get_tree(TAXON_DESCRIPTION_CHAPTERS, $lang);

	  error_reporting(E_ALL);
	
	  module_load_include('php','excel_reader','includes/PHPExcel');
	  module_load_include('php','excel_reader','includes/PHPExcel/IOFactory');
	
	  // Create new PHPExcel object
	  $objPHPExcel = new PHPExcel();
	
	  $site_name = variable_get('site_name','LifeDesk');
	
	  // Set properties
	  $objPHPExcel->getProperties()->setCreator($user->firstname . " " . $user->lastname)
								 ->setLastModifiedBy($user->firstname . " " . $user->lastname)
								 ->setTitle($site_name . " " . t('Taxon Description Template'))
								 ->setSubject(t('LifeDesk Excel template'))
								 ->setDescription(t('An Excel template to help bulk upload taxon descriptions'))
								 ->setKeywords("LifeDesk")
								 ->setCategory("");

	  // Add some data
	  $data = $objPHPExcel->setActiveSheetIndex(0);
	  
	  $data->setCellValue('A2', 'Taxon')
	            ->setCellValue('B2', 'Identifier')
	            ->setCellValue('C2', 'Status')
	            ->setCellValue('D2', 'License')
	            ->setCellValue('E2', 'Language');
	
	  // Set the headers (note: cols are 0-based whereas rows are 1-based)
	  $next_col = 5;
	  $merged = array();
	  foreach ($chapter_tree as $i){
		$prev_col = PHPExcel_Cell::stringFromColumnIndex($next_col);
	    $tid = $i->tid;
	    $name = $i->name;
	    $parent = $i->parents[0];
	    if($parent == 0) {
		  $children = count(taxonomy_get_children($tid, TAXON_DESCRIPTION_CHAPTERS, $key = 'tid'))-1;
		  if($children >= 0) {
			$data->setCellValueByColumnAndRow($next_col,1,$name);
			$next_col = $next_col + $children;
			$last_col = PHPExcel_Cell::stringFromColumnIndex($next_col);
			$next_col++;
			if($children > 1) $merged[] = $prev_col . '1:' . $last_col. '1';
		  }
		}
	  }
	  
	  // Merge the headers
	  foreach($merged as $merge) {
	    $objPHPExcel->getActiveSheet()->mergeCells($merge);	
	  }

    // Set the chapters
    $next_col = 5;
    $chapter_cols = array();
    foreach ($chapter_tree as $i) {
	    $tid = $i->tid;
	    $name = $i->name;
	    $parent = $i->parents[0];
	    if($parent != 0) {
		  $chapter_cols[$tid] = PHPExcel_Cell::stringFromColumnIndex($next_col);
		  $data->setCellValueByColumnAndRow($next_col,2,$name);
		  $next_col++;
		}
    }

    // Set the taxa
    $next_row = 3;

    foreach($taxon_name as $taxon) {
	    //fetch existing content to pre-populate Excel sheet
	    $qry = db_query("SELECT td.nid FROM {taxon_description} td INNER JOIN {node} n ON (td.nid=n.nid) WHERE n.uid=%d AND td.tid_taxon=%d AND n.language = '%s' LIMIT 1", $user->uid, $taxon['identifier'], $lang);
	    $existing_content = db_result($qry);
	    
	    if($existing_content) {
		    $qry = db_query("SELECT td.nid FROM {taxon_description} td INNER JOIN {node} n ON (td.nid=n.nid) WHERE n.uid=%d AND td.tid_taxon=%d AND n.language = '%s'", $user->uid, $taxon['identifier'], $lang);
		    while($result = db_fetch_object($qry)) {
			  $node = node_load($result->nid);
			  $data->setCellValueByColumnAndRow(0,$next_row,$taxon['name']);
			  $data->setCellValueByColumnAndRow(1,$next_row,$taxon['identifier']);
			  $data->setCellValueByColumnAndRow(2,$next_row,t('Published'));

			  $cc = ($node) ? $node->cc_lite_license : t('by');
			  $data->setCellValueByColumnAndRow(3,$next_row,$cc);

			  $lang = ($node) ? $node->language : $lang;
			  $data->setCellValueByColumnAndRow(4,$next_row,$lang);

			  foreach($node->taxon_description as $cid => $value) {
				//need to associate $cid with translated $cid here
				if($chapter_cols[$cid]) {
				  $data->setCellValue($chapter_cols[$cid] . $next_row,$node->taxon_description[$cid]['content']);	
				}	
			  }

			  $next_row++;
		    }
	    }
	    else {
			$data->setCellValueByColumnAndRow(0,$next_row,$taxon['name']);
			$data->setCellValueByColumnAndRow(1,$next_row,$taxon['identifier']);
			$data->setCellValueByColumnAndRow(2,$next_row,t('Published'));
			$data->setCellValueByColumnAndRow(3,$next_row,t('by'));
			$data->setCellValueByColumnAndRow(4,$next_row,$lang);
			$next_row++;
	    }
	  }

    // Set worksheet title
    $objPHPExcel->getActiveSheet()->setTitle('Chapter Template');

    // Set orientation to landscape
    $objPHPExcel->getActiveSheet()
              ->getPageSetup()->setOrientation(PHPExcel_Worksheet_PageSetup::ORIENTATION_LANDSCAPE);

    $style1 = new PHPExcel_Style();
    $style2 = new PHPExcel_Style();
    $style3 = new PHPExcel_Style();
    $style4 = new PHPExcel_Style();

    $style1->applyFromArray(array(
		'fill' => array(
		  'type' => PHPExcel_Style_Fill::FILL_SOLID,
		  'color' => array('argb' => '00FFE4C4'),
	 	),
	    'borders' => array(
		  'bottom' => array('style' => PHPExcel_Style_Border::BORDER_THIN),
		  'right' => array('style' => PHPExcel_Style_Border::BORDER_THIN),
		  'left' => array('style' => PHPExcel_Style_Border::BORDER_THIN),
	    ),
	    'alignment' => array(
		  'vertical' => PHPExcel_Style_Alignment::VERTICAL_CENTER,
		),
		'font' => array(
		  'bold' => true,
		),
	  ));
	
	  $style2->applyFromArray(array(
	    'font' => array(
		  'bold' => true,
		  'size' => 14,
		),
	  ));
	
	  $style3->applyFromArray(array(
	    'alignment' => array(
		  'wrap' => true,
		  'vertical' => PHPExcel_Style_Alignment::VERTICAL_TOP,
		),
	  ));
	
	  $style4->applyFromArray(array(
	    'alignment' => array(
		  'vertical' => PHPExcel_Style_Alignment::VERTICAL_CENTER,
		),
		'font' => array(
		  'bold' => true,
		),
	  ));
	
	  // Apply styles
	  $rows = $objPHPExcel->getActiveSheet()->getHighestRow();
	  $cols = $objPHPExcel->getActiveSheet()->getHighestColumn();
	
	  $objPHPExcel->getActiveSheet()->setSharedStyle($style1, "A3:B" . $rows);
	  $objPHPExcel->getActiveSheet()->setSharedStyle($style2, "A1:" . $cols . "2");
	  $objPHPExcel->getActiveSheet()->setSharedStyle($style3, "F3:" . $cols . $rows);
	  $objPHPExcel->getActiveSheet()->setSharedStyle($style4, "C3:E" . $rows);

    // Add comment to A3
    $objPHPExcel->getActiveSheet()->getComment('A3')->setAuthor('LifeDesk Team');
	  $objCommentRichText = $objPHPExcel->getActiveSheet()->getComment('A3')->getText()->createTextRun(t('WARNING:'));
	  $objCommentRichText->getFont()->setBold(true);
	  $objPHPExcel->getActiveSheet()->getComment('A3')->getText()->createTextRun("\r\n");
	  $objPHPExcel->getActiveSheet()->getComment('A3')->getText()->createTextRun(t('Content of Taxon column may not be edited'));

    // Add comment to B3
    $objPHPExcel->getActiveSheet()->getComment('B3')->setAuthor('LifeDesk Team');
	  $objCommentRichText = $objPHPExcel->getActiveSheet()->getComment('B3')->getText()->createTextRun(t('WARNING:'));
	  $objCommentRichText->getFont()->setBold(true);
	  $objPHPExcel->getActiveSheet()->getComment('B3')->getText()->createTextRun("\r\n");
	  $objPHPExcel->getActiveSheet()->getComment('B3')->getText()->createTextRun(t('Content of Identifier column may not be edited'));	

	  // Set the column widths
	  $objPHPExcel->getActiveSheet()->getColumnDimension('A')->setWidth(28);
	  $objPHPExcel->getActiveSheet()->getColumnDimension('B')->setWidth(12);
	  $objPHPExcel->getActiveSheet()->getColumnDimension('C')->setWidth(10);
	  $objPHPExcel->getActiveSheet()->getColumnDimension('D')->setWidth(10);
	  $objPHPExcel->getActiveSheet()->getColumnDimension('E')->setWidth(10);
	
	  // Set the row heights
	  for($i=3;$i<=$rows;$i++) {
	    $objPHPExcel->getActiveSheet()->getRowDimension($i)->setRowHeight(50);	
	  }

    // Make the chapter column widths & apply wrap
    foreach($chapter_cols as $chapter_col) {
	    $objPHPExcel->getActiveSheet()->getColumnDimension($chapter_col)->setWidth(28);
    }

    // Freeze panes
    $objPHPExcel->getActiveSheet()->freezePane("B3");

    // Set Published vs Draft in one cell (will be cloned)
    $objValidation = $objPHPExcel->getActiveSheet()->getCell('C3')->getDataValidation();
	  $objValidation->setType( PHPExcel_Cell_DataValidation::TYPE_LIST );
	  $objValidation->setErrorStyle( PHPExcel_Cell_DataValidation::STYLE_INFORMATION );
	  $objValidation->setAllowBlank(false);
	  $objValidation->setShowInputMessage(true);
	  $objValidation->setShowErrorMessage(true);
	  $objValidation->setShowDropDown(true);
	  $objValidation->setErrorTitle(t('Input error'));
	  $objValidation->setError(t('Value is not in list.'));
	  $objValidation->setPromptTitle(t('Pick from list'));
	  $objValidation->setPrompt(t('Please pick a value from the drop-down list.'));
	  $objValidation->setFormula1('"Published,Draft"');
	  $objPHPExcel->getActiveSheet()->getCell('C3')->setDataValidation($objValidation);

    // Set Licensing options in one cell (will be cloned)  NOTE: only relevant to Excel 2007
    $objValidation2 = $objPHPExcel->getActiveSheet()->getCell('D3')->getDataValidation();
	  $objValidation2->setType( PHPExcel_Cell_DataValidation::TYPE_LIST );
	  $objValidation2->setErrorStyle( PHPExcel_Cell_DataValidation::STYLE_INFORMATION );
	  $objValidation2->setAllowBlank(false);
	  $objValidation2->setShowInputMessage(true);
	  $objValidation2->setShowErrorMessage(true);
	  $objValidation2->setShowDropDown(true);
	  $objValidation2->setErrorTitle(t('Input error'));
	  $objValidation2->setError(t('Value is not in list.'));
	  $objValidation2->setPromptTitle(t('Pick from list'));
	  $objValidation2->setPrompt(t('Please pick a value from the drop-down list.'));
	  $objValidation2->setFormula1('"by,by-nc,by-nc-sa,by-sa,publicdomain"');
	  $objPHPExcel->getActiveSheet()->getCell('D3')->setDataValidation($objValidation2);

	// Set Language options in one cell (will be cloned)  NOTE: only relevant to Excel 2007
    $objValidation3 = $objPHPExcel->getActiveSheet()->getCell('E3')->getDataValidation();
	  $objValidation3->setType( PHPExcel_Cell_DataValidation::TYPE_LIST );
	  $objValidation3->setErrorStyle( PHPExcel_Cell_DataValidation::STYLE_INFORMATION );
	  $objValidation3->setAllowBlank(false);
	  $objValidation3->setShowInputMessage(true);
	  $objValidation3->setShowErrorMessage(true);
	  $objValidation3->setErrorTitle(t('Input error'));
	  $objValidation3->setError(t('Value is not accepted.'));
	  $objValidation3->setPromptTitle(t('Language Limitation'));
	  $objValidation3->setPrompt(t('Language must be !lang in this template', array('!lang' => $lang)));
	
	  $objValidation3->setFormula1('"' . $lang . '"');
	  $objPHPExcel->getActiveSheet()->getCell('E3')->setDataValidation($objValidation3);
	
    // Apply cloned validation to remaining cells  NOTE: only relevant to Excel 2007
    for($i=4;$i<=$rows;$i++) {
      $objPHPExcel->getActiveSheet()->getCell('C'.$i)->setDataValidation(clone $objValidation);
      $objPHPExcel->getActiveSheet()->getCell('D'.$i)->setDataValidation(clone $objValidation2);
      $objPHPExcel->getActiveSheet()->getCell('E'.$i)->setDataValidation(clone $objValidation3);
    }
	
	// Set active sheet index to the first sheet, so Excel opens this as the first sheet
	$objPHPExcel->setActiveSheetIndex(0);
	
	return $objPHPExcel;
}