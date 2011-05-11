<?php
// $Id: multisite_manager.admin.inc
/**
 * author: Schuyler Duveen
 * sponsor: Columbia Univeristy Center for New Media Teaching & Learning
 * URL: http://drupal.org/project/multisite_manager
 * License: GPL v2 or any later version
 *
 */
  global $mysql_reserve_words;
  $mysql_reserve_words = array('ACCESSIBLE' , 'ADD' , 'ADMIN', 'ALL' , 'ALTER' , 'ANALYZE' , 'AND' , 'AS' , 'ASC' , 'ASENSITIVE' , 'BEFORE' , 'BETWEEN' ,
							 'BIGINT' , 'BINARY' , 'BLOB' , 'BOTH' , 'BY' , 'CALL' , 'CASCADE' , 'CASE' , 'CHANGE' , 'CHAR' , 'CHARACTER' ,
							 'CHECK' , 'COLLATE' , 'COLUMN' , 'CONDITION' , 'CONSTRAINT' , 'CONTINUE' , 'CONVERT' , 'CREATE' , 'CROSS' ,
							 'CURRENT_DATE' , 'CURRENT_TIME' , 'CURRENT_TIMESTAMP' , 'CURRENT_USER' , 'CURSOR' , 'DATABASE' , 'DATABASES' ,
							 'DAY_HOUR' , 'DAY_MICROSECOND' , 'DAY_MINUTE' , 'DAY_SECOND' , 'DEC' , 'DECIMAL' , 'DECLARE' , 'DEFAULT' ,
							 'DELAYED' , 'DELETE' , 'DESC' , 'DESCRIBE' , 'DETERMINISTIC' , 'DISTINCT' , 'DISTINCTROW' , 'DIV' , 'DOUBLE' ,
							 'DROP' , 'DUAL' , 'EACH' , 'ELSE' , 'ELSEIF' , 'ENCLOSED' , 'ESCAPED' , 'EXISTS,EXIT' , 'EXPLAIN' , 'FALSE' ,
							 'FETCH' , 'FLOAT' , 'FLOAT4' , 'FLOAT8' , 'FOR' , 'FORCE' , 'FOREIGN' , 'FROM' , 'FULLTEXT' , 'GRANT' , 'GROUP' ,
							 'HAVING' , 'HIGH_PRIORITY' , 'HOUR_MICROSECOND' , 'HOUR_MINUTE' , 'HOUR_SECOND' , 'IF' , 'IGNORE' , 'IN' , 'INDEX' ,
							 'INFILE' , 'INNER' , 'INOUT' , 'INSENSITIVE' , 'INSERT' , 'INT' , 'INT1' , 'INT2' , 'INT3' , 'INT4' , 'INT8' ,
							 'INTEGER' , 'INTERVAL' , 'INTO' , 'IS' , 'ITERATE' , 'JOIN' , 'KEY' , 'KEYS' , 'KILL' , 'LEADING' , 'LEAVE' , 'LEFT' ,
							 'LIKE' , 'LIMIT' , 'LINEAR' , 'LINES' , 'LOAD' , 'LOCALTIME' , 'LOCALTIMESTAMP' , 'LOCK' , 'LONG' , 'LONGBLOB' ,
							 'LONGTEXT' , 'LOOP' , 'LOW_PRIORITY' , 'MASTER_SSL_VERIFY_SERVER_CERT' , 'MATCH' , 'MEDIUMBLOB' , 'MEDIUMINT' ,
							 'MEDIUMTEXT' , 'MIDDLEINT' , 'MINUTE_MICROSECOND' , 'MINUTE_SECOND' , 'MOD' , 'MODIFIES' , 'NATURAL' , 'NOT' ,
							 'NO_WRITE_TO_BINLOG' , 'NULL' , 'NUMERIC' , 'ON' , 'OPTIMIZE' , 'OPTION' , 'OPTIONALLY' , 'OR' , 'ORDER' , 'OUT' ,
							 'OUTER' , 'OUTFILE' , 'PRECISION' , 'PRIMARY' , 'PROCEDURE' , 'PURGE' , 'RANGE' , 'READ' , 'READS' , 'READ_ONLY' ,
							 'READ_WRITE' , 'REAL' , 'REFERENCES' , 'REGEXP' , 'RELEASE' , 'RENAME' , 'REPEAT' , 'REPLACE' , 'REQUIRE' , 'RESTRICT' ,
							 'RETURN' , 'REVOKE' , 'RIGHT' , 'RLIKE' , 'SCHEMA' , 'SCHEMAS' , 'SECOND_MICROSECOND' , 'SELECT' , 'SENSITIVE' , 'SEPARATOR' ,
							 'SET' , 'SHOW' , 'SMALLINT' , 'SPATIAL' , 'SPECIFIC' , 'SQL' , 'SQLEXCEPTION' , 'SQLSTATE' , 'SQLWARNING' , 'SQL_BIG_RESULT' ,
							 'SQL_CALC_FOUND_ROWS' , 'SQL_SMALL_RESULT' , 'SSL' , 'STARTING' , 'STRAIGHT_JOIN' , 'TABLE' , 'TERMINATED' , 'THEN' , 'TINYBLOB' ,
							 'TINYINT' , 'TINYTEXT' , 'TO' , 'TRAILING' , 'TRIGGER' , 'TRUE' , 'UNDO' , 'UNION' , 'UNIQUE' , 'UNLOCK' , 'UNSIGNED' , 'UPDATE' ,
							 'USAGE' , 'USE' , 'USING' , 'UTC_DATE' , 'UTC_TIME' , 'UTC_TIMESTAMP' , 'VALUES' , 'VARBINARY' , 'VARCHAR' , 'VARCHARACTER' ,
							 'VARYING' , 'WHEN' , 'WHERE' , 'WHILE' , 'WITH' , 'WRITE' , 'XOR' , 'YEAR_MONTH' , 'ZEROFILL');

