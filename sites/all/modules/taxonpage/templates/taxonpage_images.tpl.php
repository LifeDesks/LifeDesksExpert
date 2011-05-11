<?php

/**
 * @file taxonpage_images.tpl.php
 * Display the images on the taxon page
 *
 * Variables available:
 * - $images: An array of image data
 */

if(!empty($images)) {
  global $base_url;

  $comment_link = false;

  drupal_add_js(array('taxonpageImageData' => $images), 'setting');
  $cnt = 0;
?>
<div class="taxonpage_images">
<ul id="taxonpage_gallery" class="gallery jcarousel-skin-tango">

<?php
  foreach($images as $image){
    if($image['exemplar']){
      $cnt++;
    }
  }
  if($cnt){
    foreach($images as $image){
        $node_link = '/node/' . $image['node'];
?>

<li<?php print $image['exemplar'];?>><img src="<?php print $image['thumbnail']; ?>" class="taxonpage_image" name="<?php print htmlspecialchars($image['preview'],ENT_QUOTES); ?>||<?php print htmlspecialchars($node_link,ENT_QUOTES); ?>" alt="" title="" rel="<?php print $image['thumbnail']; ?>"></li>

<?php	
    }
  } else {
    $exemplar = " class='active'";
    foreach($images as $image){
        $node_link = '/node/' . $image['node'];
?>

<li<?php print $exemplar; ?>><img src="<?php print $image['thumbnail']; ?>" name="<?php print htmlspecialchars($image['preview'],ENT_QUOTES); ?>||<?php print htmlspecialchars($node_link,ENT_QUOTES); ?>" alt="" title=""></li>

<?php
      $exemplar = '';
    }
  }
?>
				
</ul>
<div style="clear:both"></div>
</div>

<?php
}
?>