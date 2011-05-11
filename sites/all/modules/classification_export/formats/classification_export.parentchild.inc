<?php

function _classification_export_parentchild($obj) {

  echo t("Identifier") . "\t" . t("Parent") . "\t" . t("Child") . "\t" . t("Rank") . "\t" . t("Synonyms") . "\t" . t("Vernaculars") . "\t" . t("VernacularsLanguage") . "\t" . t("Description") . "\n";
 
  foreach($obj as $row) {
	//$obj includes everything so need to constrain to those names w/o a taxonomic relationship to a parent
	if(!$row->status) {
		$synonym_row = '';
		$vernacular_row = '';
		if (!empty($row->synonyms)) {
		  $synonym_row = '';
		  $last_syn = end($row->synonyms);
		  foreach($row->synonyms as $synonym) {
		    $synonym_row .= trim($synonym->name);
		    if($synonym	!= $last_syn) $synonym_row .= '|';
		  }
		}
		if (!empty($row->vernaculars)) {
		  $vernacular_row = '';
		  $vernacular_lang = '';
		  $last_vern = end($row->vernaculars);
		  foreach($row->vernaculars as $vernacular) {
		    $vernacular_row .= trim($vernacular->name);
		    if($vernacular != $last_vern) $vernacular_row .= '|';
		    $vernacular_lang .= trim($vernacular->iso);
		    if($vernacular != $last_vern) $vernacular_lang .= '|';
		  }
		}
		echo $row->tid . "\t" . $row->parent . "\t" . trim($row->name) . "\t" . $row->rank . "\t" . $synonym_row . "\t" . $vernacular_row . "\t" . $vernacular_lang . "\t" . trim($row->description) . "\n";
	  }
	}
}

function _classification_export_parentchild_excel($obj) {
	  module_load_include('php','excel_reader','includes/PHPExcel');
	  module_load_include('php','excel_reader','includes/PHPExcel/IOFactory');

	  // Create new PHPExcel object
	  $objPHPExcel = new PHPExcel();
	
	  // Set properties
	  $objPHPExcel->getProperties()->setCreator($user->firstname . " " . $user->lastname)
								 ->setLastModifiedBy($user->firstname . " " . $user->lastname)
								 ->setTitle($site_name . " " . t('Classification'))
								 ->setSubject(t('!sitename Classification', array('!sitename' => $sitename)))
								 ->setDescription(t('An export of a classification.'))
								 ->setKeywords("LifeDesk")
								 ->setCategory("");

	  // Add some data
	  $data = $objPHPExcel->setActiveSheetIndex(0);
	  
	  $data->setCellValue('A1', 'Identifier')
	            ->setCellValue('B1', 'Parent')
	            ->setCellValue('C1', 'Child')
	            ->setCellValue('D1', 'Rank')
	            ->setCellValue('E1', 'Synonyms')
	            ->setCellValue('F1', 'Vernaculars')
	            ->setCellValue('G1', 'VernacularsLanguage')
	            ->setCellValue('H1', 'Description');

    $row_num = 2;
    foreach($obj as $row) {
	    if(!$row->status) {
	      $data->setCellValue('A' . $row_num, $row->tid)
	           ->setCellValue('B' . $row_num, $row->parent)
	           ->setCellValue('C' . $row_num, trim($row->name));
	      if($row->rank) {
		    $data->setCellValue('D' . $row_num, trim($row->rank));
	      }
	      if($row->synonyms) {
		    $synonym_row = '';
		    $last_syn = end($row->synonyms);
		    foreach($row->synonyms as $synonym) {
		      $synonym_row .= trim($synonym->name);
		      if($synonym	!= $last_syn) $synonym_row .= '|';
		    }
		    $data->setCellValue('E' . $row_num, $synonym_row);
	      }
	      if($row->vernaculars) {
		    $vernacular_row = '';
		    $vernacular_lang = '';
		    $last_vern = end($row->vernaculars);
		    foreach($row->vernaculars as $vernacular) {
		      $vernacular_row .= trim($vernacular->name);
		      if($vernacular != $last_vern) $vernacular_row .= '|';
		      $vernacular_lang .= trim($vernacular->iso);
		      if($vernacular != $last_vern) $vernacular_lang .= '|';
		    }
		    $data->setCellValue('F' . $row_num, $vernacular_row);
		    $data->setCellValue('G' . $row_num, $vernacular_lang);
	      }
	      if($row->description) {
		    $data->setCellValue('H' . $row_num, trim($row->description));
	      }
	      $row_num++;
	    }
    }

	  $objPHPExcel->getActiveSheet()->getColumnDimension('A')->setWidth(10);
	  $objPHPExcel->getActiveSheet()->getColumnDimension('B')->setWidth(10);
	  $objPHPExcel->getActiveSheet()->getColumnDimension('C')->setWidth(36);
	  $objPHPExcel->getActiveSheet()->getColumnDimension('D')->setWidth(18);
	  $objPHPExcel->getActiveSheet()->getColumnDimension('E')->setWidth(28);
	  $objPHPExcel->getActiveSheet()->getColumnDimension('F')->setWidth(28);
	  $objPHPExcel->getActiveSheet()->getColumnDimension('G')->setWidth(28);
	  $objPHPExcel->getActiveSheet()->getColumnDimension('H')->setWidth(36);

    $style = new PHPExcel_Style();
    $style1 = new PHPExcel_Style();
	  $style->applyFromArray(array(
	    'font' => array(
		  'bold' => true,
		  'size' => 14,
		),
	  ));
	  $style1->applyFromArray(array(
	    'alignment' => array(
		  'wrap' => true,
		  'vertical' => PHPExcel_Style_Alignment::VERTICAL_TOP,
		),
	  ));
	
	  $rowcount = $objPHPExcel->getActiveSheet()->getHighestRow();
	  $objPHPExcel->getActiveSheet()->setSharedStyle($style, "A1:H1");
	  $objPHPExcel->getActiveSheet()->setSharedStyle($style1, "A2:H" . $rowcount);
	  $objPHPExcel->getActiveSheet()->freezePane("A2");
	
	return $objPHPExcel;
}
		
