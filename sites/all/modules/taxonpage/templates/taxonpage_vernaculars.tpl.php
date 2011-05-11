<?php

/**
 * @file taxonpage_taxonomy.tpl.php
 * Display vernaculars on the taxon page
 *
 * Variables available:
 * - $vernaculars: An array of vernaculars
 */

if(!empty($vernaculars)) {
	if(user_access("edit classification")) $edit = true;
?>

<h2 class="taxonpage"><?php print t('Common Names') ?></h2>
<?php
$names = array();
foreach ($vernaculars as $vernacular) {
  $vern_lang = !empty($vernacular->language) ? ' (' . $vernacular->language . ')' : '';
  $edit_name = ($edit) ? ' <span class="taxonpage-tools taxonpage-editname">' . l(t('Edit') ,'admin/classification/biological/edit/' . $vernacular->vid . '/' . $vernacular->tid) . '</span>' : '';
  $names[] = '<span class="taxonpage_vernacularname">' . $vernacular->name . '</span>' . $vern_lang . $edit_name;
}
print '<p>' . implode(', ', $names) . '</p>';

}
?>