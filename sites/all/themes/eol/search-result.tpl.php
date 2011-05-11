<?php
// $Id: search-result.tpl.php,v 1.1.2.1 2008/08/28 08:21:44 dries Exp $

/**
 * @file search-result.tpl.php
 * Default theme implementation for displaying a single search result.
 *
 * This template renders a single search result and is collected into
 * search-results.tpl.php. This and the parent template are
 * dependent to one another sharing the markup for definition lists.
 *
 * Available variables:
 * - $url: URL of the result.
 * - $title: Title of the result.
 * - $snippet: A small preview of the result. Does not apply to user searches.
 * - $info: String of all the meta information ready for print. Does not apply
 *   to user searches.
 * - $info_split: Contains same data as $info, split into a keyed array.
 * - $type: The type of search, e.g., "node" or "user".
 *
 * Default keys within $info_split:
 * - $info_split['type']: Node type.
 * - $info_split['user']: Author of the node linked to users profile. Depends
 *   on permission.
 * - $info_split['date']: Last update of the node. Short formatted.
 * - $info_split['comment']: Number of comments output as "% comments", %
 *   being the count. Depends on comment.module.
 * - $info_split['upload']: Number of attachments output as "% attachments", %
 *   being the count. Depends on upload.module.
 *
 * Since $info_split is keyed, a direct print of the item is possible.
 * This array does not apply to user searches so it is recommended to check
 * for their existance before printing. The default keys of 'type', 'user' and
 * 'date' always exist for node searches. Modules may provide other data.
 *
 *   <?php if (isset($info_split['comment'])) : ?>
 *     <span class="info-comment">
 *       <?php print $info_split['comment']; ?>
 *     </span>
 *   <?php endif; ?>
 *
 * To check for all available data within $info_split, use the code below.
 *
 *   <?php print '<pre>'. check_plain(print_r($info_split, 1)) .'</pre>'; ?>
 *
 * @see template_preprocess_search_result()
 */

global $base_url;

module_load_include('php', 'classification', 'includes/classification_functions.class');

$names = new Names;

$hit = true;

switch($result['type']) {
  case t('Taxon Page'):
    $primary = variable_get('classification_primary',0);
    $tid = db_result(db_query("SELECT tn.tid FROM {term_node} tn INNER JOIN {node} n ON (tn.nid=n.nid) INNER JOIN {term_data} td ON (tn.tid = td.tid) WHERE tn.nid=%d AND td.vid=%d AND n.type='taxon_description'", $result['node']->nid, $primary));
    if(!$tid) $hit = false;
    $italicize = classification_check_italicize($tid);
    $result['title'] = ($italicize) ? $names->italicized_form($result['title']) : $result['title'];
    $result['link'] = $base_url . '/pages/' . $tid;
  break;
}
?>

<?php if($hit) : ?>
<dt class="title <?php print strtolower(str_replace(" ", "", $result['type'])); ?>">
  <a href="<?php print $result['link']; ?>"><?php print $result['title']; ?></a>
</dt>
<dd>
    <?php if($result['snippet']) : ?>
      <p class="search-snippet"><?php print $result['snippet']; ?></p>
    <?php endif; ?>
	<?php if($result['user']) : ?>
	  <p class="search-info"><?php print $result['link']; ?> [<?php print t('author:'); ?> <?php print $result['user']; ?>, <?php print t('created:'); ?> <?php print gmdate("Y-m-d\TH:i:s\Z", $result['date']); ?>]</p>
	<?php endif; ?>
</dd>
<?php endif; ?>

