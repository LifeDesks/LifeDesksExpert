<?php
// $Id$
/**
 * @file data_monitoring.theme.inc
 * Includes themes for data monitoring forms and email reports.
 */

function theme_data_monitoring_monitors($form) {

  $module_path = drupal_get_path('module', 'data_monitoring');
  drupal_add_css($module_path . '/themes/css/data_monitoring.css');
  drupal_add_js($module_path . '/themes/js/data_monitoring.js');
  $image_path = "/" . drupal_get_path('module', 'data_monitoring') . "/themes/images";
  $output = '';

  $last_monitoring = drupal_render($form['last_monitoring']);
  if ($last_monitoring > 0) $output .= t('Monitoring last triggered on !date', array('!date' => format_date($last_monitoring, 'custom', 'r')));

  $filter = '<div id="data-monitoring-monitors-filters">'
          . drupal_render($form['filters']['latest'])
          . drupal_render($form['filters']['all'])
          . '</div>';
  $rows = array();
  if (isset($form['name']) && is_array($form['name'])) {
    foreach (element_children($form['name']) as $key) {
      $dmid = drupal_render($form['dmid'][$key]);
      $r = array();
      $r[] = drupal_render($form['logs'][$key]);
      $murl = url('admin/reports/data-monitoring/monitor/' . $dmid, array('query' => str_replace('destination=', 'referrer=', drupal_get_destination())));
      $mname = '<a href="' .$murl . '">' . drupal_render($form['name'][$key]) . '</a>';
      unset($murl);
      $r[] = array(
        'data' => '<span class="monitor-name">' . $mname . '</span>'
                . '<span class="monitor-description">' . drupal_render($form['description'][$key]) . '</span>',
        'class' => 'monitor'
      );

      $processed = drupal_render($form['processed'][$key]);
      $r[] = array(
        'data' => ($processed > 0) ? date('c', $processed) : t('Date missing'),
        'class' => 'processed',
      );

      $r[] = array(
        'data' => drupal_render($form['result_count'][$key]),
        'class' => 'result-count',
      );

      $filepath = drupal_render($form['log_file'][$key]);
      $r[] = array(
        'data' => ($filepath) ? l("<img src=\"$image_path/table-sheet.png\" alt=\"" . t('Log file.') . '" />', $filepath, array('html' => TRUE)) : t('Log file missing'),
        'class' => 'log-file',
      );
      unset($filepath);

      $r[] = array(
        'data' => ($processed < $form['latest_processed'][$dmid]['#value']) ?
                   t('N/A') : drupal_render($form['responses_pending'][$key]),
        'class' => 'responses-pending'
      );
      $r[] = array(
        'data' => drupal_render($form['responses_implemented'][$key]),
        'class' => 'responses-implemented'
      );
      $rows[] = $r;
      unset($dmid);
      unset($processed);
    }
  }
  else {
    $rows[] = array(array('data' => t('No logs found.'), 'colspan' => '7'));
  }

  $header = $form['headers']['#value'];
  $limit = $form['limit']['#value'];
  unset($form['headers']);
  unset($form['limit']);
  unset($form['filters']);
  unset($form['latest_processed']);
  unset($form['responses_pending']);
  $output .= drupal_render($form);
  $output .= $filter;
  $output .= theme('pager', NULL, $limit, 0);
  $output .= theme('table', $header, $rows, array('id' => 'data-monitoring-monitors', 'class' => 'data-monitoring'));

  return $output;
}
/**
 * Theme for data_monitoring_form_monitor().
 * @param $form
 *    Array Drupal form.
 */
