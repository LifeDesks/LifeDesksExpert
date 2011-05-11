<?php

/**
 * @file taxonpage_taxonomic_notes.tpl.php
 * Display the term description on the taxon page
 *
 * Variables available:
 * - $description: string
 * - $vid: integer
 * - $tid: integer
 */

if(!empty($description)) {
	if(user_access("edit classification")) $edit = true;
?>

<h2 class="taxonpage"><?php print t('Taxonomic Notes') ?></h2>
<p>
<?php print filter_xss($description); ?>
<?php if($edit): ?>
<ul class="taxonpage_inline_links">
  <li class="edit_description"><?php print l(t('Edit') ,'admin/classification/biological/edit/' . $vid . '/' . $tid, array('absolute' => true)); ?></li>
</ul>
<?php endif; ?>
</p>

<?php } ?>