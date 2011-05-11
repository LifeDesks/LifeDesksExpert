<?php
/**
 * Ping function to return message to users when someone logs in
*/
function classification_pinger() {
	global $user;

    $timestamp = $user->timestamp;
	
	//let's first see who might have just logged in
	$pinger_freq = variable_get('classification_pinger_freq', 5);
/*
	//We could see who just logged in (commented out for now to reduce number of queries)
	$ld_support = variable_get("lifedesk_support", 0);
	$roles = user_roles(true, 'edit classification');
	
	$qry = db_query("
		SELECT 
		  DISTINCT u.uid, u.givenname, u.surname, u.login
		FROM 
		  {users} u 
		INNER JOIN
		  {users_roles} ur ON (u.uid = ur.uid) 
		WHERE 
		  ur.rid IN (" . implode(',', array_keys($roles)) . ") AND u.uid > 1 AND u.uid != %d AND u.uid != %d AND %d - u.login BETWEEN 0 AND %d", $ld_support, $user->uid, $timestamp, $pinger_freq);
	
	$editor = array();
	
	while($result = db_fetch_object($qry)) {
		$editor[$result->uid] = $result->givenname . " " . $result->surname;
	}
	
	$status = '';
	$message = array();
	
	if($editor) {
		$status = 'status';
		$message = t('!editor just logged in', array('!editor' => implode(", ", $editor)));
	}
*/
	//let's see if there are any recent edits and order them in the sequence they were created
	$qry = db_query("
		SELECT 
		  u.givenname, u.surname, cl.description, cl.action, cl.created
		FROM 
		  {classification_log} cl
		INNER JOIN
		  {users} u ON (cl.uid = u.uid)
		WHERE
		  u.uid != %d  AND cl.created - %d BETWEEN 0 AND %d
		ORDER BY cl.created ASC", $user->uid, $timestamp, $pinger_freq);
	
	$action = array();
	$description = array();
	
	while($result = db_fetch_object($qry)) {
		if($result->action) {
			$_action = unserialize($result->action);
			if(is_array($_action[0])){
				foreach($_action as $value) {
					$action[] = $value;
				}
			}
			else {
				$action[] = $_action;
			}
		}
		$description[] = $result->description . ' <strong>Editor:</strong> ' . $result->givenname . " " . $result->surname . ' <strong>Time:</strong> ' . date("h:i:s", $result->created);
	}
	
	if($action) {
		$vid = array();
		foreach($action as $key => $row) {
			$vid[$key] = $row['vid'];
		}
		//we have to sort the array such that the primary classification is first because the js pinger code needs to see it first (NOTE: not very smart if its vid is greater than some other vocab's vid)
		array_multisort($vid, SORT_ASC, $action);
		$status = 'warning';
		$message = implode("<br>", $description);
	}
	
	drupal_json(array('status' => $status, 'message' => $message, 'actions' => $action));
}