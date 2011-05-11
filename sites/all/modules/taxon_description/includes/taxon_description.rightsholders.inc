<?php

function theme_taxon_description_rightsholders($form) {
  $rows = array();
  $headers = array('', t('Name or institution'), array('data' => t('Weight'), 'class' => 'js-hide'));
  drupal_add_tabledrag($form['#id'], 'order', 'sibling', 'rank', NULL, NULL, TRUE);

  foreach (element_children($form) as $key) {
    // No need to print the field title every time.
    unset ($form[$key]['name']['#title']);
    // Add class to group weight fields for drag and drop.
    $form[$key]['rank']['#attributes']['class'] = 'rank';

    // Build the table row.
    $row = array('');
    $row[] = array('data' => drupal_render($form[$key]['name']));
    $row[] = array('data' => drupal_render($form[$key]['rank']), 'class' => 'js-hide');
    $rows[] = array('data' => $row, 'class' => 'draggable');
  }
  $output = theme('table', $headers, $rows, array('id' => $form['#id']));
  $output .= drupal_render($form);
  return $output;
}

/**
 * @param $sid
 * @return unknown_type
 */
function taxon_description_load_rightsholders($sid) {
  $rightsholders = array();
  $query = "SELECT tdrd.name AS name, tdr.rts_id AS rts_id, tdr.rank AS rank FROM {taxon_description_rightsholder} tdr
      INNER JOIN {taxon_description_rightsholder_data} tdrd ON tdr.rts_id=tdrd.rts_id
      WHERE tdr.sid=%d
      ORDER BY tdr.rank ASC"; // do not change order of presentation

  $result = db_query($query, $sid);
  while ($rightsholder = db_fetch_array($result)) {
    $rightsholders[] = $rightsholder;
  }
  return $rightsholders;
}

function taxon_description_insert_rightsholders($rightsholders, $nid, $sid) {
  if (empty ($rightsholders)) return true;
  return taxon_description_save_rightsholders($rightsholders, $nid, $sid);
}

function taxon_description_save_rightsholder(&$rightsholder) {
  return drupal_write_record('taxon_description_rightsholder_data', $rightsholder);
}

/**
 * Save rightsholders to the database
 * @param $rightsholders
 * @param $nid
 * @param $sid
 * @param $update
 * @return success of database operations
 */
function taxon_description_save_rightsholders(&$rightsholders, $nid, $sid) {
  db_query('DELETE FROM {taxon_description_rightsholder} WHERE nid = %d AND sid = %d', $nid, $sid);
  foreach ($rightsholders as $cat => $rightsholder) {
      if($rightsholder['name']) {
        $name = taxon_description_parse_rightsholder($rightsholders[$cat]);
        if (empty($name['rts_id'])) {
          taxon_description_save_rightsholder($name);
        }
        else {
          $name['rank'] = $rightsholder['rank'];
        }

        $link_array = array(
          'nid' => $nid,
          'rts_id' => $name['rts_id'],
          'rank' => ((isset($name['rank']) && is_numeric($name['rank'])) ? $name['rank'] : $cat),
          'sid' => $sid
        );

        if ($sid) if (!drupal_write_record('taxon_description_rightsholder', $link_array)) return false;
      }
  }
  return true; // successfully saved all rightsholders
}

function taxon_description_parse_rightsholder($rightsholder_array) {
  $rightsholder_exists = db_result(db_query("SELECT rts_id FROM {taxon_description_rightsholder_data} WHERE name = '%s'", trim($rightsholder_array['name'])));
  if ($rightsholder_exists){
    $rightsholder_array = db_fetch_array(db_query("SELECT * FROM {taxon_description_rightsholder_data} WHERE name = '%s' LIMIT 1", trim($rightsholder_array['name'])));
  } else {
   unset($rightsholder_array['rts_id']); //necessary in event author name is replaced on form with another that does not yet exit
    $value = trim($rightsholder_array['name']);
    $appellation = $prefix = $surname = $firstname = $initials = '';
    $prefix = "";
    $value = preg_replace("/\s{2,}/", ' ', $value); // replace multiple white space by single space
    $rightsholder = explode(",", $value);
    $size = sizeof($rightsholder);
    // No commas therefore something like Mark Grimshaw, Mark Nicholas Grimshaw, M N Grimshaw, Mark N. Grimshaw
    if ($size == 1) {
      // Is complete surname enclosed in {...}, unless the string starts with a backslash (\) because then it is
      // probably a special latex-sign..
      // 2006.02.11 DR: in the last case, any NESTED curly braces should also be taken into account! so second
      // clause rules out things such as author="a{\"{o}}"
      //
      if (preg_match("/(.*){([^\\\].*)}/", $value, $matches) && !(preg_match("/(.*){\\\.{.*}.*}/", $value, $matches2))) {
        $rightsholder = explode(" ", $matches[1]);
        $surname = $matches[2];
      } else {
        $rightsholder = explode(" ", $value);
        // last of array is surname (no prefix if entered correctly)
        $surname = array_pop($rightsholder);
      }
    }
    // Something like Grimshaw, Mark or Grimshaw, Mark Nicholas  or Grimshaw, M N or Grimshaw, Mark N.
    else
    if ($size == 2) {
      // first of array is surname (perhaps with prefix)
      list ($surname, $prefix) = _taxon_description_grabSurname(array_shift($author));
    }
    // If $size is 3, we're looking at something like Bush, Jr. III, George W
    else {
      // middle of array is 'Jr.', 'IV' etc.
      $appellation = implode(' ', array_splice($rightsholder, 1, 1));
      // first of array is surname (perhaps with prefix)
      list ($surname, $prefix) = _taxon_description_grabSurname(array_shift($rightsholder));
    }
    $remainder = implode(" ", $rightsholder);
    list ($firstname, $initials, $prefix2) = _taxon_description_grabFirstnameInitials($remainder);
    if (!empty ($prefix2))
    $prefix .= $prefix2;
    //var_dump($prefix);
    //$surname = $surname . ' ' . $appellation;
    $rightsholder_array['firstname'] = trim($firstname);
    $rightsholder_array['initials'] = trim($initials);
    $rightsholder_array['lastname'] = trim($surname);
    $rightsholder_array['prefix'] = trim($prefix);
    $rightsholder_array['suffix'] = trim($appellation);
  }

  return $rightsholder_array;
}