/**
 *       We would like your feedback, especially if you uncover a bug or have made adjustments to this module that would benefit both our projects.
 *
 *       Please consider dropping us a line at feedback@lifedesks.org
 */

IMPORTANT!

If there is intention to use the db_switch function in a cron hook, the module with said cron hook must have the greatest weight in the system table AND there must ONLY BE ONE cron hook executing this function.

Usage:

/**************************************************************/
$sql = 'SELECT ds.shortname FROM {drupal_site} ds';
$result = db_query($sql);

while ($drupal_site = db_fetch_object($result)) {
  //switch connection to child site
  $switch_db = db_switch($drupal_site->shortname);

  /*
  do stuff in the switched database. Note however that there are many Drupal hook functions that should NOT EVER be used because of the bootstrap process:
  drupal_rebuild_theme_registry();
  node_types_rebuild();
  menu_rebuild();
  drupal_flush_all_caches();
  module_rebuild_cache();
  
  Also note that variable_get will not work with a switched connection like this so a db_switch_variable_get function was added.
  */

  //switch back to original database (required!)
  $switch_db = db_switch();
}
/**************************************************************/