//we need a mechanism to deal with expletives, which can be added here
global $prohibited_words;
$prohibited_words = array(
	'admin', 
	'help', 
	'test', 
	'mysql', 
	'lifedesk_production', 
	'information_schema', 
	'edulifedesks', 
	'svn', 
	'testing', 
	'maintenance',
	'fuck',
	'shit',
	'bastard'
);

function multisite_manager_admin_settings_validate($form_id, $form_values) {
  if ($form_values['module'] == 'multisite_manager') {
    if (empty($form_values['multisite_manager_dbprefix_default']) && empty($form_values['multisite_manager_dbpath_default'])) {
      form_set_error('multisite_manager_dbprefix_default', t('You must, at least, enter a database or table prefix string.  Both cannot be empty'));
    }
  }
}

function multisite_manager_admin_settings() {
	
  $defaults = array('db_prefix' => '',
		    'db_path' => '{shortname}',
		    'link' => '{shortname}.{base_url}',
		    'admin_account' => 'copy admin',
		    'profile' => 'default',
		    'install_immediate' => TRUE,
		    'run_cron' => 0,
		    );
  $form['multisite_manager_run_cron'] = array(
    '#type' => 'checkbox',
    '#title' => t('Run drupal sites\' cron jobs'),
    '#required' => FALSE,
    '#default_value' => variable_get('multisite_manager_run_cron', $defaults['run_cron']),
    '#description' => t('This sets whether each drupal site\'s cron jobs will be run when this master site\'s cron is run.  If this is unset no drupal site will have their cron jobs executed regardless of each individual sites\' settings for running cron jobs.')
  );
  $form['multisite_manager_install_immediate'] = array(
    '#type' => 'checkbox',
    '#title' => t('Install site upon node creation'),
    '#required' => FALSE,
    '#default_value' => variable_get('multisite_manager_install_immediate', $defaults['install_immediate']),
    '#description' => t('This sets whether Drupal site\'s creation is done upon node creation or wether it is delayed until the batch create script is run.')
  );
  $form['multisite_manager_dbprefix_default'] = array(
    '#type' => 'textfield',
    '#title' => t('Table prefix default'),
    '#required' => FALSE,
    '#default_value' => variable_get('multisite_manager_dbprefix_default', $defaults['db_prefix']),
    '#description' => t('This is the default prefix that will be forced on those without "advanced database setup" rights. {shortname} stands for the shortname field input when creating the site.  If you make the default database different from the current one, you can reasonably make this empty.')
  );
  $form['multisite_manager_dbpath_default'] = array(
    '#type' => 'textfield',
    '#title' => t('Database name default'),
    '#required' => FALSE,
    '#default_value' => variable_get('multisite_manager_dbpath_default', $defaults['db_path']),
    '#description' => t('If left blank, it will use the current database.  Otherwise, "{shortname}" will be replaced, so another common default might be "{shortname}_drupal". Do NOT set this unless the current database account has CREATE access in MySQL or CREATEDB access in Postgres. To ensure database data is not overwritten, if you install in another database, it is either required that your current database have some db_prefix OR that it will be a new database--i.e. that your entry here includes "{shortname}".'),
  );
  $form['multisite_manager_link_default'] = array(
    '#type' => 'textfield',
    '#title' => t('Link default'),
    '#required' => FALSE,
    '#default_value' => variable_get('multisite_manager_link_default', $defaults['link']),
    '#description' => t('This is where the site will be accessible by default.  If you setup your ./sites/default/settings.php correctly along with your web server (e.g. apache/htaccess) config, you can anticipate where the new site will live and forward the user to the new site location upon creation.  Here, there are two dynamic variables, {base_url} and {shortname}.')
  );
  $form['multisite_manager_admin_account'] = array(
    '#type' => 'radios',
    '#title' => t('Admin account on new sites'),
    '#required' => TRUE,
    '#options' => array(
			'copy admin' => t('Copy the admin account from this site'),
			'copy creator' => t('Copy the account of the creator'),
			'do nothing' => t('Do nothing (only choose this if all profiles available do something instead.  the default profile does not)'),
			/*'prompt creator' => '',*/
			),
    '#default_value' => variable_get('multisite_manager_admin_account', $defaults['admin_account']),
  );
  $form['multisite_manager_profile_default'] = array(
    '#type' => 'radios',
    '#title' => t('Profile default'),
    '#required' => FALSE,
    '#options' => _multisite_manager_profile_options(),
    '#default_value' => variable_get('multisite_manager_profile_default', $defaults['profile']),
  );

  $form['multisite_manager_excluded_modules'] = array(
    '#type' => 'textarea',
    '#title' => t('Modules to exclude'),
    '#description' => t('List modules here that should not be installed. This is handy to prevent certain modules from being installed while testing (e.g. apachesolr). Note however that modules that do get installed cannot declare excluded modules as dependents.'),
    '#default_value' => variable_get('multisite_manager_excluded_modules', ''),
  );

  

  return system_settings_form($form);
}

