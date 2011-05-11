<?php

/**
 * @file taxonpage_taxonomy.tpl.php
 * Display the synonymy on the taxon page
 *
 * Variables available:
 * - $taxonomy: An array of synonyms
 */

if(!empty($taxonomy)) {
	global $NAMES;
	
	if(user_access("edit classification")) $edit = true;
?>

<h2 class="taxonpage"><?php print t('Taxonomy') ?></h2>
<ul class="taxonpage_taxonomy">

<?php
foreach ($taxonomy as $key) {
  print '<li><span class="taxonpage_synonym">' . $NAMES->italicized_form($key->name) . '</span> (' . $key->type . ')';
  if($edit) {
    print ' <span class="taxonpage-tools taxonpage-editname">' . l(t('Edit') ,'admin/classification/biological/edit/' . $key->vid . '/' . $key->tid) . '</span>';
  }
  print '</li>';
}
?>
</ul>
<?php
}
?>