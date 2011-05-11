<?php

/**
 * Form to view the classification editing logs
 */
function classification_logs_form(&$form_state) {
 
  $session = $_SESSION['classification_logs_filter'];

  $header = array(
    array('data' => t('Activity'), 'field' => 'cl.description'),
    array('data' => t('Editor'), 'field' => 'u.surname'),
    array('data' => t('Time'), 'field' => 'cl.created', 'sort' => 'desc'),
  );

  if(!$session) {
    $where[] = "cl.clid > 1";
  }

  foreach ($session as $index => $filter) {
    list($key, $value) = $filter;
    switch ($key) {
      case 'editor':
        $where['editor'] = "cl.uid = ".$value; 
        break;
      case 'action':
	      $where['action'] = "cl.type = '".$value."'";
        break;
    }
  }

  $sql = "
    SELECT
      cl.clid,
      cl.description,
      cl.created,
      cl.uid,
      u.name,
      u.surname
    FROM 
      {classification_log} cl 
    INNER JOIN 
      {users} u ON (cl.uid = u.uid)
    WHERE ".implode(' AND ', $where);

  $sql .= tablesort_sql($header);

  $query_count = "
    SELECT 
      COUNT(cl.clid) 
    FROM 
      {classification_log} cl 
    INNER JOIN 
      {users} u ON (cl.uid = u.uid)
    WHERE " . implode(' AND ', $where);

  $result = pager_query($sql, ($_SESSION['classification_logs_max_rows']) ? $_SESSION['classification_logs_max_rows'] : 50, 0, $query_count);

  while ($log = db_fetch_object($result)) {
	$form['action'][$log->clid] = array('#value' => $log->description);
    $form['editor'][$log->clid] = array('#value' => theme('username', $log));
    $form['modified'][$log->clid] = array('#value' => $log->created ? t('@time ago', array('@time' => format_interval(time() - $log->created))) : t('never'));
    $form['pager'] = array('#value' => theme('pager', NULL, 50, 0));
  }

  return $form;
}

function theme_classification_logs_form($form) {
  $feed_title = variable_get('site_name', 'LifeDesk') . ' Classification Activity';
  drupal_add_feed(url('classification/logs/rss'), $feed_title);

  $module_path = drupal_get_path('module', 'classification') .'/';
  drupal_add_css($module_path .'css/classification_logs.css');

  $header = array(
    array('data' => t('Activity'), 'field' => 'cl.description', 'width' => '450px' ),
    array('data' => t('Editor'), 'field' => 'u.surname'),
    array('data' => t('Time'), 'field' => 'cl.created', 'sort' => 'desc'),
  );

  if (isset($form['editor']) && is_array($form['editor'])) {
    foreach (element_children($form['editor']) as $key) {
      $rows[] = array(
        drupal_render($form['action'][$key]),
        drupal_render($form['editor'][$key]),
        drupal_render($form['modified'][$key]),
      );
    }
  }
  else {
    $rows[] = array(array('data' => t('No logs have been recorded.'), 'colspan' => '3'));
  }

  $output .= drupal_render($form['classification_logs']);

  $output .= _classification_logs_filter_info_line();

  $output .= theme('table', $header, $rows);

  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $output .= drupal_render($form);

  return $output;
}

