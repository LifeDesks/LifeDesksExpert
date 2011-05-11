<?php

/**
 * @file taxonpage_entire.tpl.php
 * Display content from other template files on the taxon page
 *
 * Variables available:
 *  - $tid : 
 *  - $name :
 *  - $canonical :
 *  - $ancestry : 
 *  - $images :   
 *  - $simplemappr: 
 *  - $chapters :  
 *  - $taxonomy :  
 *  - $vernaculars :  
 *  - $flags :
 *  - $biblio :  
 *  - $classification_biblio : the original species description
 *  - $attachments : attachments on nodes behind taxon page
 *  - $authors : a comma separated value of authors' uid
 *  - $languages : an array of languages for content
 */

 global $base_url,$user;

 $users = explode(',',$authors);

 $module_path = drupal_get_path('module', 'taxonpage') . '/';

 $current->tid = $tid;
 $ancestors = array();
 while ($parents = taxonomy_get_parents($current->tid)) {
   $current = array_shift($parents);
   $ancestors[] = 'n' . $current->tid;
 }
 $ancestors = array_reverse($ancestors);
 $ancestors = implode(",", $ancestors);

 drupal_add_js(array(
	'classification_tags' => array('ancestry' => $ancestors, 'tid' => 'n' . $tid),
	'lifedesk_biblio_pid' => variable_get('lifedesk_biblio_pid', ''),
	'lifedesk_biblio_parserurl' => variable_get('lifedesk_biblio_parserurl', ''),
	'lifedesk_biblio_iconpath' => '/' . drupal_get_path('module', 'lifedesk_biblio') . '/images/'
	), 'setting');
 drupal_add_css($module_path . 'css/taxonpage.css');
 drupal_add_css($module_path . 'css/jcarousel/skin.css');

 drupal_add_css($module_path . 'css/taxonpage_print.css', 'module', 'print');	
 drupal_add_css($module_path . 'css/jquery.jcarousel.css');
 
 drupal_add_js($module_path . 'js/jquery.refparser.js');
 drupal_add_js($module_path . 'js/jquery.galleria.js');
 drupal_add_js($module_path . 'js/jquery.jcarousel.pack.js');

 $browser = $_SERVER['HTTP_USER_AGENT'];

 if(strpos($browser,"MSIE") !== false) {
   drupal_add_js($module_path . 'js/jquery.bgiframe.min.js');
   drupal_add_js($module_path . 'js/excanvas.js');
 }

 drupal_add_js($module_path . 'js/jquery.bt.min.js');
 drupal_add_js($module_path . 'js/taxonpage.js');

 drupal_add_js(array('tid' => $tid), 'setting');

?>

<div class="taxonpage_wrapper">

	<div id="taxonpage_classification">
		
	<ul class="taxonpage_links classification_tree"><li class="classification_toggle classification_show"><a href="#" onclick="javascript:return false;"><?php print t('Show classification'); ?></a></li><li class="classification_toggle  classification_hidden"><a href="#" onclick="javascript:return false;"><?php print t('Hide classification'); ?></a></li></ul>

	<div id="classification_tree_wrapper">		
	  <div id="classification_tree_div" class="classification_tree_div">
	    <ul class="taxonpage_links_expand">
		  <li class="expand"><a href="#" onclick="javascript:return false">&nbsp;</a></li>
		  <li class="contract"><a href="#" onclick="javascript:return false">&nbsp;</a></li>
	    </ul>

	<?php
	  $block = module_invoke('classification', 'block', 'view', 0);
	  print $block['content'];
	?>

	  </div>
	</div>
	
	</div>
	
<div id="taxonpage-public-tools">
	<?php if(isset($_GET['lang']) && array_key_exists($_GET['lang'], $languages)): ?>
		<span class="print"><?php print l(t('Print'), 'pages/' . $tid . '/print/', array('query' => array('lang' => $_GET['lang']))); ?></span>
		<span class="pdf"><?php print l(t('PDF'), 'pages/' . $tid . '/pdf/', array('query' => array('lang' => $_GET['lang']))); ?></span>
	<?php else: ?>
		<span class="print"><?php print l(t('Print'), 'pages/' . $tid . '/print'); ?></span>
		<span class="pdf"><?php print l(t('PDF'), 'pages/' . $tid . '/pdf'); ?></span>
	<?php endif; ?>
		<span class="eol"><?php print l(t('EOL'), 'http://www.eol.org/' . str_replace(' ', '%20', htmlspecialchars($canonical)), array('absolute' => true, 'attributes' => array('target' => '_blank', 'rel' => 'nofollow')) ); ?></span>