function theme_data_monitoring_monitor($form) {

  $module_path = drupal_get_path('module', 'data_monitoring');
  drupal_add_css($module_path . '/themes/css/data_monitoring.css');
  drupal_add_js($module_path . '/themes/js/data_monitoring.js');
  drupal_add_js('misc/collapse.js');
  $image_path = "/" . drupal_get_path('module', 'data_monitoring') . "/themes/images";
  $output = '';

  $output .= l(t('Return to monitoring logs'), drupal_render($form['referrer']),
               array('attributes' => array('class' => 'data-monitoring')));

  $output .= drupal_render($form['monitor']['name'])
           . drupal_render($form['monitor']['description'])
           . theme('table',
                   array(),
                   array(array(t('Registered by module'), drupal_render($form['monitor']['module'])),
                         array(t('Current result count'), drupal_render($form['monitor']['result_count'])),
                         array(t('Processed'), date('r'))),
                   array('id' => 'data-monitoring-monitor-result', 'class' => 'data-monitoring')
                   );

  // Pending responses.
  if ($form['observers']['#access']) {

    if (isset($form['observers']['description']) && is_array($form['observers']['description'])) {
      $observers = array();
      foreach (element_children($form['observers']['description']) as $key) {
        $rid = drupal_render($form['observers']['id'][$key]);
        $row = array();
        $row[] = drupal_render($form['observers']['responses'][$key]);
        $row[] = array('data' => drupal_render($form['observers']['description'][$key]));
        $row[] = array('data' => _data_monitoring_readable_state(drupal_render($form['observers']['response_state'][$key])), 'class' => 'response-state');
        $observers[] = array(
          'data' => $row,
          'data-rid' => $rid,
        );

      }
      $output .= '<fieldset class="collapsible">'
               . '<legend>' . $form['observers']['#title'] . '</legend>'
               . '<div class="fieldset-wrapper">'
               . '<p>' . $form['observers']['#description'] . '</p>'
               . theme('table', $form['observers']['header']['#value'], $observers, array('id' => 'data-monitoring-responses', 'class' => 'data-monitoring'))
               . drupal_render($form['observers']['preview_response'])
               . '</div></fieldset>';
      unset($form['observers']);
    }
    else {
      $output .= drupal_render($form['observers']);
    }
  }

  // Logs.
  $logs = array();
  if (isset($form['logs']['processed']) && is_array($form['logs']['processed'])) {
    foreach (element_children($form['logs']['processed']) as $dmlid) {
      $monitor_log_row = array();
      $processed = drupal_render($form['logs']['processed'][$dmlid]);
      $monitor_log_row[] = array(
        'data' => ($processed > 0) ? date('c', $processed) : t('Date missing'),
        'class' => 'processed',
      );
      unset($processed);

      $monitor_log_row[] = array(
        'data' => drupal_render($form['logs']['result_count'][$dmlid]),
        'class' => 'result-count',
      );

      $filepath = drupal_render($form['logs']['log_file'][$dmlid]);
      $monitor_log_row[] = array(
        'data' => ($filepath) ? l("<img src=\"$image_path/table-sheet.png\" alt=\"" . t('Log file.') . '" />', $filepath, array('html' => TRUE)) : t('Log file missing'),
        'class' => 'log-file',
      );
      unset($filepath);

      $monitor_log_row[] = array(
        'data' => drupal_render($form['logs']['responses_pending'][$dmlid]),
        'class' => 'responses-pending'
      );
      $monitor_log_row[] = array(
        'data' => drupal_render($form['logs']['responses_implemented'][$dmlid]),
        'class' => 'responses-implemented'
      );
      if (isset($form['logs']['responses'][$dmlid])) {
        $responses = array();
        foreach (element_children($form['logs']['responses'][$dmlid]['r_processed']) as $dmrlid) {
          $response_row = array();
          $processed = drupal_render($form['logs']['responses'][$dmlid]['r_processed'][$dmrlid]);
          $response_row[] = array(
            'data' => ($processed > 0) ? '<a href="' . url('admin/reports/data-monitoring/response/' . $dmrlid) . '">' . date('c', $processed) . '</a>' : t('Date missing'),
            'class' => 'response-processed',
          );
          unset($processed);
          $response_row[] = array(
            'data' => drupal_render($form['logs']['responses'][$dmlid]['created_result_count'][$dmrlid]),
            'class' => 'created-count'
          );
          $response_row[] = array(
            'data' => drupal_render($form['logs']['responses'][$dmlid]['updated_result_count'][$dmrlid]),
            'class' => 'updated-count'
          );
          $response_row[] = array(
            'data' => drupal_render($form['logs']['responses'][$dmlid]['deleted_result_count'][$dmrlid]),
            'class' => 'deleted-count'
          );
          $response_row[] = array(
            'data' => drupal_render($form['logs']['responses'][$dmlid]['remaining_result_count'][$dmrlid]),
            'class' => 'remaining-count'
          );
          $filepath = drupal_render($form['logs']['responses'][$dmlid]['r_log_file'][$dmrlid]);
          $response_row[] = array(
            'data' => ($filepath) ? l("<img src=\"$image_path/table-sheet.png\" alt=\"" . t('Log file.') . '" />', $filepath, array('html' => TRUE)) : t('Log file missing'),
            'class' => 'log-file',
          );
          unset($filepath);
          $response_row[] = array(
            'data' => _data_monitoring_readable_state(drupal_render($form['logs']['responses'][$dmlid]['response_state'][$dmrlid])),
            'class' => 'response-state'
          );
          $responses[] = array('data' => $response_row, 'data_dmrlid' => $dmrlid);
        }
      }
      $logs[] = array(
        'data' => $monitor_log_row,
        'class' =>  (!empty($responses)) ? 'monitor-log has-responses' : 'monitor-log',
        'data_dmlid' => $dmlid,
      );
      if (!empty($responses)) {
        $logs[] = array(
          'data' => array(array(
            'data' => theme('table', $form['responses_header']['#value'], $responses, array('id' => 'data-monitoring-response-logs', 'class' => 'data-monitoring')),
            'colspan' => 5,
            'class' => 'response-logs'
          )),
          'data_dmlid' => $dmlid,
          'class' => 'response-logs'
        );
      }
      unset($responses);
    }
  }
  else {
    $logs[] = array(array('data' => t('No logs found.'), 'colspan' => '5'));
  }
  $pager = (isset($_GET['page'])) ? explode(',', $_GET['page']) : array();
  $collapsed[0] = (isset($pager[0]) && $pager[0] == 0) ? '' : ' collapsed';
  $output .= '<fieldset class="collapsible' . $collapsed[0] . '">'
           . '<legend>' . drupal_render($form['logs_title']) . '</legend>'
           . '<div class="fieldset-wrapper">'
           . drupal_render($form['logs_description'])
           . theme('table', $form['logs_header']['#value'], $logs, array('id' => 'data-monitoring-monitor-logs', 'class' => 'data-monitoring'))
           . theme('pager', NULL, $form['logs_limit']['#value'], 1)
           . '</div></fieldset>';

  unset($form['observers_header']);
  unset($form['responses_header']);
  unset($form['logs_header']);
  unset($form['logs_limit']);
  $output .= drupal_render($form);
  return $output;

}
/**
 * Theme for data_monitoring_form_response().
 * @param $form
 *    Array Drupal form.
 */