function classification_logs_filter_form(&$form_state) {
	
	$session = $_SESSION['classification_logs_filter'];
	$session = is_array($session) ? $session : array();
	$filters = _get_classification_logs_filters();
	
	// general settings display (max rows & content kind)
	$form['general'] = array('#type' => 'fieldset',
	  '#title' => t('General Settings'),
	  '#collapsible' => TRUE,
	  '#collapsed' => TRUE,
	); 
	$form['general']['max-rows'] = array(
	  '#type' => 'textfield', 
	  '#title' => t('Max rows'),
	  '#size' => 5,
	  '#default_value' => isset($_SESSION['classification_logs_max_rows']) ? $_SESSION['classification_logs_max_rows'] : 50,
	  '#prefix' => '<div class="container-inline">',
	  '#suffix' => '</div>',
	);
	$form['general']['buttons']['apply'] = array(
	  '#type' => 'submit', 
	  '#value' => t('Apply'),
	);
	
	$i = 0;
    $form['filters'] = array('#type' => 'fieldset',
      '#title' => t('Show only items where'),
      '#theme' => 'node_filters',
      '#prefix' => '<div id="node-admin-filter">',
      '#suffix' => '</div>',
    );
    foreach ($session as $filter) {
      list($type, $value) = $filter;
      $value = $filters[$type]['options'][$value];
      // avoid not applicable filter verbose
      if ($true && !($type == 'editor')) {
        $string = ($i++ ? '<em>and</em> where <strong>%a</strong> is <strong>%b</strong>' : 
        '<strong>%a</strong> is <strong>%b</strong>');
        $form['filters']['current'][] = array('#value' => t($string, 
        array('%a' => $filters[$type]['title'] , '%b' => $value)));
      }
      else {
        $string = ($i++ ? '<em>and</em> where <strong>%a</strong> is <strong>%b</strong>' : 
        '<strong>%a</strong> is <strong>%b</strong>');
        $form['filters']['current'][] = array('#value' => t($string, 
        array('%a' => $filters[$type]['title'], '%b' => $value)));    
      }
      // those ifs remove the respective option if it is already being filtered on.
      if ($type == 'editor') {
        unset($filters['editor']);
      }
    }

    // preparing select boxes
    foreach ($filters as $key => $filter) {
      $names[$key] = $filter['title'];

      switch ($key) {
        case 'editor':
          $form['filters']['status'][$key] = array(
            '#type'     => 'select', 
            '#options'  => $filter['options'], 
          );
          break;

        default: 
          $form['filters']['status'][$key] = array(
            '#type'    => 'select', 
            '#options' => $filter['options'],
          );
      }
    }
    // building radio buttons
    $keyz = array_keys($names);
    $form['filters']['filter'] = array(
      '#type'          => 'radios',
      '#options'       => $names,
      '#default_value' => $keyz[0],
    );
    // building buttons depending on the filter state
    $form['filters']['buttons']['submit'] = array(
      '#type'  => 'submit', 
      '#value' => (count($session) ? t('Refine') : t('Filter')),
    );
    if (count($session)) {
      $form['filters']['buttons']['undo'] = array(
        '#type'  => 'submit', 
        '#value' => t('Undo'),
      );
      $form['filters']['buttons']['reset'] = array(
        '#type'  => 'submit', 
        '#value' => t('Reset'),
      );
    }
	
	return $form;
}

function classification_logs_filter_form_submit($form, &$form_state) {
	$filters = _get_classification_logs_filters();
	
	switch ($form_state['values']['op']) {
    case t('Filter'):
    case t('Refine'):
      if (isset($form_state['values']['filter'])) {
        $filter = $form_state['values']['filter'];
        $flat_options = form_options_flatten($filters[$filter]['options']);
        if (isset($flat_options[$form_state['values'][$filter]])) {
	      if($filter == 'action') {
		    $_SESSION['classification_logs_filter']['action'] = array($filter, $form_state['values'][$filter]);
	      }
	      else {
		    $_SESSION['classification_logs_filter'][] = array($filter, $form_state['values'][$filter]);
	      }
        }
      }
      break;
    case t('Undo'):
      array_pop($_SESSION['classification_logs_filter']);
      break;
    case t('Reset'):
      $_SESSION['classification_logs_filter'] = array();
      break;
    case t('Apply'):
      $_SESSION['classification_logs_max_rows'] = $form_state['values']['max-rows'];
      break;
  }

  cache_clear_all();
  $form_state['redirect'] = 'admin/classification/biological/logs';
}

function _get_classification_logs_filters() {

    $actions = array('moved' => t('moved'), 'added' => t('added'), 'updated' => t('updated'), 'deleted' => t('deleted'), 'copied' => t('copied'));

	$filters['editor'] = array('title' => t('editor'), 'options' => _get_classification_logs_users());
	$filters['action'] = array('title' => t('action'), 'options' => $actions);

	return $filters;
}

function classification_logs_settings_form(&$form_state) {
    $form['classification_logs_settings'] = array(
	  '#type' => 'fieldset',
	  '#title' => t('General Settings'),
    );

    $form['classification_logs_settings']['logging'] = array(
	 '#type' => 'radios',
	 '#title' => t('Clear activity log entries greater than'),
	 '#options' => array(2 => t('2 weeks old'), 4 => t('4 weeks old'), 6 => t('6 weeks old')),
	 '#default_value' => variable_get('classification_logs_delete_interval',4),
    );

    $form['classification_logs_settings']['submit'] = array(
  	 '#type' => 'submit',
  	 '#value' => t('Save Configuration'),
    );
  
    return $form;
}

function classification_logs_settings_form_submit($form, &$form_state) {
  variable_set('classification_logs_delete_interval',$form_state['values']['logging']);
  drupal_set_message(t('Configuration settings have been saved'));
}

