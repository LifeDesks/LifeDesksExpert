<?php

/**
 * Callback for rendering tree
 */
 function classification_js_tree($vid,$type='xml',$tid='',$search='') {
   
     global $NAMES, $NAMES_OPTIONS_ELEMENTS;

     $matches = array();
 	  
     if($_GET['tid']) { 	 
       $tid = $_GET['tid'];
     }
 
     if(!$tid) {
        $tid = 0;
     }
     else {
        $tid = str_replace("n", "",$tid);
     }

     if($_GET['search']) {
	   $search = $_GET['search'];
     }
     else {
	   $search = '';
     }

     if(!$search) {
     
     switch ($NAMES_OPTIONS_ELEMENTS) {
     	   // Show all names
     	   case 0:
         $query = db_query("
           SELECT 
             td.tid as tid,
             td.name as name,
             td2.name as relationship,
             CASE COALESCE(td2.name,'valid') WHEN 'valid' THEN 0 WHEN 'vernacular name' THEN 2 ELSE 1 END AS relation_order
           FROM 
             {term_data} td 
           INNER JOIN 
             {term_hierarchy} th ON (td.tid=th.tid) 
           LEFT JOIN 
             ({term_relation} tr
	            INNER JOIN
	            {term_data} td2 ON (tr.tid2=td2.tid)
	           ) ON (tr.tid1=td.tid AND td2.vid=%d)
           WHERE
             th.parent=%d AND td.vid=%d",CLASSIFICATION_RELATIONS,$tid,$vid);
         break;
	       // Hide taxonomic relationships & only show valid names
	       case 1:
	         $query = db_query("
	           SELECT
	             td.tid,
	             td.name,
	             null as relationship,
	             0 as relation_order
	           FROM
	             {term_data} td
	           INNER JOIN
	             {term_hierarchy} th ON (td.tid=th.tid) 
	           LEFT JOIN
	             ({term_relation} tr
	              INNER JOIN
	              {term_data} td2 ON (tr.tid2=td2.tid)
	             ) ON (tr.tid1=td.tid AND td2.vid = %d)
	         WHERE
	           th.parent=%d AND td.vid=%d AND ISNULL(td2.name)",CLASSIFICATION_RELATIONS,$tid,$vid);
	         break;
	       // Show all names again
	       default:
	         $query = db_query("
	           SELECT 
	             td.tid as tid,
	             td.name as name,
	             td2.name as relationship,
               CASE COALESCE(td2.name,'valid') WHEN 'valid' THEN 0 WHEN 'vernacular name' THEN 2 ELSE 1 END AS relation_order
	           FROM 
	             {term_data} td 
	           INNER JOIN 
	             {term_hierarchy} th ON (td.tid=th.tid) 
	           LEFT JOIN 
	             ({term_relation} tr
	               INNER JOIN
	               {term_data} td2 ON (tr.tid2=td2.tid)
	             ) ON (tr.tid1=td.tid AND td2.vid=%d)
	           WHERE
	             th.parent=%d AND td.vid=%d",CLASSIFICATION_RELATIONS,$tid,$vid);        
     }

     $canonicals = variable_get('classification_elements_canonicals',0);

     while ($result = db_fetch_object($query)) {
       $tid = $result->tid;
       $patterns = array('/name/', '/ = homotypic/', '/ = heterotypic/');
       $relation = trim(preg_replace($patterns,'',$result->relationship));

       //convert to canonicals if selected
       if($canonicals == 1 && $relation !== 'vernacular') {
          $name = $NAMES->canonical_form($result->name);
       }
       else {
       	  $name = $result->name;
       }

       $matches[] = array(
	     'name' => $name,
	     'tid' => $tid, 
	     'relation' => $relation,
	     'relation_order' => $result->relation_order
	   );
	
     }

     foreach($matches as $key => $row) {
	   $sort_name[$key] = $row['name'];
	   $sort_order[$key] = $row['relation_order'];
     }

     array_multisort($sort_order, SORT_ASC, $sort_name, SORT_ASC, $matches);

     if($type=='json') {
       echo classification_to_js($matches,$vid);	
     }
     else {
	   echo classification_to_xml($matches,$vid);
     }

    }

    else {
     $result = db_query(db_rewrite_sql("
     SELECT 
       td.tid 
     FROM {term_data} td 
     WHERE 
       td.vid = %d AND LOWER(td.name) LIKE LOWER('%%%s%%')
     ",'t', 'tid'), $vid,$search,0,2);
      while($object = db_fetch_object($result)) {
	    $allparents = taxonomy_get_parents_all($object->tid);
	    foreach($allparents as $parent) {
		  $parents[] = 'n' . $parent->tid;
	    }
      }

      if(!$parents) {
	    $parents[] = 'n0';
      }
      $parents = array_unique($parents);
      sort($parents);

      echo implode(",",$parents);
    }
}

/**
 * Build some json for the tree renderer
 */
function classification_to_js($var,$vid) {

   drupal_set_header('Content-Type: text/plain; charset=utf-8');
   drupal_set_header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0, max-age=0');
   drupal_set_header('Pragma: no-cache');
   drupal_set_header('Expires: Thu, 01 Jan 1970 00:00:00 GMT');

   if(!$var) return;
   $str = '';
   if(count($var) > 1) {
      $str .= "[" . "\n";
   }
   foreach($var as $parent) {
      $str .= "{" . "\n";
      $str .= "\"attributes\" : { \"id\" : \"n" . $parent['tid'] . "\"";
      if($parent['relation']) {
      	$str .= ", \"class\" : \"" . $parent['relation'] . "\"";
      }
      $str .= " }," . "\n";
      $str .= "\"data\" : \"" . $parent['name'] . "\"";

      $children = classification_count_children($vid,$parent['tid']);
      if($children > 0){
         $str .= "," . "\n" . "\"state\" : \"closed\"" . "\n";
      }
      else {
      	 $str .= "\n";
      }
      
      $str .= "}";
      $counter++;
      if($counter !== count($var)){
         $str .= ","  . "\n";
      }
   }
   if(count($var) > 1) {
      $str .= "\n" . "]" . "\n";
   }

   return $str;
}

/**
 * Build some xml for the tree renderer
 */
function classification_to_xml($var,$vid) {

   drupal_set_header('Content-Type: text/xml; charset: utf-8');
   drupal_set_header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0, max-age=0');
   drupal_set_header('Pragma: no-cache');
   drupal_set_header('Expires: Thu, 01 Jan 1970 00:00:00 GMT');

   if(!$var) return '<root></root>';
   $str = '<root>' . "\n";

   foreach($var as $parent) {
      $str .= '<item id="n' . $parent['tid'] . '"';
      $children = classification_count_children($vid,$parent['tid']);
      if($children > 0){
         $str .= ' state="closed" hasChildren="true"';
      }
      if($parent['relation'] || $parent['content']) {
	    $str .= ' class="' . trim($parent['relation'] . ' ' . $parent['content']) . '"';
      }
      $str .= '>' . "\n";
      $str .= '<content>' . "\n";
      $str .= '<name>';      
      $str .= '<![CDATA[' . $parent['name'] . ']]></name>' . "\n";
      $str .= '</content>' . "\n";
      $str .= '</item>' . "\n";
   }

   $str .= '</root>';

   return $str;
}

/**
 * Helper function to determine if term has children
 */
function classification_count_children($vid,$tid) {
   
   global $NAMES_OPTIONS_ELEMENTS;
   
   switch ($NAMES_OPTIONS_ELEMENTS) {
   	 // Show all names
   	 case 0:
	     $result = db_result(db_query("
	       SELECT 
	         COUNT(*) AS count 
	       FROM 
	         {term_data} td 
	       INNER JOIN 
	         {term_hierarchy} th ON (td.tid = th.tid) 
	       WHERE 
	         th.parent = %d AND td.vid = %d", $tid,$vid));
       break;
     // Hide synonyms and vernaculars: nestedset doesn't really help us here
     case 1:
       $result = db_result(db_query("
         SELECT
           COUNT(*) AS count
         FROM
           {term_data} td
         INNER JOIN
           {term_hierarchy} th ON (td.tid=th.tid) 
         LEFT JOIN
           ({term_relation} tr
            INNER JOIN
            {term_data} td2 ON (tr.tid2=td2.tid)
           ) ON (tr.tid1=td.tid AND td2.vid = %d)
         WHERE
           th.parent=%d AND td.vid=%d AND ISNULL(td2.name)",CLASSIFICATION_RELATIONS,$tid,$vid));
       break;
     // Show all names by default
     default:
       $result = db_result(db_query("
         SELECT 
           COUNT(*) AS count 
         FROM 
           {term_data} td 
         INNER JOIN 
           {term_hierarchy} th ON (td.tid=th.tid) 
         WHERE 
           td.vid = %d AND th.parent = %d", $vid,$tid));
   }
   
   return $result;
}

/**
 * Find what nodes in the tree should have checked checkboxes
 */
function classification_js_tree_checked($vid,$nid) {
	drupal_set_header('Content-Type: text/plain; charset=utf-8');
	$matches = array();
	
	$result = db_query("
	SELECT tid 
	FROM
	  {term_node} 
	WHERE
	  nid = %d",$nid);
	
	while ($object = db_fetch_object($result)) {
		$matches[] = $object->tid;
	}

	asort($matches);
	
	echo classification_checked_to_js($vid,$matches);
}

function classification_checked_to_js($vid,$matches) {
	$str = '';
	if(count($matches) > 0) {
      $str .= "[" . "\n";
    }
	foreach ($matches as $tid) {
		$str .= "{\"id\" : \"n" . $tid . "\"}";
		$counter++;
        if($counter !== count($matches)){
          $str .= ","  . "\n";
        }
	}
	if(count($matches) > 0) {
      $str .= "\n" . "]" . "\n";
    }
	return $str;
}