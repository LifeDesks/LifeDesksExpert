<?php
// $Id: classification-block-page.tpl.php,v 1.1 2010/04/13 18:06:38 dhorthouse Exp $

/**
 * @file classification-block-page.tpl.php
 * Theme the admin/classification/biological page.
 *
 */
?>

<?php if ($content): ?>
<div id="admin-classification-list" class="admin-lifedesk-panel">
<ul>
<?php if (user_access('administer taxonomy')): ?>
<li>
<div class="admin-lifedesk-item">
	<h3 class="add-classification"><?php print t('Add & List Classifications'); ?></h3>
	<p class="description"><?php print t('Add a new empty classification or see all created.'); ?></p>
	<p class="create"><?php print l(t('Add Classification'), 'admin/content/taxonomy/add/vocabulary'); ?>, <?php print l(t('List Classifications'), 'admin/content/taxonomy/'); ?></p>
</div>
</li>
<?php endif; ?>
<?php foreach ($content as $item):
$class_name = preg_replace('#[^A-z]#i', '-', $item['link_path']);
?>
<li>
<div class="admin-lifedesk-item">
	<h3 class="<?php print $class_name; ?>"><?php print $item['title']; ?></h3>
	<p class="description"><?php print filter_xss_admin($item['description']); ?></p>
	<p class="create"><?php print l(t('!item', array('!item' => $item['title'])), $item['href'], $item['localized_options']); ?></p>
</div>
</li>
<?php endforeach; ?>
</ul>
</div>
<?php endif; ?>