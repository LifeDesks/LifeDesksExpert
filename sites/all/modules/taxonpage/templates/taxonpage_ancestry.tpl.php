<?php

/**
 * @file taxonpage_ancestry.tpl.php
 * Display the parental classification on the taxon page
 *
 * Variables available:
 * - $ancestry: An array of parental names
 */

if(!empty($ancestry)): ?>
<?php if(user_access("edit classification")) $edit = true; ?>
<ul class="taxonpage_ancestry">
<?php
$counter = 0;
foreach ($ancestry as $taxon): ?>
<?php $edit_name = ($edit) ? ' <span class="taxonpage-tools taxonpage-editname">' . l(t('Edit') ,'admin/classification/biological/edit/' . $taxon['vid'] . '/' . $taxon['tid'], array('html' => true)) . '</span>' : ''; ?>
<li class="taxonpage_rank_<?php print $counter ?>"><?php print $taxon['rank'] . ' ' . $taxon['link'] . ' ' .$edit_name; ?></li>
<?php 
$counter++; 
endforeach;
?>
</ul>
<?php endif; ?>