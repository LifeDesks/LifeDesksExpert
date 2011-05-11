<?php

define("EOL_SERVICES_URL","http://services.eol.org/lifedesk/service.php?");

/**
 * Classification search callback on web service
 * 
 * @return
 * JSON data with status, message, and data objects. Latter will result in construction of radio buttons from which to choose a classification for import
 */
function classification_import_tcs_search() {
  $form = array(
    '#post' => $_POST,
  );

  $name = urlencode($_REQUEST['search']);

  $response = drupal_http_request(EOL_SERVICES_URL . "function=search&search=" . $name);

  if(!$response->code == 200) {
	$status = "failed";
	$message = t("Sorry, this service is currently unavailable");
	$data = "";  	
  }
  else {	
	$status = "ok";
	$result = simplexml_load_string($response->data);
	if($result->value) {
	  $message = t('Select one of the following classification(s) and you will be presented with an import button.');
	  $data = _classification_import_tcs_object_to_array($result);
	}
	else {
	  $message = t('Sorry, there was nothing found by that name');
	  $data = "";
	}
  }

  drupal_json(array("status" => $status, "message" => $message, "data" => $data));
}

/**
 * Classification import callback, executes the initiation of recursive web service calls
 */
function classification_import_tcs() {

  global $vid;

  $form = array(
    '#post' => $_POST,
  );
  
  $tid = $_REQUEST['tid'];
  $vid = $_REQUEST['vid'];
  $tot_count = $_REQUEST['tot_count'];
  
  $form_state = array('submitted' => FALSE);
  
  if($tid && $vid) {
    $url = EOL_SERVICES_URL . 'function=details_tcs&id=' . $tid;
    _classification_import_tcs_init($url,$vid,$tot_count);
  }
}
 
/**
 * Start the loop process by first getting the highest tid in the {term_data} table
 * @param $url
 *   The URL of the web service with function parameters
 *
 * @param $vid
 *   The vocabulary id
 *
 * @param $tot_count
 *  The total number of names that will be imported. Used for user feedback.
 *
 * @return
 *
**/
function _classification_import_tcs_init($url, $vid, $tot_count) {

   global $id, $counter, $user, $relations, $syn_tid, $vern_tid;

   if(!$vid) {
     drupal_set_message(t('Sorry, you need to select a classification before you may import names.'),'error');
     return;
   }

   //get the rank, relations, and vernacular vocabs for later reuse (again, because this was a callback)
   $relations = array();
   $query = db_query("SELECT rtid,name,vid FROM {term_relation_types}");
   while($result = db_fetch_array($query)) {
     $relations[$result['name']] = $result; 	
   }

   //get the tid for the term 'synonym' for later reuse and if not found, then add it
   $syn_tid = db_result(db_query("SELECT tid FROM {term_data} WHERE vid=%d and LOWER(name)='%s'",$relations['taxonomic relationships']['vid'],'synonym'));
   if(!$syn_tid) {
     $syn_term = array(
	   'name' => 'synonym',
	   'vid' => $relations['taxonomic relationships']['vid'],
	   'parent' => 0,
	 );	
	 taxonomy_save_term($syn_term);
	 $syn_tid = $syn_term['tid'];
   }

   $vern_tid = db_result(db_query("SELECT tid FROM {term_data} WHERE vid=%d AND name='vernacular name'",$relations['taxonomic relationships']['vid']));

   _classification_lock_classification();

   $counter = 1;

   $cache_data = array(
	    'curr_count' => "$counter",
	    'term' => "" . t('Getting started...') . "",
	    'vid' => "$vid",
	  );
   cache_set('classification_name_' . $user->uid . $user->login, $cache_data);

   $txn = new Transaction();
   _classification_import_tcs_loop($url, $vid, 'full');
	
   cache_clear_all('classification_name_' . $user->uid . $user->login,'cache');

   _classification_unlock_classification();

}

/**
 * Classification import status callback for user feedback while importing names
 *
 * @param $tot_count = ''
 *   The total number of names that will be imported
 *
 * @return
 *   JSON data with objects 'status', 'curr_count', 'tot_count', and 'name'
 */
