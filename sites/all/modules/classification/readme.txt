/**
 * README for classification module
 */

Modifications to core taxonomy.module (see the taxonomy.patch file):

taxonomy_save_term by default deletes records in the term_relation table then recreates new rows (with new trid primary keys since this is an autoincrement column). Because we extend the term_relation table with term_relation_has_type, deleting records in the term_relation table will break the relationship metadata. Thus, the taxonomy_save_term function in the taxonomy module must be adjusted to remove that offending section:

Comment out the following:

db_query('DELETE FROM {term_relation} WHERE tid1 = %d OR tid2 = %d', $form_values['tid'], $form_values['tid']);
if (!empty($form_values['relations'])) {
  foreach ($form_values['relations'] as $related_id) {
    if ($related_id != 0) {
      db_query('INSERT INTO {term_relation} (tid1, tid2) VALUES (%d, %d)', $form_values['tid'], $related_id);
    }
  }
}

Plus, the classification module uses taxonomy_override_selector (i.e. sets this variable to TRUE) such that relationships are not loaded up on term edit pages. With the above modification, loading the related terms into memory would be futile.

line 490 (appprox):

function taxonomy_form_alter(&$form, $form_state, $form_id) {
  if (isset($form['type']) && isset($form['#node']) && (!variable_get('taxonomy_override_selector', FALSE)) && $form['type']['#value'] .'_node_form' == $form_id) {

Needs to be changed to:

function taxonomy_form_alter(&$form, $form_state, $form_id) {
  if (isset($form['type']) && isset($form['#node']) && $form['type']['#value'] .'_node_form' == $form_id) {

REQUIREMENTS:

For real-time collaborative classification editing (if desired):

Installation of the redis.so PHP extension available here: https://github.com/nicolasff/phpredis
Install a juggernaut server using node.js as outlined here: https://github.com/maccman/juggernaut




