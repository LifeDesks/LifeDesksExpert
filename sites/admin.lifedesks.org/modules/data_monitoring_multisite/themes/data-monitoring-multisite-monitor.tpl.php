<?php

/**
 * @file data-monitoring-multisite-monitor.tpl.php
 * Results for a specific monitor over time.
 */

?>
<?php if (!empty($referrer)): ?><a href="/<?php print $referrer; ?>">Back to previous results</a><?php endif; ?>
<h2><?php print $monitor['name']; ?></h2>
<p><?php print $monitor['description']; ?></p>
<img id="chart" src="<?php print $chart_src; ?>" />
<?php print $last_count_table; ?>