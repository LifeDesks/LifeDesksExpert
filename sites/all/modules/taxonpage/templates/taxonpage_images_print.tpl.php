<?php

/**
 * @file taxonpage_images_print.tpl.php
 * Display the images on the taxon page
 *
 * Variables available:
 * - $images: An array of image data
 */

if(!empty($images)) {
?>
<?php 
  foreach($images as $image) {
	  $info = image_get_info(getcwd() . $image['preview']);
?>
<p class="taxonpage_images"><img src="<?php print $image['preview']; ?>" class="taxonpage_image" alt="" style="width:<?php print $info['width']; ?>px;height:<?php print $info['height']; ?>px"><br />
<?php print $image['cclicense'] . '<br /><strong>' . $image['title'] . '</strong><br /><strong>' . t('Photographer:') . '</strong> ' . $image['credit'] . ' <strong>' . t('Submitted by:') . '</strong> ' . $image['author']; ?>
<?php print ($image['license'] != 'publicdomain') ? ' <strong>' . t('Rights holder:') . '</strong> ' . $image['rights'] : ''; ?> </p>
<?php } ?>
<?php 
} 
?>