function theme_data_monitoring_response($form) {

  $module_path = drupal_get_path('module', 'data_monitoring');
  drupal_add_css($module_path . '/themes/css/data_monitoring.css');
  drupal_add_js($module_path . '/themes/js/data_monitoring.js');
  $image_path = "/" . drupal_get_path('module', 'data_monitoring') . "/themes/images";
  $output = '';

  drupal_set_title(t('Response to monitor:') . ' ' . drupal_render($form['m_name']));
  $output .= drupal_render($form['description']);

  $response_state = drupal_render($form['response_state']);
  $output .= '<h2>'
           . t('Response !response_state', array('!response_state' => drupal_strtolower(_data_monitoring_readable_state($response_state))))
           . '</h2>';
  $rows = array();
  if ($response_state == DATA_MONITORING_STATE_PREVIEW) {
    $output .= '<p>' . t('This is a preview, results shown are the expected actions that will '
                       . 'be taken once you implement the response.') .'</p>';
    $rows[] = array(array('data' => t('Module')), array('data' => drupal_render($form['module'])));
    $rows[] = array(array('data' => t('Processed')), array('data' => date('r', drupal_render($form['processed']))));
    $rows[] = array(array('data' => t('Count of primary records that will be created')), array('data' => drupal_render($form['created_result_count'])));
    $rows[] = array(array('data' => t('Count of primary records that will be updated')), array('data' => drupal_render($form['updated_result_count'])));
    $rows[] = array(array('data' => t('Count of primary records that will be deleted')), array('data' => drupal_render($form['deleted_result_count'])));
    $rows[] = array(array('data' => t('Count of primary records that will be remaining')), array('data' => drupal_render($form['remaining_result_count'])));
  }
  elseif ($response_state == DATA_MONITORING_STATE_COMPLETED) {
    $output .= '<p>' . t('The response has been implemented. The following results outline the actions '
                       . 'that were taken on the data records directly affected. Related data '
                       . 'may also have been modified.') .'</p>';
    $rows[] = array(array('data' => t('Module')), array('data' => drupal_render($form['module'])));
    $rows[] = array(array('data' => t('Processed')), array('data' => date('r', drupal_render($form['processed']))));
    $rows[] = array(array('data' => t('Count of primary records that were created')), array('data' => drupal_render($form['created_result_count'])));
    $rows[] = array(array('data' => t('Count of primary records that were updated')), array('data' => drupal_render($form['updated_result_count'])));
    $rows[] = array(array('data' => t('Count of primary records that were deleted')), array('data' => drupal_render($form['deleted_result_count'])));
    $rows[] = array(array('data' => t('Count of primary records that were remaining')), array('data' => drupal_render($form['remaining_result_count'])));
  }
  $filepath = drupal_render($form['log_file']);
  if ($filepath) {
    $rows[] = array(array('data' => t('Log file')),
                    array('data' => l("<img class='log' src=\"$image_path/table-sheet.png\" alt=\""
                                    . t('Downloadable file containing response results as a tab delimited '
                                    . 'text file suitable for importing into a spreadsheet.') . '" />'
                                    , $filepath, array('html' => TRUE))));
  }
  unset($filepath);
  $output .= theme('table', array(), $rows, array('id' => 'data-monitoring-response-result', 'class' => 'data-monitoring'));

  unset($form['destination']);
  unset($form['trigger_mode']);
  $output .= drupal_render($form);
  return $output;
}
/**
 * Plain text summary of monitor results.
 * @param $monitor
 */
