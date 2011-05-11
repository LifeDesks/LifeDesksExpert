<?php

/**
 * @file taxon_description_chapter.tpl.php
 * Display the chapter within a taxon description
 *
 * Variables available:
 * - $chapter_heading: The title of the chapter, pulled from the 'Taxon Description Chapters' vocabulary
 * - $chapter_content: The content of the chapter.
 */
?>

<h3><?php print $name; ?></h3>
<div class="taxon_description_chapter_content">
  <?php print $content; ?>
</div>
