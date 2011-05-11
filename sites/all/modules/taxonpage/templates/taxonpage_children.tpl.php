<?php

/**
 * @file taxonpage_children.tpl.php
 * Display children on the taxon page
 *
 * Variables available:
 * - $children: An array of parental names
 */

if(!empty($children['taxa'])):
	if(user_access("edit classification")) $edit = true;
?>
<h2 class="taxonpage"><?php print t('Taxonomic Children') ?></h2>

<?php
$names = array();
foreach ($children['taxa'] as $child) {
  $edit_name = ($edit) ? ' <span class="taxonpage-tools taxonpage-editname">' . l(t('Edit') ,'admin/classification/biological/edit/' . $child['vid'] . '/' . $child['tid'], array('html' => true)) . '</span>' : '';
  $names[] = '<span>' . $child['link'] . '</span>'.$edit_name;
}
print '<p><strong>Total: ' . $children['total'] . '</strong></p><p>' . implode(', ', $names) . '</p>';
endif;
?>