function classification_import_tcs_status($tot_count='') {
	header('Content-Type: text/plain; charset=utf-8');
	header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0, max-age=0');
	header('Pragma: no-cache');
	header('Expires: Thu, 01 Jan 1970 00:00:00 GMT');
	
	global $user;
  
    $data = cache_get('classification_name_' . $user->uid . $user->login);
	$name = check_plain($data->data['term']);
	$curr_count = $data->data['curr_count'];
	$vid = $data->data['vid'];
    
    drupal_json(array('status' => 'success', 'curr_count' => $curr_count ,'tot_count' => $tot_count, 'name' => $name, 'vid' => $vid));
}

/**
 *  Recursion function for import of names from a TCS web service
 *
 * @param $url
 *   The URL of the web service with appropriate parameters passed
 * @param $type
 *   Possible values 'full', 'syn', and 'vern' to differentially handle taxonomic name, synonym, or vernacular
 * @param $tid=''
 *   The term id
 * @param $pid=false
 *   The parent id
 * @return
 *
**/
function _classification_import_tcs_loop($url, $vid, $type, $tid='', $pid=false) {

   global $id,$user,$counter,$temp_table,$relations,$syn_tid,$vern_tid;

   $response = drupal_http_request($url);
   if(!$response->code == 200) {
      drupal_set_message(t('Sorry, that URL is unreachable. If an import ended prematurely, you may want to delete the classification and start the process again.'), 'error');
      return;
   }
  
   $id++;

   if(!$pid) {
     $pid = 0;	
   }

   $data = $response->data;
   $xml = simplexml_load_string($data);

   $TaxonNames = $xml->TaxonNames;
   $TaxonConcept = $xml->TaxonConcepts->TaxonConcept;
   $TaxonRelationships = $TaxonConcept->TaxonRelationships;
   $ProviderSpecificData = $TaxonNames->TaxonName->ProviderSpecificData->NameSources;

   $simple_name = $TaxonNames->TaxonName->Simple;
   $canonical_name = $TaxonNames->TaxonName->CanonicalName->Simple;
   $rank = strtolower($TaxonNames->TaxonName->Rank);

  switch ($type) {
    case 'full':
      $term = array(
        'name' => $simple_name,
        'parent' => $pid,
        'vid' => $vid,
      );
      taxonomy_save_term($term);

      if(trim($rank) != "") {
	      //insert the rank relationship & if not recognized, then add it
	      $rank_db = db_result(db_query("SELECT td.tid FROM {term_data} td WHERE td.vid=%d AND LOWER(td.name)='%s'",$relations['taxonomic ranks']['vid'],$rank));
	      if(!$rank_db) {
		    $new_rank = array(
			  'name' => $rank,
			  'vid' => $relations['taxonomic ranks']['vid'],
			  'parent' => 0,
			);
			taxonomy_save_term($new_rank);
			$rank_db = $new_rank['tid'];
	      }

	      db_query("INSERT INTO {term_relation} (tid1,tid2) VALUES (%d,%d)",$term['tid'],$rank_db);
	      $trid = db_last_insert_id('term_relation','trid');

	      db_query("INSERT INTO {term_relation_has_type} (trid,rtid) VALUES (%d,%d)",$trid,$relations['taxonomic ranks']['rtid']);
      }
   
      $cache_data = array(
	    'curr_count' => "$counter",
	    'term' => "$simple_name",
	    'vid' => "$vid",
	  );
      cache_set('classification_name_' . $user->uid . $user->login, $cache_data);
    break;
    case 'syn':
      $syn = array(
	    'name' => $simple_name,
	    'vid' => $vid,
	    'parent' => $pid,
	  );
	  taxonomy_save_term($syn);

      db_query("INSERT INTO {term_relation} (tid1,tid2) VALUES (%d,%d)",$syn['tid'],$syn_tid);
      $trid = db_last_insert_id('term_relation','trid');

      db_query("INSERT INTO {term_relation_has_type} (trid,rtid) VALUES (%d,%d)",$trid,$relations['taxonomic relationships']['rtid']);

      $cache_data = array(
	    'curr_count' => "$counter",
	    'term' => "$simple_name",
	    'vid' => "$vid",
	  );
      cache_set('classification_name_' . $user->uid . $user->login,$cache_data);
    break;
    case 'vern':
      $simple_name = $TaxonConcept->Name;
      $language = $TaxonConcept->Name->attributes()->language;

      $vern = array(
	    'name' => $simple_name,
	    'vid' => $vid,
	    'parent' => $pid,
	  );
	  taxonomy_save_term($vern);
	
	  //we need the vern tid for the taxonomic relationship
	  db_query("INSERT INTO {term_relation} (tid1,tid2) VALUES (%d,%d)",$vern['tid'],$vern_tid);
	  $trid = db_last_insert_id('term_relation','trid');
	
	  db_query("INSERT INTO {term_relation_has_type} (trid,rtid) VALUES (%d,%d)",$trid,$relations['taxonomic relationships']['rtid']);
	
	  //now we do the same for the ISO language code but only if we have a match
	  if($language) {
		$lang_tid = db_result(db_query("SELECT tid FROM {term_data} WHERE vid=%d AND RIGHT(name,2)='%s'",$relations['names to vernacular codes']['vid'],$language));
		if($lang_tid) {
		  db_query("INSERT INTO {term_relation} (tid1,tid2) VALUES (%d,%d)",$vern['tid'],$lang_tid);
		  $trid = db_last_insert_id('term_relation','trid');

		  db_query("INSERT INTO {term_relation_has_type} (trid,rtid) VALUES (%d,%d)",$trid,$relations['names to vernacular codes']['rtid']);	
		}	
	  }

      $cache_data = array(
	    'curr_count' => "$counter",
	    'term' => "$simple_name",
	    'vid' => "$vid",
	  );
      cache_set('classification_name_' . $user->uid . $user->login,$cache_data);
    break;
  }

  //Slow down the recursion a hundredth of a second
  usleep(10000);
  $counter++;
  
  if($type == 'full') { 
    foreach ($TaxonRelationships->TaxonRelationship as $relationships) {
  	 switch ($relationships->attributes()->type) {
  	   case "is child taxon of":
  	     break;
  	   case "is parent taxon of":
  	     $url = $relationships->ToTaxonConcept->attributes()->ref;
  	     _classification_import_tcs_loop($url,$vid,'full',$id,$term['tid']);
  	     break;
  	   case "has synonym":
  	     $url = $relationships->ToTaxonConcept->attributes()->ref;
  	     _classification_import_tcs_loop($url,$vid,'syn',$id,$term['tid']);
         break;
       case "has vernacular":
         $url = $relationships->ToTaxonConcept->attributes()->ref;
         _classification_import_tcs_loop($url,$vid,'vern',$id,$term['tid']);
         break;
  	 }
    }	     
  }
}

/**
 * Helper function to recursively convert objects to array
 */
function _classification_import_tcs_object_to_array($mixed) {
    if(is_object($mixed)) $mixed = (array) $mixed;
    if(is_array($mixed)) {
        $new = array();
        foreach($mixed as $key => $val) {
            $key = preg_replace("/^\\0(.*)\\0/","",$key);
            $new[$key] = _classification_import_tcs_object_to_array($val);
        }
    }
    else $new = $mixed;
    return $new;       
}

function theme_classification_import_tcs_message(){

  $tot_count = $_REQUEST['tot_count'];
  $status = $_REQUEST['status'];
  $vid = $_REQUEST['vid'];
    
  $json = array(
    'message' => '<div class="messages status" id="import_message">',
  );

  switch ($status){
    case 'fail':
      $json['message'] .= t('Import complete.');
      break;
  	case 'success':
  	  $json['message'] .= t('Import complete (or is almost done).');
  	  break;
  }
  $json['message'] .= ' ';
  $json['message'] .= t('!link your classification', array('!link' => l(t('View and Edit'), 'admin/classification/biological/edit/' . $vid)));
  $json['message'] .= '</div>';

  return drupal_json($json);
}