<?php
/**
 * @file taxon_description_entire.tpl.php
 * Display the entire taxon description node
 *
 * Variables available:
 * - $taxon: The taxon about which the description is about
 * - $taxon_desc_chapters: The description about the taxon
 * - $link: An iconographic link to the taxon page
 */
?>
<h3><?php print $taxon; ?></h3>
<div id="taxon_description_preview">
<?php print $taxon_desc_chapters; ?>
<?php print $link; ?>
</div>