function _get_classification_logs_users() {

  $ld_support = variable_get('lifedesk_support', 0);
  $qry = db_query('
        SELECT uid, surname, givenname
        FROM {users}
        WHERE uid > 1 AND uid != '.$ld_support.' AND status = 1
        ORDER BY surname'
  );
  
  $users = array();

  while ($user = db_fetch_object($qry)) {
    $users[$user->uid] = $user->surname . ', ' . $user->givenname;
  }
  
  return $users;
}

function _classification_logs_filter_info_line() {
  $content = '';
  $session = ($_SESSION['classification_logs_filter']) ? $_SESSION['classification_logs_filter'] : array();
  $filters = _get_classification_logs_filters();

  if($session) {
	  foreach ($session as $filter) {
        list($type, $value) = $filter;
	    $value = $filters[$type]['options'][$value];
	    if ($type == 'editor') {
	      $string = ($i++ ? '<em>and</em> where <strong>%a</strong> is <strong>%b</strong> ' : 
	        '<strong>%a</strong> is <strong>%b</strong> ');
	      $filtercontent .= t($string, array('%a' => $filters[$type]['title'] , '%b' => $value));
	    }
	    else {
	      $string = ($i++ ? '<em>and</em> where <strong>%a</strong> is <strong>%b</strong> ' : 
	        '<strong>%a</strong> is <strong>%b</strong> ');
	      $filtercontent .= t($string, array('%a' => $filters[$type]['title'], '%b' => $value));    
	    }
	  }
      $content .= '<div class="classification-logs-current-filters"><b>'.t('Filters').': </b>';
	  $content .= $filtercontent;

	  $content .= '&nbsp;&nbsp;'.l('['.t('Clear All Filters').']', "admin/classification/biological/logs/filter/clear");
	  $content .= '</div>';	
  }

  return $content;
}

function classification_logs_filter_clear() {
  $_SESSION['classification_logs_filter'] = array();
  cache_clear_all();
  drupal_goto('admin/classification/biological/logs');
}

/**
 * RSS feed for classification logs
 */
function classification_logs_rss() {

  $output = '<?xml version="1.0" encoding="utf-8" standalone="yes"?>
    <rss version="2.0" xmlns:atom="http://www.w3.org/2005/Atom">
      <channel>
      <atom:link href="' . url('classification/logs/rss',array('absolute' => true)) . '" rel="self" type="application/rss+xml"></atom:link>
      <title>' . variable_get('site_name', 'LifeDesk') . ' ' . t('Classification Activity') . '</title>
      <link>' . url('classification/logs', array('absolute' => TRUE)) . '</link>
      <description>' . t('Activity of names management in !lifedesk', array('!lifedesk' => htmlspecialchars(variable_get('site_name', 'LifeDesk')))) . '</description>';

    $query = "SELECT
      cl.clid,
      cl.description,
      cl.created,
      cl.uid,
      u.name
    FROM 
      {classification_log} cl 
    INNER JOIN 
      {users} u ON (cl.uid = u.uid) 
    ORDER BY cl.created DESC";
    
    $query_pager = "SELECT 
      COUNT(cl.clid) 
    FROM 
      {classification_log} cl";
  
    $result = pager_query($query,6, 0, $query_pager);

    while ($row = db_fetch_object($result)) {
	//Tue, 03 Jun 2003 09:39:21 GMT
	  $item = array(
		'guid' => $row->clid,
		'title' => $row->description,
		'pubDate' => date("r",$row->created),
		'link' => url('classification/logs', array('absolute' => TRUE)),
		'description' => t('!message executed by !user on !date', array('!message' => $row->description, '!user' => theme('username', $row), '!date' => date("r", $row->created))),
	  );
	  $output .= classification_logs_rss_item($item);
    }

    $output .=  '
    </channel>
    </rss>';

  drupal_set_header('Content-Type: application/rss+xml; charset=utf-8');
  echo $output;
}

function classification_logs_rss_item($item) {
  $rssitem = '
    <item>
      <title>'. check_plain($item['title']) .'</title>
      <link>'. $item['link']  .'</link>
      <description><![CDATA[' . $item['description'] . ']]></description>
      <pubDate>' . $item['pubDate'] . '</pubDate>
      <guid isPermaLink="false">' . variable_get('shortname','LifeDesk') . '-classification-log-' . $item['guid'] . '</guid>
    </item>';
  return $rssitem;
}