function theme_data_monitoring_monitor_summary_plain($monitor, $language = NULL) {
  $rows = array();
  $rows[] = array(t('Monitor'));
  $rows[] = array(t('Processed:'), date("Y-m-d H:i:s T", $monitor->getProcessed()));
  $rows[] = array(t('Name:'), $monitor->getName());
  $rows[] = array(t('Description:'), $monitor->getDescription());
  $rows[] = array(t('Module:'), $monitor->getModule());
  $rows[] = array(t('Identifier:'), $monitor->getId());

  $feedback = DataMonitoringFeedbackHandler::getInstance()->getFeedback();
  if (!empty($feedback)) {
    $rows[] = array();
    $rows[] = array(t('Notes'));
  }
  foreach ($feedback as $type => $messages) {
    foreach ($messages as $message) {
      $rows[] = array(t(drupal_strtoupper($type)), t($message['message'], $message['vars']));
    }
  }

  $rows[] = array();
  $rows[] = array(t('Result'));
  $rows[] = array(t('Result count:'), $monitor->getResultCount());
  $rows[] = array(t('Responses pending at time of log:'), $monitor->getResponsesPending());
  $rows[] = array(t('Responses implemented at time of log:'), $monitor->getResponsesImplemented());
  $rows[] = array(t('Log file:'), url($monitor->getLogFile(), array('absolute' => TRUE))); // FIXME: Url is not showing.

  $rows[] = array();
  foreach ($rows as $i => $row) {
    $rows[$i] = implode("\t", $row);
  }
  return implode("\n", $rows);
}
/**
 * Plain text representation of monitor results.
 * @param Object $monitor
 */
function theme_data_monitoring_monitor_details_plain($monitor) {

  $rows = array();
  $rows[] = array(t('Records returned'));
  $result = $monitor->getResult();
  $rows[] = $result['header'];
  $rows = array_merge($rows, $result['rows']);
  foreach ($rows as $i => $row) {
    foreach ($row as $k => $cell) {
      $row[$k] = _data_monitoring_remove_control_characters($cell);
      if (is_null($cell)) $row[$k] = "NULL";
    }
    $rows[$i] = implode("\t", $row);
  }

  return theme('data_monitoring_monitor_summary_plain', $monitor) . "\n" . implode("\n", $rows);

}
/**
 * Plain text response log.
 * @param Object $monitor
 */
