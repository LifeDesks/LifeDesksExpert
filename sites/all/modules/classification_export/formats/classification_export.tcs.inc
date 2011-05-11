<?php

function _classification_export_tcs($obj) {

  module_load_include("php", "classification", "includes/classification_functions.class");

  $Names = new Names;

  //make the TaxonNames element
  echo "<TaxonNames>" . "\n";
  foreach($obj as $row) {
	$code = ($row->nomenclatural_code) ? _classification_export_encode($row->nomenclatural_code) : "Zoological";
    echo "<TaxonName id=\"n".$row->tid."\" nomenclaturalCode=\"" . $code . "\">" . "\n";
    echo "<Simple>" . _classification_export_encode(trim($row->name)) . "</Simple>" . "\n";
    if($row->rank) {
	  echo "<Rank>" . _classification_export_encode(strtolower($row->rank)) . "</Rank>" . "\n";
    }
    if(!$row->status) {
	  echo "<CanonicalName>" . "\n";
	  echo "<Simple>" . _classification_export_encode($Names->canonical_form(trim($row->name))) . "</Simple>" . "\n";
	  echo "</CanonicalName>" . "\n";
    }
    echo "</TaxonName>" . "\n";
  }
  echo "</TaxonNames>" . "\n";

  //make the TaxonConcepts element
  echo "<TaxonConcepts>" . "\n";
  foreach($obj as $row) {
    echo "<TaxonConcept id=\"c" . $row->tid . "\">" . "\n";
    $scientific = ($row->status == "vernacular name") ? "false" : "true";
    $language = ($row->status == "vernacular name") ? " language=\"" . $obj[$row->parent]->vernaculars[$row->tid]->iso . "\"" : "";
    echo "<Name scientific=\"" . $scientific . "\" ref=\"n" . $row->tid . "\"" . $language . ">" . _classification_export_encode(trim($row->name)) . "</Name>" . "\n";
    if($row->rank) {
	  echo "<Rank>" . _classification_export_encode(strtolower($row->rank)) . "</Rank>" . "\n";
    }

    if($row->parent) echo "<TaxonRelationships>" . "\n";

    if($row->parent) {
	  if(!$row->status) {
	    $relationship = 'is child taxon of';	
	  }
	  elseif($row->status == 'vernacular name') {
	    $relationship = 'is vernacular for';
	  }
	  else {
	    //no relationship 'is synonym for' defined in TCS
	    $relationship = 'is child taxon of';	
	  }
	  echo "<TaxonRelationship type=\"" . $relationship . "\">" . "\n";
	  echo "<ToTaxonConcept ref=\"c" . $row->parent . "\" linkType=\"local\"></ToTaxonConcept>" . "\n";
	  echo "</TaxonRelationship>" . "\n";
    }
    if($row->synonyms) {
	  foreach($row->synonyms as $synonym) {
	    echo "<TaxonRelationship type=\"has synonym\">" . "\n";
	    echo "<ToTaxonConcept ref=\"c" . $synonym->tid . "\" linkType=\"local\"></ToTaxonConcept>" . "\n";
	    echo "</TaxonRelationship>" . "\n";	
	  }
    }
    if($row->vernaculars) {
	  foreach($row->vernaculars as $vernacular) {
	    echo "<TaxonRelationship type=\"has vernacular\">" . "\n";
	    echo "<ToTaxonConcept ref=\"c" . $vernacular->tid . "\" linkType=\"local\"></ToTaxonConcept>" . "\n";
	    echo "</TaxonRelationship>" . "\n";	
	  }
    }
    
    if($row->parent) echo "</TaxonRelationships>" . "\n";

    echo "</TaxonConcept>" . "\n";	
  }
  echo "</TaxonConcepts>" . "\n";
}
