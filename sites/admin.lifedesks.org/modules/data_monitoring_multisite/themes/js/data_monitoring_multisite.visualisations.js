Drupal.monitoring = {};

Drupal.monitoring.drawChart = function() {
  var monitors_summary = Drupal.settings.monitors_summary;
  var chart_data = Drupal.settings.chart_data;

  var data = new google.visualization.DataTable();
  data.addColumn('date', 'Date');
  for (monitor in monitors_summary) {
    data.addColumn('number', monitors_summary[monitor]['name']);
  }

  var rows = new Array();
  var i = 0;
  for (timestamp in chart_data) {
    var row = new Array();
    var j = 0;
    for (column in chart_data[timestamp]) {
      row[j] = (j == 0) ? new Date(chart_data[timestamp][column]) : parseInt(chart_data[timestamp][column]);
      j++;
    }
    rows[i] = row;
    i++;
  }
  console.debug(rows);
  data.addRows(rows);

  var chart = new google.visualization.AnnotatedTimeLine(document.getElementById('interactive_chart'));
  chart.draw(data, {displayAnnotations: true, legendPosition: 'newRow'});
};

google.load('visualization', '1', {'packages':['annotatedtimeline']});
google.setOnLoadCallback(Drupal.monitoring.drawChart);
