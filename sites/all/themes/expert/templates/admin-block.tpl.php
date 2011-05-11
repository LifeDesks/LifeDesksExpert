<?php
// $Id: admin-block.tpl.php,v 1.1 2010/04/13 18:06:38 dhorthouse Exp $

/**
 * @file admin-block.tpl.php
 * Theme the admin page blocks.
 *
 */
?>

<?php if (!empty($block['content'])): ?>
  <div class="admin-panel admin-lifedesk-item">
    <h3 class="admin-panel-default <?php print preg_replace('#[^A-z]#i', '-', $block['path']); ?>">
      <?php print $block['title']; ?>
    </h3>
    <div class="body">
      <p class="description">
        <?php print $block['description']; ?>
      </p>
      <?php print $block['content']; ?>
    </div>
  </div>
<?php endif; ?>