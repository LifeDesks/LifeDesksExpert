<?php

/**
 * @file taxonpage_biblio.tpl.php
 * Display the bibliography on the taxon page
 *
 * Variables available:
 * - $biblio: An array of biblio items, sorted by author with elements clean, full, and attachments
 */

if(!empty($biblio)) {
?>
<h2 class="taxonpage"><?php print t('References') ?></h2>
<?php
  foreach ($biblio as $nid => $biblio_item) {
    print $biblio_item['full'];
  }
}
?>