<?php
/**
 * @file taxon_description_chapter.tpl.php
 * Display chapter content along with author & right holder information
 *
 * Variables available:
 * - $chapters
 * - $cc_license: The license chosen for content
 * - $authors: The contributing authors
 * - $rightsholders: The rights holders
 */

  global $base_url;

  $module_path = drupal_get_path('module', 'taxon_description') .'/';
  drupal_add_css($module_path .'css/taxon_description.css');
  $license = '<img src="/' . drupal_get_path('module', 'creativecommons_lite') . '/images/buttons_small/' . $cc_license . '.png" alt="cc-' . $cc_license . '">';

?>
<?php print $chapters; ?>
<?php print $authors; ?>
<div id="taxon_description_cc_license"><?php print $license; ?></div>
<?php print $rightsholders; ?>
