<?php

/**
 * @file taxon_description_config.tpl.php
 * Display the chapter configuration page
 *
 * Variables available:
 * - $vid: The relevant vocabulary
 * - $arg: 'mainchapters' or 'subchapters'
 */

  global $base_url;
  $module_path = drupal_get_path('module', 'taxon_description') .'/';
  drupal_add_css($module_path .'css/taxon_description_settings.css');
  drupal_add_js($module_path .'js/taxon_description_settings.js');
  drupal_add_js("misc/collapse.js", "core");

  drupal_add_js(array('chapter_config_callback' => base_path() . 'chapter_config/'),'setting');
  drupal_add_js(array('lang' => locale_language_list()),'setting');

  jquery_ui_add(array('ui.sortable'));

?>
<!--<div id="taxon_description_messages"></div>-->
<div id="contentLeft">
  <?php
  if($arg == 'headings') {
    echo '<div id="chapter_message" class="messages status">' . t('Headings are similar to book ends; they do not contain content. Instead, they organize chapters that do contain content.') . '</div>';
  }
  ?>
  <ul>
  <?php
  if($arg == 'headings') {
    $tree = taxon_description_taxonomy_get_tree($vid, 'en'); ?>
          <li class="ui-state-default ui-state-stable stable" id="tid_0" rel="0">
            <div class="column1"></div>
            <div class="column2"><strong><?php echo t("Headings"); ?></strong></div>
            <div class="column3" align="right"><a href="#" onclick="javascript:TD.addChapter(0);return false;"><?php print t('add'); ?></a></div>
          </li>
          <li class="ui-state-hidden" id="hidden">
            <div class="column1"><img src="<?php echo $base_url.'/'.$module_path.'images/move.png' ?>" alt="<?php print t("drag up or down"); ?>" /></div>
            <div class="column2"><span class="editable"></span></div>
            <div class="column4" id="removeme" align="right"><a href="#" class="delete" onclick="javascript:TD.deleteChapter($(this));return false;"><?php print t('delete'); ?></a></div>
            <div class="column5" id="editme-<?php print $tid; ?>" align="right"><a href="#" class="edit" onclick="javascript:TD.editChapter($(this));return false;"><?php print t('edit'); ?></a></div>
          </li>
    <?php foreach ($tree as $i){
      $tid = $i->tid;
      $name = $i->name;
      $description = ($i->description) ? (strlen($i->description) > 75) ? substr($i->description,0,75) . '...' : $i->description : '';
      $parent = $i->parents[0];
      if($parent == 0) { ?>
        <li class="ui-state-default" id="tid_<?php echo $tid; ?>">
          <div class="column1"><img src="<?php echo $base_url.'/'.$module_path.'images/move.png' ?>" alt="<?php print t('drag up or down'); ?>" /></div>
          <div class="column2">
	      <span class="editable">
          <?php
            echo '<strong>' . $name . '</strong>';
            if($description) echo ': ' . $description;
          ?>
          </span>
          </div>
          <div class="column4" id="removeme-<?php print $tid; ?>" align="right"><a href="#" class="delete" onclick="javascript:TD.deleteChapter($(this));return false;"><?php print t('delete'); ?></a></div>
          <div class="column5" id="editme-<?php print $tid; ?>" align="right"><a href="#" class="edit" onclick="javascript:TD.editChapter($(this));return false;"><?php print t('edit'); ?></a></div>
        </li>
      <?php
      }
    }
  } else {
    $tree = taxon_description_taxonomy_get_tree($vid, 'en');
    $cnt = 0;
    foreach ($tree as $i){
    if($i->language == "en") {
    $tid = $i->tid;
    $name = $i->name;
    $description = ($i->description) ? (strlen($i->description) > 75) ? substr($i->description,0,75) . '...' : $i->description : '';
    $parent = $i->parents[0];
      if($parent == 0) {
        if($cnt == 0) { ?>
          <li class="ui-state-default ui-state-stable stable" id="tid_<?php echo $tid; ?>" rel="<?php echo $tid; ?>">
            <div class="column1"></div>
            <div class="column2"><strong><?php echo $name; $cnt = 1; ?></strong></div>
            <div class="column3" align="right"><a href="#" onclick="javascript:TD.addChapter(<?php print $tid; ?>);return false;"><?php print t('add'); ?></a></div>
          </li>
          <li class="ui-state-hidden" id="hidden">
            <div class="column1"><img src="<?php echo $base_url.'/'.$module_path.'images/move.png' ?>" alt="<?php print t('drag up or down') ?>" /></div>
            <div class="column2"><?php $cnt = 1; ?><span class="editable"></span></div>
            <div class="column4" id="removeme-<?php print $tid; ?>" align="right"><a href="#" class="delete" onclick="javascript:TD.deleteChapter($(this));return false;"><?php print t('delete'); ?></a></div>
            <div class="column5" id="editme-<?php print $tid; ?>" align="right"><a href="#" class="edit" onclick="javascript:TD.editChapter($(this));return false;"><?php print t('edit'); ?></a></div>
          </li>
        <?php } else { ?>
          <li class="ui-state-default ui-state-disabled" id="tid_<?php echo $tid; ?>">
            <div class="column1"></div>
            <div class="column2"><strong><?php echo $name; $cnt = 1; ?></strong></div>
            <div class="column3" align="right"><a href="#" onclick="javascript:TD.addChapter(<?php print $tid; ?>);return false;"><?php print t('add'); ?></a></div>
          </li>
        <?php }
      }	else { ?>
        <li class="ui-state-default" id="tid_<?php echo $tid; ?>">
          <div class="column1"><img src="<?php echo $base_url.'/'.$module_path.'images/move.png' ?>" alt="<?php print t('drag up or down'); ?>" /></div>
          <div class="column2">
	        <span class="editable">
            <?php
              echo '<strong>' . $name . '</strong>';
              if($description) echo ': ' . $description;
              $cnt = 1;
            ?>
            </span></div>
          <div class="column4" id="removeme-<?php print $tid; ?>" align="right"><a href="#" class="delete" onclick="javascript:TD.deleteChapter($(this));return false;"><?php print t('delete'); ?></a></div>
          <div class="column5" id="editme-<?php print $tid; ?>" align="right"><a href="#" class="edit" onclick="javascript:TD.editChapter($(this));return false;"><?php print t('edit'); ?></a></div>
        </li>
      <?php }
    }
    }
  }?>
  </ul>
</div>
