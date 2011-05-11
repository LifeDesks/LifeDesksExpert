<?php

function _classification_export_fullhierarchy($obj) {

  echo t("Identifier") . "\t";

  $total = array();
  $parents = array();

  foreach($obj as $row) {
	$arr = array_reverse(_classification_export_get_parents($obj,$row->tid),true);
	$total[$row->tid] = count($arr);
	$parents[$row->tid] = $arr;
  }

  $total = max($total);

  for($i=1;$i<=$total;$i++) {
    echo t("Taxon" . $i) . "\t";	
  }

  echo t("Rank") . "\t" . t("Synonyms") . "\t" . t("Vernaculars") . "\t" . t("VernacularsLanguage") . "\t" . t("Description") . "\n";

  foreach($obj as $row) {
	//$obj includes everything so need to constrain to those names w/o a taxonomic relationship to a parent
	if(!$row->status) {
	  echo $row->tid . "\t";
	  $taxon_parent_count = count($parents[$row->tid]);
      foreach($parents[$row->tid] as $parent) {
	    echo $parent . "\t";
      }
      echo trim($row->name) . "\t";

      for($i=1;$i<=$total-$taxon_parent_count;$i++) {
	    echo "\t";
      }

      echo trim($row->rank) . "\t";

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
	      $vernacular_lang .= $vernacular->iso;
	      if($vernacular != $last_vern) $vernacular_lang .= '|';
	    }
	  }
	
	  echo $synonym_row . "\t";
	  echo $vernacular_row . "\t" . $vernacular_lang . "\t";
	  echo trim($row->description) . "\n";
    }
  }
}

function _classification_export_get_parents($obj, $tid, $a=array()) {
	$parent = $obj[$tid]->parent;
	if($parent != 0 && $parent != $tid) $a[$parent] = $obj[$parent]->name;
	if($parent) {
	  $a = _classification_export_get_parents($obj, $parent, $a);
	}
	return $a;
}

function _classification_export_fullhierarchy_excel($obj) {
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
	
	  $total = array();
	  $parents = array();
	  foreach($obj as $row) {
		$arr = array_reverse(_classification_export_get_parents($obj,$row->tid));
		$total[$row->tid] = count($arr);
		$parents[$row->tid] = $arr;
	  }
	
	  $total = max($total);
	  
	  $data->setCellValue('A1', 'Identifier');
	
	  for($i=1;$i<=$total;$i++) {
		$data->setCellValue(PHPExcel_Cell::stringFromColumnIndex($i) . '1', t('Taxon') . $i);	
	  }

	  $data->setCellValue(PHPExcel_Cell::stringFromColumnIndex($total+1) . '1', t('Rank'))
	            ->setCellValue(PHPExcel_Cell::stringFromColumnIndex($total+2) . '1', t('Synonyms'))
	            ->setCellValue(PHPExcel_Cell::stringFromColumnIndex($total+3) . '1', t('Vernaculars'))
	            ->setCellValue(PHPExcel_Cell::stringFromColumnIndex($total+4) . '1', t('VernacularsLanguage'))
	            ->setCellValue(PHPExcel_Cell::stringFromColumnIndex($total+5) . '1', t('Description'));

    $row_num = 2;

    foreach($obj as $row) {
	    if(!$row->status) {

	      $data->setCellValue('A' . $row_num, $row->tid);
	
	      $taxon_parent_count = count($parents[$row->tid]);
	
	      $colcount = 1;
	      foreach($parents[$row->tid] as $parent) {
		    $data->setCellValue(PHPExcel_Cell::stringFromColumnIndex($colcount) . $row_num, trim($parent));
		    $colcount++;
	      }
	      
	      $data->setCellValue(PHPExcel_Cell::stringFromColumnIndex($colcount) . $row_num, trim($row->name));
	      $colcount = $total-$taxon_parent_count+$colcount++;
	
	      if($row->rank) {
		    $data->setCellValue(PHPExcel_Cell::stringFromColumnIndex($colcount) . $row_num, trim($row->rank));
	      }
	      $colcount++;
	
	      if($row->synonyms) {
		    $synonym_row = '';
		    $last_syn = end($row->synonyms);
		    foreach($row->synonyms as $synonym) {
		      $synonym_row .= trim($synonym->name);
		      if($synonym != $last_syn) $synonym_row .= '|';
		    }
		    $data->setCellValue(PHPExcel_Cell::stringFromColumnIndex($colcount) . $row_num, $synonym_row);
	      }
	      $colcount++;
	
	      if($row->vernaculars) {
		    $vernacular_row = '';
		    $last_vern = end($row->vernaculars);
		    foreach($row->vernaculars as $vernacular) {
		      $vernacular_row .= trim($vernacular->name);
		      if($vernacular != $last_vern) $vernacular_row .= '|';
		    }
		    $data->setCellValue(PHPExcel_Cell::stringFromColumnIndex($colcount) . $row_num, $vernacular_row);
	      }
	      $colcount++;

	      if($row->vernaculars) {
		    $vernacular_lang = '';
		    $last_vern = end($row->vernaculars);
		    foreach($row->vernaculars as $vernacular) {
		      $vernacular_lang .= trim($vernacular->iso);
		      if($vernacular != $last_vern) $vernacular_lang .= '|';
		    }
		    $data->setCellValue(PHPExcel_Cell::stringFromColumnIndex($colcount) . $row_num, $vernacular_lang);
	      }
	      $colcount++;
	
	      if($row->description) {
		    $data->setCellValue(PHPExcel_Cell::stringFromColumnIndex($colcount) . $row_num, trim($row->description));
	      }
	      $row_num++;

	    }
    }

	$objPHPExcel->getActiveSheet()->getColumnDimension('A')->setWidth(10);
	
	for($i=1;$i<=$total;$i++) {
	  $objPHPExcel->getActiveSheet()->getColumnDimension(PHPExcel_Cell::stringFromColumnIndex($i))->setWidth(36);
	}
	
	$objPHPExcel->getActiveSheet()->getColumnDimension(PHPExcel_Cell::stringFromColumnIndex($total+1))->setWidth(18);
    $objPHPExcel->getActiveSheet()->getColumnDimension(PHPExcel_Cell::stringFromColumnIndex($total+2))->setWidth(28);
    $objPHPExcel->getActiveSheet()->getColumnDimension(PHPExcel_Cell::stringFromColumnIndex($total+3))->setWidth(28);
    $objPHPExcel->getActiveSheet()->getColumnDimension(PHPExcel_Cell::stringFromColumnIndex($total+4))->setWidth(28);
	$objPHPExcel->getActiveSheet()->getColumnDimension(PHPExcel_Cell::stringFromColumnIndex($total+5))->setWidth(36);

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
	
	  $objPHPExcel->getActiveSheet()->setSharedStyle($style, "A1:" . PHPExcel_Cell::stringFromColumnIndex($total+5) . "1");
      $objPHPExcel->getActiveSheet()->setSharedStyle($style1, "A2:" . PHPExcel_Cell::stringFromColumnIndex($total+5) . $row_num--);
	  $objPHPExcel->getActiveSheet()->freezePane("A2");
	
	return $objPHPExcel;
}
		
