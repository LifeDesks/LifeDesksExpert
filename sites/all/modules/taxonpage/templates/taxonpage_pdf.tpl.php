<?php

/**
 * @file taxonpage_pdf.tpl.php
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
global $base_url;

$css = $base_url . '/' . drupal_get_path('module', 'taxonpage') . '/';

?>

<html>
<head>
	<link type="text/css" rel="stylesheet" href="<?php print $css . 'css/taxonpage_print.css'; ?>" />
	<meta name="ROBOTS" content="NOINDEX,NOFOLLOW" />
</head>
<body>
<div class="taxonpage_wrapper">
<h1 class="taxonpage"><?php print $name; ?></h1>

<?php print $ancestry; ?>
<?php print $classification_biblio; ?>
<?php print $chapters; ?>
<?php print $taxonomy; ?>
<?php print $taxonomic_notes; ?>
<?php print $vernaculars; ?>
<?php print $children; ?>
<?php print $biblio; ?>
<?php print $images_print; ?>
<?php print $simplemappr_print; ?>
</div>
</body>
</html>