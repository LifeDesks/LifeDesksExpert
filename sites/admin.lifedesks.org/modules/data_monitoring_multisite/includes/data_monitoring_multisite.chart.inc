<?php

/**
 * @file data_monitoring_multisite.analysis.inc
 * Menu callback for data monitoring multisite analysis form.
 */
/**
 * Callback for monitor chart.
 * Generates a line chart for a single monitor on multiple sites.
 *
 */
function data_monitoring_multisite_chart($module, $id = NULL) {
  $debug = FALSE;

  if (is_null($id) && $module == DATA_MONITORING_MULTISITE_OVERVIEW) {
    $result = db_query("SELECT id, name, description, module, SUM(result_count) AS total_count,
                      (FLOOR(processed / 3600) * 3600) AS processed_floor
                      FROM {data_monitoring_multisite_log} GROUP BY module, id, processed_floor");

    $chart_title = t('Total count of affected records from each LifeDesk and for each issue over time.');
    $max_y_values = array();
    $i = 0;
    while ($log = db_fetch_object($result)) {
      $issue = $log->module . '_' . $log->id;
      // x = processed
      // y = count
      // line = issue + lifedesk
      if (!is_null($log->total_count) && !is_null($log->processed_floor) && $log->processed_floor > 0) {
        $lines[$issue]['x'][] = $log->processed_floor;
        $lines[$issue]['y'][] = $log->total_count;
        $axes['x'][] = $log->processed_floor;
        $axes['y'][] = $log->total_count;
        if (!isset($max_y_values[$issue]) || $max_y_values[$issue] < $log->total_count) {
          $max_y_values[$issue] = $log->total_count;
        }
      }
      $i++;
    }
  }
  else {
    $result = db_query("SELECT dmmlid, shortname, id, module, name, description, processed, result_count
                        FROM {data_monitoring_multisite_log} dmml
                        WHERE dmml.module = '%s'
                        AND dmml.id = '%s'", $module, $id);

    $max_lines_to_show = 20;
    $max_y_values = array();
    $i = 0;
    while ($log = db_fetch_object($result)) {
      // x = processed
      // y = count
      // line = issue + lifedesk
      if (!is_null($log->result_count) && !is_null($log->processed) && $log->processed > 0) {
        $lines[$log->shortname]['x'][] = $log->processed;
        $lines[$log->shortname]['y'][] = $log->result_count;
        $axes['x'][] = $log->processed;
        $axes['y'][] = $log->result_count;
        if (!isset($max_y_values[$log->shortname]) || $max_y_values[$log->shortname] < $log->result_count) {
          $max_y_values[$log->shortname] = $log->result_count;
        }
      }
      $i++;
    }
  }

  $line_count = count($lines);
  arsort($max_y_values);
  $x_min = min($axes['x']);
  $x_max = max($axes['x']);
  $total_time = $x_max - $x_min;
  $y_min = min($axes['y']); if ($y_min < 10) $y_min = 0;
  $y_max = max($axes['y']);
  $total_values = $y_max - $y_min;

  $date_format = "d/M/y";
  $x_axis_labels = '|' . date($date_format, $x_min);
  $percentile = 0.25;
  while ($percentile < 1) {
    $x_axis_labels .= '|' . date($date_format, (($total_time * $percentile) + $x_min));
    $percentile += 0.25;
  }
  $x_axis_labels .= '|' . date($date_format, $x_max);

  $y_axis_labels = '|' . $y_min;
  $interval = round($total_values / 4);
  $mark = $interval;
  while ($mark < $y_max) {
    $y_axis_labels .= '|' . ($y_min + $mark);
    $mark += $interval;
  }
  $y_axis_labels .= '|' . $y_max;


  $base_colors = array(
    '00d5ff',
    'ff0000',
    '77ff33',
    'ffff00',
    'aa99ff',
    'ffaa00',
  );
  $colors = _generate_colors($base_colors, $line_count);

  $chart_data_labels = '';
  $chart_data_labels_color = '';
  $chart_data = 't:';
  $chart_data_markers = '';
  $i = 0;
  foreach ($max_y_values as $site => $max_y) {
    if ($i > ($max_lines_to_show - 1)) break; // We can only fit about 20 lines on our chart so we stop generating chart data here.

    if (count($lines[$site]['y']) == 1) {
      // Google line chart will only display points if there are at least two point values
      // so we prepend a point with x and y minimum values.
      array_unshift($lines[$site]['x'], $x_min);
      array_unshift($lines[$site]['y'], $y_min);
    }
    if ($i != 0) { // Add line delimiters to chart data strings if we're not on the first line.
      $chart_data .= '|';
      $chart_data_labels .= '|';
      $chart_data_labels_color .= ',';
      $chart_data_markers .= '|';
    }

    $chart_data_labels .= $site;
    $chart_data_labels_color .= $colors[$i];
    $chart_data .= implode(',', $lines[$site]['x']) . '|' . implode(',', $lines[$site]['y']);

    $chart_data_markers .= 'o,' . $colors[$i] . ",$i,,6"; //|A$site,000000,$i,1,10";

    $i++;
  }

  $chart_data_scale = '';
  for ($l = 0; $l < $line_count; $l++) {
    if (!empty($chart_data_scale)) $chart_data_scale .= ',';
    $chart_data_scale .= "$x_min,$x_max,$y_min,$y_max";
  }

  $url =  'http://chart.googleapis.com/chart?';


  if (count($lines) > $max_lines_to_show) {
    $chart_title = t('Count of records affected by issue over time (top !max_lines sites shown)', array('!max_lines' => $max_lines_to_show));
  }
  else {
    $chart_title = t('Count of records affected by issue over time.');
  }
  // Add data, chart type, chart size, and scale to params.
  $chart = array(
  'chid' => md5(uniqid(rand(), TRUE)),
  'chtt' => $chart_title,
  'cht' => 'lxy',
  'chs' => '735x408',
  'chma' => '20,20,0,20',
  'chxt' => 'x,y,x,y',
  'chxr' => "0,$x_min,$x_max|1,$y_min,$y_max",
  'chxl' => "0:$x_axis_labels|1:$y_axis_labels|2:|Date|3:|Count",
  'chxp' => '2,50|3,50',
  'chg' => '20,50,1,5',
  'chco' => $chart_data_labels_color,
  'chds' => $chart_data_scale,
  'chdl' => $chart_data_labels,
  'chm' => $chart_data_markers,
  'chd' => $chart_data,
  );

  $headers = array(
    'Content-Type' => 'application/x-www-form-urlencoded',
  );

  $result = drupal_http_request($url, $headers, 'POST', http_build_query($chart, '', '&'), $retry = 3);

  $ih = imagecreatefromstring($result->data);
  if ($debug) return t("Debugging");
  drupal_set_header("Content-type: image/png");
  imagepng($ih);
  imagedestroy($ih);

}
/**
 * Helper function to generate colors for lines.
 * @param $hex_base_colors
 *     Array of base colors.
 * @param $number_of_colors_needed
 *     Number of different colors to return.
 */
function _generate_colors($hex_base_colors, $number_of_colors_needed) {

  $colors = array();
  $valid_hex_pattern = '[a-fA-F0-9]';

  foreach ($hex_base_colors as $color) {
    if (preg_match("/^($valid_hex_pattern$valid_hex_pattern)($valid_hex_pattern$valid_hex_pattern)($valid_hex_pattern$valid_hex_pattern)\$/", $color, $matches)) {
      $valid_hex_base_colors[] = $color;
      $rgbs[] = array(
        hexdec($matches[1]),
        hexdec($matches[2]),
        hexdec($matches[3]),
      );
    }
  }
  $number_of_base_colors = count($valid_hex_base_colors);
  if ($number_of_base_colors == 0) return FALSE;

  for ($i = 0; $i < $number_of_colors_needed; $i++) {
    if ($i < $number_of_base_colors) {
      $colors[$i] = $valid_hex_base_colors[$i]; // Use base colors provided until we run out of colours.
      continue;
    }
    // We've run out of base colors so we loop through the base colors again to generate shades of them.
    $mod = $i % $number_of_base_colors;
    $mask = ($i > ($number_of_base_colors + (($number_of_colors_needed - $number_of_base_colors) / 2))) ? 255 : 0;
    $colors[$i] = '';
    foreach ($rgbs[$mod] as $k => $v) {
      $rgbs[$mod][$k] = round($v * 0.9) + round($mask * (1 - 0.9));
      if ($rgbs[$mod][$k] > 255) $rgbs[$mod][$k] = 255;
      $hex_value = dechex($rgbs[$mod][$k]);
      $colors[$i] .= (strlen($hex_value) == 1) ? "0$hex_value" : $hex_value;
      unset($hex_value);
    }
  }
  return $colors;

}
?>