<?php

function theme_taxon_description_contributors($form) {
  $rows = array();
  $headers = array('', t('Name'), array('data' => t('Weight'), 'class' => 'js-hide'));
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
function taxon_description_load_contributors($sid) {
  $contributors = array();
  $query = "SELECT tdcd.name AS name, tdc.cntr_id AS cntr_id, tdc.rank AS rank FROM {taxon_description_contributor} tdc
      INNER JOIN {taxon_description_contributor_data} tdcd ON tdc.cntr_id=tdcd.cntr_id
      WHERE tdc.sid=%d
      ORDER BY tdc.rank ASC"; // do not change order of presentation

  $result = db_query($query, $sid);
  while ($contributor = db_fetch_array($result)) {
    $contributors[] = $contributor;
  }
  return $contributors;
}

function taxon_description_insert_contributors($contributors, $nid, $sid) {
  if (empty ($contributors)) return true;
  return taxon_description_save_contributors($contributors, $nid, $sid);
}

function taxon_description_save_contributor(&$author) {
  return drupal_write_record('taxon_description_contributor_data', $author);
}

/**
 * Save contributors to the database
 * @param $authors
 * @param $nid
 * @param $sid
 * @param $update
 * @return success of database operations
 */
function taxon_description_save_contributors(&$contributors, $nid, $sid) {
  db_query('DELETE FROM {taxon_description_contributor} WHERE nid = %d AND sid = %d', $nid, $sid);
  foreach ($contributors as $cat => $authors) {
      if($authors['name']) {
        $author = taxon_description_parse_author($contributors[$cat]);
        if (empty($author['cntr_id'])) {
          taxon_description_save_contributor($author);
        }
        else {
          $author['rank'] = $authors['rank'];
        }

        $link_array = array(
          'nid' => $nid,
          'cntr_id' => $author['cntr_id'],
          'rank' => ((isset($author['rank']) && is_numeric($author['rank'])) ? $author['rank'] : $cat),
          'sid' => $sid
        );

        if ($sid) if (!drupal_write_record('taxon_description_contributor', $link_array)) return false;
      }
  }
  return true; // successfully saved all contributors
}

function taxon_description_parse_author($author_array) {
  $author_exists = db_result(db_query("SELECT cntr_id FROM {taxon_description_contributor_data} WHERE name = '%s'", trim($author_array['name'])));
  if ($author_exists){
    $author_array = db_fetch_array(db_query("SELECT * FROM {taxon_description_contributor_data} WHERE name = '%s' LIMIT 1", trim($author_array['name'])));
  } else {
	unset($author_array['cntr_id']); //necessary in event author name is replaced on form with another that does not yet exit
    $value = trim($author_array['name']);
    $appellation = $prefix = $surname = $firstname = $initials = '';
    $prefix = "";
    $value = preg_replace("/\s{2,}/", ' ', $value); // replace multiple white space by single space
    $author = explode(",", $value);
    $size = sizeof($author);
    // No commas therefore something like Mark Grimshaw, Mark Nicholas Grimshaw, M N Grimshaw, Mark N. Grimshaw
    if ($size == 1) {
      // Is complete surname enclosed in {...}, unless the string starts with a backslash (\) because then it is
      // probably a special latex-sign..
      // 2006.02.11 DR: in the last case, any NESTED curly braces should also be taken into account! so second
      // clause rules out things such as author="a{\"{o}}"
      //
      if (preg_match("/(.*){([^\\\].*)}/", $value, $matches) && !(preg_match("/(.*){\\\.{.*}.*}/", $value, $matches2))) {
        $author = explode(" ", $matches[1]);
        $surname = $matches[2];
      } else {
        $author = explode(" ", $value);
        // last of array is surname (no prefix if entered correctly)
        $surname = array_pop($author);
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
      $appellation = implode(' ', array_splice($author, 1, 1));
      // first of array is surname (perhaps with prefix)
      list ($surname, $prefix) = _taxon_description_grabSurname(array_shift($author));
    }
    $remainder = implode(" ", $author);
    list ($firstname, $initials, $prefix2) = _taxon_description_grabFirstnameInitials($remainder);
    if (!empty ($prefix2))
    $prefix .= $prefix2;
    //var_dump($prefix);
    //$surname = $surname . ' ' . $appellation;
    $author_array['firstname'] = trim($firstname);
    $author_array['initials'] = trim($initials);
    $author_array['lastname'] = trim($surname);
    $author_array['prefix'] = trim($prefix);
    $author_array['suffix'] = trim($appellation);
  }

  return $author_array;
}