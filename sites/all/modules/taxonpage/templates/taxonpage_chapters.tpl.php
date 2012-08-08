<?php

/**
 * @file taxonpage_chapters.tpl.php
 * Display the chapters and their contents on a taxon page
 *
 * Variables available:
 * - $chapters: An array of chapters and their data
 */

if(!empty($chapters)) {
global $user;

$comment_link = false;
if(user_access('access comments')) {
 $comment_link = true;
}

?>

<div class="taxon_description">

<?php
foreach($chapters as $parent_chapter => $parents) {
    foreach($parents as $child_chapter => $children) {
      $show_chapter = FALSE;
      if(gettype($children) == "Array") {
        foreach($children as $child) {
          if($child['status'] == 1 || user_access('edit any taxon description') || (user_access('edit own taxon description') && $user->uid == $child['uid'])) {
            $has_content = TRUE;
          }
        }
      }
    }
    
    if($has_content) {
    $overview_class = ($parents['term_en'] == 'overview') ? " class=\"taxonpage_overview\"" : "";
?>
<div<?php print $overview_class; ?>>
<h2 class="taxonpage"><?php print filter_xss($parent_chapter); ?></h2>
   <?php
   foreach($parents as $child_chapter => $children) {
     $show_chapter = FALSE;   
     foreach($children as $child) {
       if($child['status'] == 1 || user_access('administer nodes') || (user_access('edit own taxon description') && $user->uid == $child['uid'])) {
         $show_chapter = TRUE;
       }    
     }
     if($show_chapter) {
   ?>
   <div class="sub-chapter">
   <h3 class="taxonpage"><?php print filter_xss($child_chapter); ?></h3>
     <?php
     foreach($children as $child) {
       switch ($child['status']) {
         case 0:
           if(user_access('administer nodes') || (user_access('edit own taxon description') && $user->uid == $child['uid'])) {
           ?>
         <div class="chapter-content draft">
           <?php print '<div class="taxonpage_draft">' . t('Unpublished') . '</div>' . check_markup($child['body'],FILTER_FORMAT_DEFAULT); ?>
            <?php if(user_access('edit any taxon description') || (user_access('edit own taxon description') && $user->uid == $child['uid'])) { ?>
              <ul class="taxonpage_inline_links">
              <li class="edit_chapter"><?php print l(t('Edit'),'node/' . $child['nid'] . '/edit',array('query' => array('chapter' => $child_chapter))); ?></li>
              </ul>
             <?php
           }
             ?>
             <div class="chapter-author">
               <?php 
                 if(variable_get("taxonpage_show_license",FALSE) != FALSE) { ?>
                 <?php print '<span class="taxonpage_author">' . t('Author(s)') . ': ' . filter_xss($child['authors']) . '</span>'; ?>
                 <div class="taxonpage_rightsholder_wrapper">
                 <?php print $child['license_img']; ?>
                 <?php
                   if ($child['cc_license'] != 'publicdomain') {
                     $rgtholders = ($child['rightsholders']) ? filter_xss($child['rightsholders']) : filter_xss($child['authors']);
                     print '<span class="taxonpage_rightsholder">' . t('Rights holder(s)') . ': ' . $rgtholders . '</span>';
                   }
                 ?>
                 </div>
                 <?php
                 }
                 ?>
                 <ul class="taxonpage_inline_links">
                 <?php 
                   if($child['taxon_tid']) { ?>
                     <li class="taxonpage_relations_content"><?php print '<b>' . t('Submitted as:') . '</b> ' . $child['taxon_name']; ?></li>
                 <?php     
                   }
                 ?>
                 <?php $comment_link ? print '<li class="comment_balloon">' . l(t('Comment'),'node/' . $child['nid']) . ' (' . $child['comment_count'] . ')</li>': "" ?>
                 </ul>
            </div>
         </div>
         <?php
           }
         break;
         case 1:
         ?>
         <div class="chapter-content">
           <?php print check_markup($child['body'],FILTER_FORMAT_DEFAULT); ?>
            <?php if(user_access('edit any taxon description') || (user_access('edit own taxon description') && $user->uid == $child['uid'])) { ?>
                <ul class="taxonpage_inline_links">
              <li class="edit_chapter"><?php print l(t('Edit'),'node/' . $child['nid'] . '/edit',array('query' => array('chapter' => $child_chapter))); ?></li>
        </ul>
             <?php
           }
             ?>
             <div class="chapter-author">
               <?php 
                 if(variable_get("taxonpage_show_license",FALSE) != FALSE) { ?>
                 <?php print '<span class="taxonpage_author">' . t('Author(s)') . ': ' . filter_xss($child['authors']) . '</span>'; ?>
                 <div class="taxonpage_rightsholder_wrapper">
                 <?php print $child['license_img']; ?>
                 <?php
                   if ($child['cc_license'] != 'publicdomain') {
                     $rgtholders = ($child['rightsholders']) ? filter_xss($child['rightsholders']) : filter_xss($child['authors']);
                     print '<span class="taxonpage_rightsholder">' . t('Rights holder(s)') . ': ' . $rgtholders . '</span>';
                   }
                 ?>
                 </div>
                 <?php
                 }
                 ?>
                 <ul class="taxonpage_inline_links">
                     <?php 
                       if($child['taxon_tid']) { ?>
                         <li class="taxonpage_relations_content"><?php print t('Submitted as') . ' <span>' . $child['taxon_name'] . '</span>'; ?></li>
                     <?php     
                       }
                     ?>
                 <?php $comment_link ? print '<li class="comment_balloon">' . l(t('Comment'),'node/' . $child['nid']) . ' (' . $child['comment_count'] . ')</li>': "" ?>
                 </ul>
            </div>
         </div>
        <?php
         break; 
       }
       
     ?>

     <?php
     }
     ?>
     </div>
<?php
}
}
?>
</div>
<?php
$has_content = FALSE;
}
}
?>
</div>

<?php
}
?>