<?php
// $Id: node.tpl.php,v 1.5 2007/10/11 09:51:29 goba Exp $
?>
<div id="node-<?php print $node->nid; ?>" class="node<?php if ($sticky) { print ' sticky'; } ?><?php if (!$status) { print ' node-unpublished'; } ?>">

<?php print $picture ?>

<?php if ($page == 0): ?>
  <h2><?php print $title ?></h2>
<?php endif; ?>

  <?php if ($submitted): ?>
    <span class="submitted"><?php print $submitted; ?></span>
  <?php endif; ?>

  <div class="content clear-block node-image">
    <?php print $content ?>
  </div>

  <div class="clear-block">
    <?php if($credit): ?>
      <div class="credit"><strong><?php print t('Photographer'); ?></strong>: <?php print $credit; ?></div>
    <?php endif; ?>
    <?php if($cc_lite_license): ?>
      <div class="license"><img src="<?php print $cc_lite_license_img; ?>" alt="<?php print $cc_lite_license; ?>"/></div>
    <?php endif; ?>
  </div>

  <div class="clear-block">
    <div class="meta">
    <?php if ($taxonomy): ?>
      <div class="terms"><?php print $terms ?></div>
    <?php endif;?>
    </div>

    <?php if ($links): ?>
      <div class="links"><?php print $links; ?></div>
    <?php endif; ?>
  </div>

</div>