/*
 * multisite_manager_admin_file_settingsdotphp
 * @return string of PHP code that would be included based on the default settings for link/database name
 */
function multisite_manager_admin_file_settingsdotphp() {
  global $base_url;
  $link = variable_get('multisite_manager_link_default', '');
  ///
  $fakenode = new stdClass();
  $fakenode->shortname = '". $my_site_base ."';
  $fakenode->db_prefix = variable_get('multisite_manager_dbprefix_default', '');
  $fakenode->db_user = '';
  $fakenode->db_pass = '<b style="color:red">DATABASE_PASSWORD</b>';
  $fakenode->db_path = variable_get('multisite_manager_dbpath_default', '');
  $newdb = _multisite_manager_dbobj($fakenode);

  $recommended_directory = '';

  if (!$link || !$fakenode->db_path) {
    return "ERROR: no link or db url";
  }
  $link = str_replace('{base_url}', $base_url, $link);

  //$url_pieces[2] is hostname, $url_pieces[3] is path
  $url_pieces = explode('/', $link, 4);

  $rv = array('<p>Based on your configuration in '.l( 'Multisite Manager Defaults','admin/settings/multisite_manager/defaults').' The following code should be added to a special settings.php file which .  Note, <b>This is an experimental feature</b> and you should look over this carefully before adding it in place of the database configuration (Also, please '.l('report suggestions/issues','http://drupal.org/project/multisite_manager') .').</p><blockquote><pre><code>',
	      '$matches = FALSE;');

  $host_regex = str_replace('{shortname}', '(\w+)', $url_pieces[2]);
  $path_regex = str_replace('{shortname}', '(\w+)', $url_pieces[3]);

  //assumes shortname will never be the TLD
  $recommended_directory = preg_replace('|\(\\w\+\)[^.]*\.|','',$host_regex);
  ///shortname in hostname
  if (strpos($url_pieces[2],'{shortname}') !== FALSE) {
    $rv[] = 'if (preg_match("|'. $host_regex .'|",$_SERVER["HTTP_HOST"], $matches)';
    $rv[] = '    && preg_match("|'. $path_regex .'|",request_uri())';
    $rv[] = '    && $matches)';
    $rv[] = '{';
  }///shortname in path
  elseif (strpos($url_pieces[3],'{shortname}') !== FALSE) {
    $rv[] = 'if (preg_match("|'. $host_regex .'|",$_SERVER["HTTP_HOST"])';
    $rv[] = '    && preg_match("|'. $path_regex .'|",request_uri(), $matches)';
    $rv[] = '    && $matches)';
    $rv[] = '{';
    $recommended_directory .= '.' . str_replace('/','.',preg_replace('|(/[^/]*)?\(\\\w\+\)([^/]*)|','',$path_regex));
  }
  else {
    ///{shortname} has to appear in the link somewhere!
    return "ERROR: shortname must be in the link template " .$url_pieces[2] .'XX' . $url_pieces[3];
  }
  $rv[] = '  $my_site_base = $matches[1];';
  $rv[] = '  if(($my_site_base != \'q\'))';
  $rv[] = '  {';
  ///prefix
  $rv[] = '    $db_prefix = "' . $newdb['prefix'] .'";';
  ///needs URL structure
  $rv[] = '    $base_url = "'. str_replace('{shortname}','". $my_site_base ."',$link) .'";  // NO trailing slash!';

  ///needs newDB URI structure
  $rv[] = '    $db_url = "' .$newdb['url'] .'";';

  $rv[] = '    ///This assumes you have a ./files directory in your base drupal directory';
  $rv[] = '    $conf = array(';
  $rv[] = '	     "file_directory_path" => "files/".$my_site_base,';
  $rv[] = '    );';
  $rv[] = '  }';
  $rv[] = '}';

  $rv[] = "</code></pre></blockquote>";
  $rv[] = 'Suggested directory under sites/ to include settings.php file: <b>' . $recommended_directory . "</b></p>";
  return implode("\n",$rv);
}


