<?php

/**
 * @file taxonpage_classification_biblio.tpl.php
 * Display the bibliographic item for the original description on the taxon page
 *
 * Variables available:
 * - $classification_biblio: the biblio item
 */

if(!empty($classification_biblio)) {
?>
<div id="taxonpage_original_description">
<h4 class="taxonpage"><?php print t('Original Published Description') . ':'; ?></h4>
<?php
  foreach($classification_biblio as $biblio_item) {
    print $biblio_item['full'];	
  }
?>
</div>
<?php
}
?>