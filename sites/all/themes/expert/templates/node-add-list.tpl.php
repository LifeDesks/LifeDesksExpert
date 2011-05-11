<?php
// $Id$
/**
 * @file node-add-list.tpl.php
 * Theme the listings on node/add page.
 */
?>
<?php if ($content): ?>
<div id="node-type-list" class="admin-lifedesk-panel">
<ul>
<?php foreach ($content as $item): ?>
<?php if($item['link_title']): ?>
<li>
<div class="node-type admin-lifedesk-item">
	<h3 class="<?php print preg_replace('#[^A-z]#i', '-', $item['link_path']); ?>"><?php print $item['title']; ?></h3>
	<p class="description"><?php print filter_xss_admin($item['description']); ?></p>
	<?php if ($item['link_path'] == 'node/add/taxon-template'): ?>
	  <p class="create"><?php print l(t('Export/Import Template'), $item['href'], $item['localized_options']); ?></p>
	<?php elseif ($item['link_path'] == 'node/add/biblio'): ?>
	  <p class="create"><?php print l(t('Create !item', array('!item' => $item['title'])), $item['href'], $item['localized_options']); ?>, <?php print l(t('Import Biblio'), 'biblio/import'); ?></p>
	<?php elseif ($item['link_path'] == 'node/add/taxon-description'): ?>
	  <p class="create"><?php print l(t('Create !item', array('!item' => $item['title'])), $item['href'], $item['localized_options']); ?>, <?php print l(t('Import Pages'), 'taxa/template'); ?></p>
	<?php else: ?>
	  <p class="create"><?php print l(t('Create !item', array('!item' => $item['title'])), $item['href'], $item['localized_options']); ?></p>
	<?php endif; ?>
</div>
</li>
<?php endif; ?>
<?php endforeach; ?>
</ul>
</div>
<?php endif; ?>