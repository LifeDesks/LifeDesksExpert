<?php
// $Id: biblio_handler_filter_contributor_lastname.inc,v 1.1.2.1 2009/12/04 19:41:04 rjerome Exp $
/**
 * Filter handler for contributors
 */
class biblio_handler_filter_contributor_lastname extends views_handler_filter_many_to_one {
  function get_value_options() {
    $result = db_query("SELECT lastname, firstname, initials, cid
              FROM {biblio_contributor_data} cd
                  ORDER by lastname, firstname");
    $this->value_options = array();
    while ($row = db_fetch_object($result)) {
      $this->value_options[$row->lastname] =  "$row->lastname, $row->firstname $row->initials";
    }
  }
}
