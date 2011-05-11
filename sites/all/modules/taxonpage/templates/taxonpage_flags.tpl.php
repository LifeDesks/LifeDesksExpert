<?php

/**
 * @file taxonpage_flags.tpl.php
 * Display vernaculars on the taxon page
 *
 * Variables available:
 * - $flags: An array of flags and their data
 */

if(!empty($flags)) {	
?>

<ul class="taxonpage_flags">
<?php
foreach ($flags as $flag) {
  $description = '';
  if($flag->description) {
    $description = '<a class="flag-info" title="' . $flag->description . '"></a>';	
  }
?>
<li><?php print '<strong>' . $flag->vocab . '</strong>: ' . $flag->name; ?><?php print $description; ?></li>
<?php
}
?>
</ul>

<?php
}
?>