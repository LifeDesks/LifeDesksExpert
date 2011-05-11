<?php

/**
 * @file data-monitoring-multisite-overview.tpl.php
 * Results for monitors over time.
 */

?>
<h2><?php print t('Summary of monitor results'); ?></h2>

<h3><?php print t('Total count from all LifeDesks of records affected by each issue over time'); ?></h3>
<div id="interactive_chart" style="width: 735px; height: 500px;"></div>

<?php print $last_counts; //print $latest_counts; ?>