<?php

/**
 * @file taxon_description_rightsholder.tpl.php
 * Display the rightsholder(s) a taxon description chapter
 *
 * Variables available:
 * - $title: The title
 * - $rightsholders: The rightsholders
 */
?>
<div class="taxon_description_chapter_rightsholders">
  <span class="contributors"><?php print $title; ?></span>
  <?php print $rightsholders; ?>
</div>