function theme_data_monitoring_response_summary_plain($response) {

  global $base_root;

  $site_name = variable_get('site_name', 'Drupal');
  $site_url = $base_root;

  $rows = array();

  $rows[] = array(t('Site name:'), $site_name);
  $rows[] = array(t('Site url:'), $site_url);
  $rows[] = array();
  $rows[] = array(t('Response details'));
  $rows[] = array(t('Description:'), $response->getDescription());
  $rows[] = array(t('Processed:'), date("Y-m-d H:i:s T", $response->getProcessed()));
  $rows[] = array(t('Response state:'), _data_monitoring_readable_state($response->getResponseState()));
  $rows[] = array();
  $rows[] = array(t('Result'));
  $rows[] = array(t('Created:'), $response->getCreatedResultCount());
  $rows[] = array(t('Updated:'), $response->getUpdatedResultCount());
  $rows[] = array(t('Deleted:'), $response->getDeletedResultCount());
  $rows[] = array(t('Remaining:'), $response->getRemainingResultCount());

  foreach ($rows as $i => $row) {
    $rows[$i] = implode("\t", $row);
  }
  return implode("\n", $rows);
}
/**
 * Plain text email report.
 * @param $monitors
 */
function theme_data_monitoring_email_report_plain($monitors, $language = NULL) {

  $output = '';
  foreach ($monitors as $monitor) {
    if ($monitor->getResponsesPending() || $monitor->getResponsesImplemented()) {
      $output .= "\n\n\n" . theme('data_monitoring_monitor_summary_plain', $monitor);
    }
  }
  if (empty($output)) return NULL;

  global $base_root;

  return t('Site:', array(), $language)
         . "\t"
         . variable_get('site_name', 'Drupal')
         . "($base_root)"
         . $output;
}
/**
 * HTML email report.
 * @param $monitors
 */
function theme_data_monitoring_email_report_html($monitors, $language = NULL) {

  $output = '';
  foreach ($monitors as $monitor) {
    if ($monitor->getResponsesPending() || $monitor->getResponsesImplemented()) {
      $output .= "<div class=\"monitor\">\n"
               . "  <h2>" . $monitor->getName() . "</h2>\n"
               . "  <p>" . $monitor->getDescription() . "</p>\n"
               . "  <p>" . t('Processed on !date', array('!date' => gmdate("d F, Y h:m:i A T", $monitor->getProcessed())), $language) . "</p>\n"
               . "  <p>" . t('Monitor id !id', array('!id' => $monitor->getId()), $language) . "</p>\n"
               . "  <p>" . t('Monitor registered by !module_name module', array('!module_name' => $monitor->getModule()), $language) . "</p>\n"
               . "  <p>" . format_plural($monitor->getResultCount(), '1 record found.', '@count records found.', array(), $language) . "</p>\n"
               . "  <p>" . format_plural($monitor->getResponsesPending(), '1 response pending at time of log.', '@count responses pending at time of log.', array(), $language) . "</p>\n"
               . "  <p>" . format_plural($monitor->getResponsesImplemented(), '1 response implemented at time of log.', '@count responses implemented at time of log.', array(), $language) . "</p>\n"
               . "  <p>" . t('<a href="!url">Log file</a> (also see attached)', array('!url' => url($monitor->getLogfile(), array('absolute' => TRUE))), $language)
               . "</div>\n";
    }
  }
  if (empty($output)) return NULL;

  $css = url(drupal_get_path('module', 'data_monitoring') .'/css/data_monitoring.css', array('absolute' => TRUE));

  return "<html>\n"
          . "  <head>\n"
          . "    <link type=\"text/css\" rel=\"stylesheet\" media=\"all\" href=\"{$css}\" />\n"
          . "  </head>\n"
          . "  <body class=\"email\">\n"
          . "    $output\n"
          . "  </body>\n"
          . "</html>"
          . "\n\n";

}
