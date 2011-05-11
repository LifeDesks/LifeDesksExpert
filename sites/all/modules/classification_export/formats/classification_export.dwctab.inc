<?php

function _classification_export_dwc_tab($obj, $shortname='') {
	  global $base_url;
      $output  = array();

	  $output['classification'] = t("TaxonID") . "\t" . t("dc:source") . "\t" . t("ScientificName") . "\t" . t("HigherTaxonID") . "\t" . t("TaxonRank") . "\t" . t("TaxonomicStatus") . "\n";
	
	  $output['vernaculars'] = t("TaxonID") . "\t" . t("vernacularName") . "\t" . t("languageCode") . "\n";

	  foreach($obj as $row) {
		//$obj includes everything so need to constrain to those names w/o a taxonomic relationship to a parent
		if(!$row->status) {
			$synonym_row = '';
			$vernacular_row = '';

			if (!empty($row->synonyms)) {
			  foreach($row->synonyms as $synonym) {
				$output['classification'] .= $shortname . ":tid:" . $synonym->tid . "\t" . $base_url . "/pages/" . $synonym->tid . "\t" . trim($synonym->name) . "\t" . $shortname . ":tid:" . $row->tid . "\t" . "\t" . $synonym->type . "\n";
			  }
			}
			if (!empty($row->vernaculars)) {
			  foreach($row->vernaculars as $vernacular) {
				$output['vernaculars'] .= $shortname . ":tid:" . $row->tid . "\t" . trim($vernacular->name) . "\t" . $vernacular->iso . "\n";
			  }
			}
			$parent = ($row->parent != 0) ? $shortname . ":tid:" . $row->parent : "/N";
			$output['classification'] .= $shortname . ":tid:" . $row->tid . "\t" . $base_url . "/pages/" . $row->tid . "\t" . trim($row->name) . "\t" . $parent . "\t" . $row->rank . "\t" . "\n";
		  }
		}
		
	return $output;
}