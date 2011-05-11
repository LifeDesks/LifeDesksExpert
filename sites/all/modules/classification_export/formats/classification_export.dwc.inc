<?php

function _classification_export_dwc($obj,$shortname='') {
  echo "\n";
  foreach($obj as $row) {
	if(!$row->status) {
      echo "<dwc:Taxon>" . "\n";
      echo "<dwc:taxonID>" . $shortname . ":tid:" . $row->tid . "</dwc:taxonID>" . "\n";
      if($row->parent) {
	    echo "<dwc:parentNameUsageID>" . $shortname . ":tid:" . $row->parent . "</dwc:parentNameUsageID>" . "\n";
      }
      echo "<dwc:scientificName>" . _classification_export_encode(trim($row->name)) . "</dwc:scientificName>" . "\n";
      if($row->rank) {
	    echo "<dwc:taxonRank>" . strtolower($row->rank) . "</dwc:taxonRank>" . "\n";
      }
      if($row->description) {
	    echo "<dwc:taxonRemarks>" . _classification_export_encode(trim($row->description)) . "</dwc:taxonRemarks>" . "\n";
      }
      $status = ($row->taxonomic_status) ? _classification_export_encode($row->taxonomic_status) : "";
      echo "<dwc:taxonomicStatus>" . $status . "</dwc:taxonomicStatus>" . "\n";
      if($row->vernaculars) {
	    foreach($row->vernaculars as $vernacular) {
		  $language = ($vernacular->iso) ? " xml:lang=\"" . $vernacular->iso . "\"" : "";
		  echo "<dwc:vernacularName" . $language . ">" . _classification_export_encode(trim($vernacular->name)) . "</dwc:vernacularName>" . "\n";
	    }
      }
      echo "</dwc:Taxon>" . "\n";
    
      if($row->synonyms) {
	    foreach($row->synonyms as $synonym) {
		  echo "<dwc:Taxon>" . "\n";
		  echo "<dwc:taxonID>" . $shortname . ":tid:" . $synonym->tid . "</dwc:taxonID>" . "\n";
		  echo "<dwc:parentNameUsageID>" . $shortname . ":tid:" . $row->tid . "</dwc:parentNameUsageID>" . "\n";
		  echo "<dwc:scientificName>" . _classification_export_encode(trim($synonym->name)) . "</dwc:scientificName>" . "\n";
		  echo "<dwc:taxonomicStatus>" . $synonym->type . "</dwc:taxonomicStatus>" . "\n";
		  echo "</dwc:Taxon>" . "\n";
	    }
      }
    }
  }

  
}