</div>

<?php
    if(count($users) > 1) {
?>

<div id="taxonpage_author_list">
	<form id="taxonpage_author_selection">
		<label for="taxonpage_author_select"><?php print t('Filter by compiler:'); ?></label>
		<select id="taxonpage_author_select">
			<option value="" selected><?php print t('--Select--'); ?></option>
			<option value="0"><?php print t('All'); ?></option>
			<?php
			  foreach($users as $author) {
				$profile = user_load(array('uid' => $author));
			    print '<option value="' . $author . '">' . $profile->surname . ', ' . $profile->givenname . '</option>';	
			  }
			?>
		</select>
	</form>
</div>

<?php
	}
?>	
<h1 class="taxonpage">
  <?php 
    print $name; 
  	if(user_access("edit classification")) {
	  print ' <span class="taxonpage-tools taxonpage-editname">' . l(t('Edit') ,'admin/classification/biological/edit/' . $vid . '/' . $tid) . '</span>';
	}
  ?> 
</h1>

<div class="taxonpage-ancestry">
<?php print $ancestry; ?>
</div>
<div class="taxonpage-flags">
<?php if ($flags) { ?>
<div>
<h3 class="h"><?php print t('Biological Flags'); ?></h3>
<?php print $flags; ?>
</div>
<?php } ?>
</div>
<div style="clear:both"></div>

<?php print $classification_biblio; ?>
<?php print $vernaculars; ?>

<?php 
  print $images;
  if(user_access("create images")) {  
    print '<span class="taxonpage-tools taxonpage-add taxonpage-add-image">' . l(t('Add image'), 'node/add/image/' . $tid) . '</span>';
  }
?>

<?php 
  print $simplemappr;
  if(user_access("create simplemap")) {  
    print '<span class="taxonpage-tools taxonpage-add taxonpage-add-simplemappr">' . l(t('Add shaded map'), 'node/add/simplemappr/' . $tid) . '</span>';
  }
?>

      <?php if (count($languages) > 0): ?>
	  <div id="taxonpage-language-selection"><a name="taxonpage-language"></a>
	  <?php print t("Languages:"); ?>
	  <?php foreach($languages as $key => $language): ?>
		  <span class="taxonpage-language"><?php print l($language, 'pages/'.$tid.'/', array('query' => array('lang' => $key), 'fragment' => 'taxonpage-language')); ?></span>
	  <?php endforeach; ?>
	  </div>
	  <?php endif; ?>

<?php
  print $chapters; 
  if(user_access("create taxon description") && !in_array($user->uid,$users) && !$authors) {
    print '<span class="taxonpage-tools taxonpage-add taxonpage-add-description">' . l(t('Add description'), 'node/add/taxon-description/' . $tid) . '</span>';
  }
?>

<?php if(user_access("create taxon description") && in_array($user->uid,$users)): ?>
<div id="taxonpage_add_chapter">

<form id="taxonpage_language_selection">
<label for="taxonpage_language_select" class="taxonpage_language_select"><?php print t('Add Chapter:'); ?></label>
<select id="taxonpage_language_select" name="taxonpage_language_select">
<option value=""> --- <?php print t('Select') ?> --- </option>
<?php
foreach(locale_language_list() as $key => $language) {
	print '<option value="' . $key . '">' . $language . '</option>';
}
?>
</select>
<span id="taxonpage_chapter_selections"></span>
</form>

</div>
<?php endif; ?>

<div class="taxonpage-children">
<?php print $children; ?>
</div>
<?php print $taxonomy; ?>
<?php print $taxonomic_notes; ?>
<?php print $biblio; ?>
<?php print $attachments; ?> 

</div>
