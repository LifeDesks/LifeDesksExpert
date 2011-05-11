<?php

/**
 * Taxon Export chapter definitions page
 */
 
require_once("taxon_export.export.inc");
global $base_url;
function taxon_export_instructions() {
  $module_path = drupal_get_path('module', 'taxon_export') .'/';
  drupal_add_css($module_path .'css/taxon_export.css');

$output = '<div class="messages status">' . t('1. What is this?') . '</div>';

$output .= '<p>This is specialized export interface which will let Encyclopedia of Life (and, potentionally, your other harvesting partners) to harvest the content of your LifeDesk.</p>';

$output .= '<div class="messages status">' . t('2. Why do I have to do export my content?') . '</div>';

$output .= '<p>If you decide to share the information from your LifeDesk with Encyclopedia of Life, you must either <a href="' . $base_url . '/admin/export/taxon_export/export">make endpoint manually</a>, or initiate automatic <a href="' . $base_url . '/admin/export/taxon_export/schedule">schedule</a>.</p>';

$output .= '<div class="messages status">' . t('3. What content is transferred?') . '</div>';

$output .= '<p>Textual descriptions (as separate chapters), images and bibliography items, in association with taxon names from main classification. If the content belongs to vernacular name or synonym, it will be transferred with a "parent" valid scientific name.</p>';

$output .= '<div class="messages status">' . t('4. Is draft content transferred?') . '</div>';

$output .= '<p>No. Only published content will be transferred.</p>';

$output .= '<div class="messages status">' . t('5. Is my classification transferred? What if my taxa are not in Encyclopedia of Life?') . '</div>';

$output .= '<p>The classification will be transfered only if the taxa are associated with a content. If the given taxon has no content, it will not appear in the exported file. Please also note that for exporting classification itself (without content), you may want to use <a href="' . $base_url . '/classification/export">our special tool</a>. If your taxa are not in EOL, they will finally (but not immediately) appear there after Encyclopedia of life will merge your classification.</p>';

$output .= '<div class="messages status">' . t('6. Will my image files be exported as well?') . '</div>';

$output .= '<p>No. The endpoint will contain only textual links to your image files locations inside LifeDesk.</p>';

$output .= '<div class="messages status">' . t('7. Will my content be immediately flagged as vetted (or trusted)?') . '</div>';

$output .= '<p>This is a matter of agreement with Encyclopedia of Life.</p>';

$output .= '<div class="messages status">' . t('8. How often will my endpoint be harvested by Encyclopedia of Life?') . '</div>';

$output .= '<p>The LifeDesk can provide you with the one end of transfer process: your content will be exported as often as you want. However, it is due to Encyclopedia of Life to decide how often your content will be imported into EOL.</p>';

$output .= '<div class="messages status">' . t('9. Can other organizations besides Encyclopedia of Life harvest the content from my LifeDesk?') . '</div>';

$output .= '<p>Yes. General-purpose search engines like Google will harvest your site. You can also inform your specific harvesting partners and provide them with a <a href="' . $base_url . '/files/' . taxon_export_get_lifedesk_name() . '/taxon_export/' . taxon_export_get_lifedesk_name() . '_last_taxonexport.xml.gz">following permanent link</a> (please save this link with "Copy Link location" context menu item of your browser; please also note that if you did not export anything yet, this link would not work).</p>';

$output .= '<div class="messages status">' . t('10. Will all the appropriate attribution be preserved?') . '</div>';

$output .= '<p>All attribution specified in <a href="http://www.eol.org/files/pdfs/docs/EOL_Transfer_Schema_Documentation.pdf">Encyclopedia of Life Taxon Resource Transfer Schema</a> will be preserved.</p>';

return $output;
}