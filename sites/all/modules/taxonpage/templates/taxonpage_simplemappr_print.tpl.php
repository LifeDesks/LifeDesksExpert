<?php

/**
 * @file taxonpage_simplemappr_print.tpl.php
 * Display a shaded map on the taxon page
 *
 * Variables available:
 * $simplemappr : an array of shaded map data
 */ 
?>

<?php if(!empty($simplemappr)): ?>
<?php foreach($simplemappr as $map): ?>
<p class="taxonpage_simplemappr">
<?php 
print '<img src="' . $map['preview'] . '" class="taxonpage_simplemappr" alt="' . $map['title'] . '" title="' . $map['title'] . '" /><br />';
print $map['title'];
?>
</p>
<?php endforeach; ?>
<?php endif; ?>