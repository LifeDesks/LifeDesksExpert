<?php
/**
 * @file data_monitoring_multisite.overview.inc
 * Menu callback for data monitoring multisite overview which includes a summary of results.
 */

/**
 * Callback for overview form.
 * Summarises monitor counts from all sites over time.
 *
 * @return
 *    String
 */

function data_monitoring_multisite_overview() {

  $module_path = drupal_get_path('module', 'data_monitoring_multisite');

  drupal_set_html_head('<script src="https://www.google.com/jsapi" type="text/javascript"></script>');
  drupal_add_js($module_path . '/themes/js/data_monitoring_multisite.visualisations.js');
  drupal_add_js($module_path . '/themes/js/data_monitoring_multisite.js');
  drupal_add_css($module_path . '/themes/css/data_monitoring_multisite.css');


  // We need to sum monitor counts from all sites for a particular time
  // Graph of total count over time for all issues.

  $result = db_query("SELECT id, name, description, module, SUM(result_count) AS total_count,
                      (FLOOR(processed / 3600) * 3600) AS processed_floor
                      FROM {data_monitoring_multisite_log}
                      WHERE processed > 0
                      GROUP BY module, id, processed_floor");
  $last_processed = db_result(db_query("SELECT (FLOOR(MAX(processed) / 3600) * 3600) AS last_processed
                                        FROM {data_monitoring_multisite_log}"));
  $table_headers = array(
    t('Monitor'),
    t('Count'),
  );
  $latest_counts_headers = $table_headers;
  $latest_counts_headers[] = t('Processed');
  $latest_interval = 604800; // One week.


  $monitors_summary = array();
  $chart_data = array();
  while ($row = db_fetch_object($result)) {
    $identifier = $row->module . '_' . $row->id;
    if (!isset($monitors_summary[$identifier]['summary'])) {
      $monitors_summary[$identifier] = array(
        'module' => $row->module,
        'id' => $row->id,
        'name' => $row->name,
        'description' => $row->description,
        'latest_count' => 0,
      );
    }
    if ($row->processed_floor == $last_processed) $monitors_summary[$identifier]['latest_count'] = $row->total_count;
    $chart_data[$row->processed_floor][$identifier] = $row->total_count;
    unset($identifier);
  }

  ksort($chart_data);
  // Make sure we have points for each column at each time
  $lastest_counts_rows = array();
  foreach ($chart_data as $processed => $column) {
    foreach ($monitors_summary as $monitor => $summary) {
      if (!isset($chart_data[$processed][$monitor])) $chart_data[$processed][$monitor] = 0;
      // Show the latest counts according to the latest interval.
      if ($processed > (time() - $latest_interval)) {
        $latest_counts_rows[] = array(
          $summary['name'],
          $chart_data[$processed][$monitor],
          date('d M Y', $processed),
        );
      }
    }
    ksort($chart_data[$processed]);
    array_unshift($chart_data[$processed], $processed * 1000);
  }
  ksort($monitors_summary);

  drupal_add_js(array('chart_data' => $chart_data, 'monitors_summary' => $monitors_summary), 'setting');

  foreach ($monitors_summary as $monitor => $summary) {
    $last_counts_rows[] = array(
      '<a class="monitor-name" href="'
      . url('admin/reports/data-monitoring-multisite/monitor/' . $summary['module'] . '/' . $summary['id']) . '">'
      . $summary['name'] . '</a> <span class="monitor-description">' . $summary['description'] . '</span>',
      $summary['latest_count'],
    );
  }
  $last_counts = theme('table', $table_headers, $last_counts_rows, array('id' => 'last_counts', 'class' => 'data-monitoring-multisite'),
    t('Most recent counts from each site summed for each monitor. '
    . 'Counts were taken on !last_processed', array('!last_processed' => date('d F, Y', $last_processed))));
  $latest_counts = theme('table', $latest_counts_headers, $latest_counts_rows, array('id' => 'latest_counts', 'class' => 'data-monitoring-multisite'),
    t('Latest counts from each site are summed for each monitor over time.'));

  return theme('data_monitoring_multisite_overview', $monitors_summary, $last_counts, $latest_counts);


}