// vim:fenc=utf-8:ft=php:ai:si:ts=2:sw=2:et:

/**
 * Callback to check if term actually exists
 */
 function multisite_manager_check_shortname($shortname=null){
  global $mysql_reserve_words, $prohibited_words, $conf;
  $output = '';
  $status = TRUE;
  $link = '';
  $shortname = ($shortname) ? $shortname : $_REQUEST['shortname'];

  $result = db_result(db_query("SELECT nid FROM drupal_site WHERE shortname = '%s'", $shortname));
  if($result) {
    $output = '<div class="error">' . t('The shortname you have entered, '. $shortname .', is already taken. Please use a different one.') . '</div>';
    $status = FALSE;
    $link = 'http://' . $shortname . '.' . $conf['_lifedesks_domain'];
  }
  $already_exists = db_result(@db_query("SHOW DATABASES LIKE '%s'", $shortname));
  if($already_exists) {
    $output = '<div class="error">' . t('The shortname you have entered, '. $shortname .', is already taken. Please use a different one.') . '</div>';
    $status = FALSE;
  }

  foreach($mysql_reserve_words as $reserve_word) {
    if(strtoupper($shortname) == $reserve_word) {
      $output = '<div class="error">' . t('Please use a different shortname than ') . '"' . $shortname . '"</div>';
      $status = FALSE;
    }
  }

  foreach($prohibited_words as $prohibited_word) {
    if($shortname == $prohibited_word) {
      $output = '<div class="error">' . t('Please use a different shortname than ') . '"' . $shortname . '"</div>';
      $status = FALSE;
    }
  }

  if(preg_match("/^.*[^a-z].*$/",$shortname)) {
    $status = FALSE;
  }

  if($_REQUEST['shortname']) {
    drupal_json(array('status' => $status,'data' => $output));
  }
  else {
	drupal_json(array('status' => $status, 'link' => $link));
  }
}