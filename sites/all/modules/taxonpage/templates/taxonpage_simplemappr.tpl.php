<?php

/**
 * @file taxonpage_simplemappr.tpl.php
 * Display a shaded map on the taxon page
 *
 * Variables available:
 * $simplemappr : an array of shaded map data
 */
?>

<?php if(!empty($simplemappr)): ?>
<?php
    global $user; 
	$comment_link = false;
	if(user_access('access comments')) {
	 $comment_link = true;
	} 
?>
<div class="taxonpage_simplemappr">
	<?php foreach($simplemappr as $map): ?>
		<?php
			$showmap = false;
			if($map['status'] == 1) $showmap = true;
			if($map['status'] == 0 && user_access('edit any simplemap') || (user_access('edit own simplemap') && $user->uid == $map['uid'])) $showmap = true;
		?>
		<?php if($showmap): ?>
			<div class="taxonpage_simplemappr_item<?php print ($map['status'] == 0) ? " draft" : ""; ?>">
			<?php $node_link = '/node/' . $map['node']; ?>
			<?php if($map['status'] == 0): ?>
				<div class="taxonpage_simplemappr_draft"><?php print t('Unpublished'); ?></div>
			<?php endif; ?>
			<?php print l('<img src="' . $map['preview'] . '" class="taxonpage_simplemappr" alt="' . $map['title'] . '" title="' . $map['title'] . '" />', 'node/' . $map['node'], array('html' => TRUE));
			?>
			<div class="taxonpage_simplemappr_caption">
				<p><?php print $map['title']; ?></p>
				<ul class="taxonpage_inline_links">
			  		<?php if(variable_get("taxonpage_show_license",FALSE) != FALSE): ?>
			     	<li><?php print $map['cclicense']; ?></li>
			  		<?php endif; ?>
			  		<?php if($map['permission'] == 1): ?>
		        	<li class="edit_simplemappr"><?php print l(t('Edit'),'node/' . $map['node'] . '/edit'); ?></li>
		      		<?php endif; ?>
		        	<?php $comment_link ? print '<li class="comment_balloon">' . l(t('Comment'),'node/' . $map['node']) . ' (' . $map['comment_count'] . ')</li>': "" ?>
		     	</ul>
		    </div>
			</div>
		<?php endif; ?>
	<?php endforeach; ?>
</div>
<?php endif; ?>