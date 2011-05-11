<?php

/**
 * @file taxonpage_print.tpl.php
 * Display print view of the taxon page
 *
 * Variables available:
 *  - $name :
 *  - $tid : 
 *  - $ancestry : 
 *  - $images_print :
 *  - $simplemappr_print :  
 *  - $chapters :  
 *  - $taxonomy : 
 *  - $taxonomic_notes : 
 *  - $vernaculars :  
 *  - $biblio :  
 *  - $classification_biblio : the original species description
 *  - $citation : 
 *  - $authors : an array of authors' uid
 *  _ $empty_chapters: an array of chapters with no content (includes a node id if the user has authored any content)
 */

 $module_path = drupal_get_path('module', 'taxonpage') . '/';
 drupal_set_html_head('<meta name="ROBOTS" content="NOINDEX,NOFOLLOW" />');
 drupal_add_css($module_path . 'css/taxonpage_print.css');	

?>
	
<h1 class="taxonpage"><?php print $name; ?></h1>

<?php print $ancestry; ?>
<?php print $classification_biblio; ?>
<?php print $images_print; ?>
<?php print $simplemappr_print; ?>
<?php print $chapters; ?>
<?php print $taxonomy; ?>
<?php print $taxonomic_notes; ?>
<?php print $vernaculars; ?>
<?php print $children; ?>
<?php print $biblio; ?>
