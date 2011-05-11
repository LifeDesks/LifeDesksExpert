<?php

/**
 * Helper function to save the file, compressed by default into tar.gz format.
 * @param $file Drupal file object
 * @param $compress Boolean, whether to compress file or not, default TRUE
 * @return Boolean, TRUE if successful, FALSE if failed.
 */
function _classification_import_file_save_file($file, $status = NULL, $destination = NULL, $compress = TRUE, $replace = FILE_EXISTS_RENAME) {
  
  global $user;
  
  $current_filepath = $file->filepath;
  $current_filename = file_check_path($current_filepath);
  if (empty($destination)) {
    $destination = file_create_path('classification_import') . '/files';
    // If a failed check then store in separate folder (and don't write to files table later).
    if (isset($status['checked']['success']) && $status['checked']['success'] === FALSE) {
      $destination .= '/failed_check';
    }
  }
  
  // Build new file object.
  $new_file = new stdClass();
  $filename_suffix = '';
  // Check for possible partial imports.
  if (isset($status['inserted']['success']) && $status['inserted']['success'] === FALSE) {
    $filename_suffix = '_FAILED_IMPORT';
    if (isset($status['checked']['partial']) && $status['checked']['partial'] === TRUE) {
      $filename_suffix = '_PARTIAL_IMPORT'; 
    }
  }
  if ($compress === TRUE) {
    // Generate new filename, tar and gz file in its current location, will move it later.
    $new_file->filename = gmdate("Y_m_d_G-i-s") . '_uid' . $user->uid . $filename_suffix .'.tar.gz';
    // Since this is going to be a unique filename we override $replace, setting it to replace, to prevent renaming of file when moved to destination.
    $replace = FILE_EXISTS_REPLACE;
    $new_file->filepath = $current_filepath .'/' . $new_file->filename;
    $command = "tar cfz $new_file->filepath -C $current_filepath/ $current_filename";
    // Run command to tar and gz existing file.
    $response = _classification_import_proc($command);
    if ($response['code'] > 0) {
      drupal_set_message(t('Could not compress and copy %file_name, error received: %error_message', array('%file_name' => $file->filename, '%error_message' => $response['message'])), 'error', FALSE);
      return FALSE;
    }
    $new_file->filemime = file_get_mimetype($new_file->filename);
    $new_file->filesize = filesize($new_file->filepath);
  }
  else {
    // No compression so just pass values directly from existing file object.
    $new_file = $file;
  }
  
  // Move file to permanent location, update new_file object and save record to files table.
  if (file_move($new_file, $destination, $replace)) {
    $new_file->destination = file_destination($destination .'/' . $new_file->filename, $replace);
    $new_file->filepath = $new_file->destination;
    $new_file->status = FILE_STATUS_PERMANENT;
    $new_file->timestamp = time();
    $new_file->uid = $user->uid;
    
    // Update new file details to files table, but only if file did not fail the import check.
    if (!isset($status['checked']['success']) || $status['checked']['success'] === TRUE) {
      if (drupal_write_record('files', $new_file)) {
        return TRUE;
      }
      else {
        drupal_set_message(t('%file_name was saved to !destination, but could not write file metadata to files table.', array('%file_name' => $file->filename, '!destination' => $destination)), 'error', FALSE);
        return FALSE;
      }
    }
  }
  else {
    drupal_set_message(t('Could not save %file_name to !destination', array('%file_name' => $file->filename, '!destination' => $destination)), 'error', FALSE);
    return FALSE;
  }
}

/**
 * Helper function to check file is in a correct format.
 * @param $file Drupal file object
 * @param $verbose boolean, TRUE for verbose feedback, FALSE for returning only brief metadata and critical errors.
 * @return array('success', 'feedback')
 */
function _classification_import_file_check($file, $verbose = TRUE) {
  
  $checked = array();
  
  // If file object is missing, return failure.
  if (empty($file)) {
    watchdog('classification_import', t('Variable $file is empty, and could not be checked in _classification_import_file_check.'), array(), WATCHDOG_ERROR);
    $checked['success'] = FALSE;
    $checked['feedback'] = t('An error occurred while processing your file and it could not be checked, please contact your site administrator if the problem persists.');
    return $checked;
  }
  
  // Convert file to PHPExcel object.
  $excel_obj = excel_reader_parse($file);
  
  if (!is_object($excel_obj)) {
    watchdog('classification_import', t('File could not be converted to an Excel object, and could not be checked in _classification_import_file_check.'), array(), WATCHDOG_ERROR);
    $checked['success'] = FALSE;
    $checked['feedback'] = t('An error occurred while processing your file and it could not be checked, please contact your site administrator if the problem persists.');
    return $checked;
  }
  
  // Increase PHP maximum execution time for the duration of this script.
  ini_set('max_execution_time', 360);
  
  // Check length of file. Return if length exceed limits.
  $rowcount = $excel_obj->getActiveSheet()->getHighestRow(); // Includes header row.
  if ($rowcount > variable_get('classification_import_file_limit', 15000) + 1) {
    $checked['success'] = FALSE;
    $checked['feedback'] = t('Sorry, there is a limit of !limit rows', array('!limit' => number_format(variable_get('classification_import_file_limit', 15000))));
    return $checked;
  }
  
  // Get info about the excel file.
  $excel_info = _classification_import_get_excel_info($excel_obj);
  $headers = $excel_info['header'];
  // Define header rows as list items for later display to user.
  $header_items = array();
  foreach ($headers as $column) {
    $header_items[] = array('data' => check_plain($column), 'class' => 'cell');
  }
  if ($verbose) { 
    // Define first five rows as list items for later display to user.
    $firstfive_items = array();
    foreach ($excel_info['firstfive'] as $i => $row) {
      $firstfive_cells = array();
      foreach ($row as $cell) {
        $class = 'cell';
        if (empty($cell) && !is_numeric($cell)) { // Can't use empty() alone as need to show values of 0.
          $cell = '<em>' . t('empty') .'</em>';
          $class .= ' empty';
        }
        else {
          $cell = check_plain($cell);
        }
        $firstfive_cells[] = array('data' => $cell, 'class' => $class);
      }
      $firstfive_items[] = array('data' => '', 'children' => $firstfive_cells, 'class' => 'row');
    }
  }
  
  // Generate feedback list items for display to user with regards to excel info.
  $feedback_items = array();
  $feedback_items[] = array('data' => t('File name: !filename', array('!filename' => $file->filename)), 'class' => 'stats');
  $feedback_items[] = array('data' => t('Total rows: !rowcount (excludes header)', array('!rowcount' => $excel_info['rowcount'] - 1)), 'class' => 'stats');
  $feedback_items[] = array('data' => t('Total columns: !colcount', array('!colcount' => $excel_info['colcount'])), 'class' => 'stats');
  
  
  // Determine what structure the file is in by checking for expected or unexpected (banned) columns. Expected file structures include 'parent-child', 'full hierarchy' and 'full hierarchy (alternate)'
  $expected_columns = array(
    FILE_STRUCTURE_PARENT_CHILD => array(t('Parent'), t('Child')),
    FILE_STRUCTURE_FULL_HIERARCHY_ALT => array(t('Taxon1'), t('Taxon2')),
  );
  $banned_columns = array(FILE_STRUCTURE_FULL_HIERARCHY => array(t('Rank')));
  foreach ($expected_columns as $structure => $columns) {
    if (_classification_import_file_check_columns($columns, $headers)) $excel_info['file_structure'] = $structure; // Expected columns exist in Excel file.
    $banned_columns[FILE_STRUCTURE_FULL_HIERARCHY] = array_merge($banned_columns[FILE_STRUCTURE_FULL_HIERARCHY], $columns); // Add columns that should not be included in the full hierarchy format.
  }

  // If format not assigned, do quick sanity check to make sure its not a badly/partially formed expected format, by making sure none of the expected columns from parent-child or full hierarchy alternate are found.
  if (empty($excel_info['file_structure']) && count(array_diff($banned_columns[FILE_STRUCTURE_FULL_HIERARCHY], $headers)) == count($banned_columns[FILE_STRUCTURE_FULL_HIERARCHY])) {
    // Not badly/partially formed so assume it is full hierarchy format.
    $excel_info['file_structure'] = FILE_STRUCTURE_FULL_HIERARCHY;
  }
  elseif (empty($excel_info['file_structure'])) {
    // If we get this far, file structure is unrecognised, it contains all required columns and some expected columns but not all, so can't assume it's a full hierarchy.
    // Accepted known columns. Can also have columns labelled with a rank term.
    $accepted_columns = array(t('Identifier'), t('Taxon1'), t('Taxon2'), t('Taxon3'), t('Taxon4'), t('Taxon5'), t('Taxon6'), t('Taxon7'), t('Taxon8'), t('Taxon9'), t('Taxon10'), t('Taxon11'), t('Taxon12'), t('Taxon13'), t('Taxon14'), t('Taxon15'), t('Taxon16'), t('Taxon17'), t('Taxon18'), t('Taxon19'), t('Taxon20'), t('Parent'), t('Child'), t('Rank'), t('Vernaculars'), t('VernacularsLanguage'), t('Synonyms'), t('Synonym'), t('Description'));
   
    // List header rows for user to examine, highlighting unrecognised headers.
    foreach ($header_items as $i => $column) {
      if (!in_array($column['data'], $accepted_columns)) $header_items[$i]['class'] .= ' error';
    }
    $feedback_items[] = array('data' => t('<strong>Header row (required):</strong>'), 'children' => $header_items);
    if ($verbose) $feedback_items[] = array('data' => t('<strong>First five rows:</strong>'), 'children' => $firstfive_items);
    $feedback_items[] = array('data' => t('File structure: unrecognized. Try downloading the example Excel files and pay particular attention to the inline comments.'), 'class' => 'stats failed'); 
    // Return check as failure, unrecognised file structure.
    $checked['success'] = FALSE;
    $checked['feedback'] = theme_item_list($feedback_items, NULL, 'ul', array('class' => 'import_parse_message'));
    return $checked;
  }
  
  // If we've got this far then file structure should have been determined, so add it to feedback items. Also add headers.
  $feedback_items[] = array('data' => t('File structure: !file_structure', array('!file_structure' => $excel_info['file_structure'])), 'class' => 'stats');
  $feedback_items[] = array('data' => t('<strong>Header row (required):</strong>'), 'children' => $header_items);
  if ($verbose) $feedback_items[] = array('data' => t('<strong>First five rows:</strong>'), 'children' => $firstfive_items);

  // Check to see if a synonym column exists in headers as save for later use, can be either "Synonyms" or "Synonym".
  foreach (array(t('Synonyms'), t('Synonym')) as $column) {
    if (_classification_import_file_check_columns($column, $headers)) {
      $excel_info['synonym_column'] = $column;
      break;
    }
  }
  
  // Get data values from PHPExcel object.
  // TODO: Adjust _classification_import_build_content to return array that closer matches what we need for taxa array, to optimise code.
  $data = _classification_import_build_content($excel_obj);
  
  // Run some data validation checks on known file structures.
  $missing_columns = array();
  switch ($excel_info['file_structure']) {
    
    case FILE_STRUCTURE_PARENT_CHILD:
      
      // Check for required columns.
      $missing_columns = _classification_import_file_check_columns(array(t('Identifier')), $headers, TRUE, TRUE);
      
      // Check that all values in Identifier and Parent columns are integers.
      $id_values = array();
      foreach ($data as $row) {
        foreach ($row as $column => $cell) {
          switch ($column) {
            case t('Identifier'):
              if (($cell['type'] !== 'n' && $cell['type'] !== 'f') || in_array($cell['value'], $id_values)) {
                $feedback_items[] = array('data' => t('For each data row, the Identifier column must contain a unique integer.', array('%identifier' => $row[t('Identifier')]['value'])), 'class' => 'error');
                $checked['success'] = FALSE;
              }
              $id_values[] = $cell['value'];
              break;
            case t('Parent'):
              if ($cell['type'] !== 'n') {
                $feedback_items[] = array('data' => t('For each data row, the Parent column must contain an integer, and cannot contain formulae. The first offending row has the Identifier %identifier.', array('%identifier' => $row[t('Identifier')]['value'])), 'class' => 'error');
                $checked['success'] = FALSE;
              }
              break;
          }
        }
      }
      unset($id_values);
      
    break;
    
    
    case FILE_STRUCTURE_FULL_HIERARCHY:
    case FILE_STRUCTURE_FULL_HIERARCHY_ALT:
      
      // Check for required synonym column.
      if (empty($excel_info['synonym_column'])) $missing_columns[] = t('Synonyms');
      
    break;
    
  }
  
  // Check for missing required columns, return check failed if required columns are missing.
  if (count($missing_columns) > 0) {
    // Add feedback item regarding missing columns then return check as failed.
    $feedback_items[] = array('data' => t('<strong>Header row (required):</strong>'), 'children' => $header_items);
    $feedback_items[] = array('data' => format_plural(count($missing_columns), 'Missing required column: !missing_cols', 'Missing required columns: !missing_cols', array('!missing_cols' => implode(', ', $missing_columns))), 'class' => 'stats failed');
    $checked['success'] = FALSE;
    $checked['feedback'] = theme_item_list($feedback_items, NULL, 'ul', array('class' => 'import_parse_message'));
    return $checked;
  }
  
  
  // If we've got this far all checks have been done so return feedback.
  // If failure not explicity set previously, then assume we have passed all checks, and return data.
  if (!isset($checked['success'])) {
    $checked['success'] = TRUE;
    $checked['data'] = array('excel_info' => $excel_info, 'content' => $data);
    $feedback_items[] = array('data' => t('Will be parsed as %structure.', array('%structure' => $excel_info['file_structure'])), 'class' => 'stats success');
  }
  $checked['feedback'] = theme_item_list($feedback_items, NULL, 'ul', array('class' => 'import_parse_message'));
  return $checked;
  
}
/**
 * Helper function to compare subset of columns with header in the file.
 * @param $columns
 * @param $header
 * @param $exist
 * @param $difference
 * @return unknown_type
 */
function _classification_import_file_check_columns($columns, $header, $exist = TRUE, $return_difference = FALSE) {
  
  if (!is_array($columns)) $columns = array($columns);
  $difference = array_diff($columns, $header);
  if ($return_difference) return $difference; // Just return the difference.
  if ($exist && (count($difference) == 0) || (!$exist && count($difference) == count($columns))) return TRUE;
  return FALSE;
  
}

/**
 * Wrapper function for the process of importing Excel file into the database. Called from import file form.
 * @param $file
 * @param $vocab
 * @param $excel_info
 * @param $data
 * @return unknown_type
 */
function _classification_import_file_insert_data($file, $vocab, $excel_info, $data) {

  global $root_terms_count;
  
  // Generate vars array for use in watchdog errors.
  $msg_vars = array('%file' => $file->filename);
  // Initialise variable that we will use to record success/failure and user messages to return.
  $inserted = array();
  // Generate feedback list items for display to user with regards to excel info.
  $feedback_items = array();
  // Make sure we have all of the required variables to proceed, otherwise return as failed.
  if (empty($file) || empty($vocab->vid) || empty($excel_info) || empty($data)) {
    // Looks like checks haven't completed correctly, write to watchdog and return error.
    watchdog('classification_import', t('Required variables are empty, _classification_import_file_insert_data() could not be initiated for %file.'), $msg_vars, WATCHDOG_ERROR);
    $inserted['success'] = FALSE;
    $inserted['partial'] = FALSE;
    $inserted['feedback'] = t('A problem occurred when preparing file %file for import. Please contact your site administrator if the problem persists.', $msg_vars);
    return $inserted;
  }
  
  // Start transaction.
  $txn = new Transaction();
  
  // Get the term ids for the taxonomic relationships 'synonym' and 'vernacular name'.
  $relation_types = array();
  $query = db_query("SELECT rtid, name, vid FROM {term_relation_types}");
  while ($result = db_fetch_array($query)) {
    $relation_types[$result['name']] = $result;
  }
  $synonym_relationship_term_tid = db_result(db_query("SELECT tid FROM {term_data} WHERE vid = %d AND name = '%s'", $relation_types['taxonomic relationships']['vid'], 'synonym'));
  $vernacular_relationship_term_tid = db_result(db_query("SELECT tid FROM {term_data} WHERE vid = %d AND name = '%s'", $relation_types['taxonomic relationships']['vid'], 'vernacular name'));
  unset($query);
  
  $taxa = array();
  // Process insert depending on file structure.
  switch ($excel_info['file_structure']) {
    
    case FILE_STRUCTURE_PARENT_CHILD:
      
      // Convert data rows into more useable array.
      foreach ($data as $index => $row) {
        $id = $row[t('Identifier')]['value'];
        if (empty($id) && !is_numeric($id)) continue; // Ignore empty Identifier cell (could be blank row?), but include Identifier = 0. After foreach is finished $data will hold all skipped rows if we want to do further error handling. TODO: Remove empty rows on file check, why are we getting empty row here?
        // Reassign data to more useful arrays.
        $taxa[$id] = array(
          'name' => trim($row[t('Child')]['value']),
          'vid' => $vocab->vid,
          'description' => trim($row[t('Description')]['value']),
        );
        $extras[$id] = array(
          'parent_id' => $row[t('Parent')]['value'],
          'rank' => trim($row[t('Rank')]['value']),
          'synonyms' => trim($row[$excel_info['synonym_column']]['value']),
          'vernaculars' => trim($row[t('Vernaculars')]['value']),
          'vernaculars_language' => trim($row[t('VernacularsLanguage')]['value']),
        );

        // Don't need data row any more so get rid.
        unset($data[$index]);
      }
      if (count($data)) {
        // Got more than one 'empty row', or missing identifier, why?
        // TODO: Remove empty rows when $data is generated in file check.
      }
      unset($data);


      // Insert term data into database.
      foreach ($taxa as $id => $term_record) {
        $msg_vars = array_merge($msg_vars, array('%id' => $id));
        if (!_classification_import_file_insert_taxon_term_parent_child($id, $taxa, $extras, &$root_terms_count)) {
          $inserted['success'] = FALSE;
          $inserted['partial'] = TRUE; // Problem inserting data, could be a partial import issue.
          $inserted['feedback'] = t('A problem occurred when attempting to import taxon with Identifier: %id. Please check for partial data import.', $msg_vars);
          // Transaction rollback.
          $txn->rollback();
          return $inserted;
        }
      }

    break;
    
    
    case FILE_STRUCTURE_FULL_HIERARCHY:
    case FILE_STRUCTURE_FULL_HIERARCHY_ALT:
      
      // Assume that everything in between (Identifier or 0) and (Synonym or Rank) columns is taxon column.
      $column_index = array();
      foreach (array(t('Identifier'), t('Rank'), $excel_info['synonym_column']) as $column) {
        $i = array_search($column, $excel_info['header'], TRUE);
        if (is_numeric($i)) $column_index[] = $i;
        unset($i);
      }
      sort($column_index, SORT_NUMERIC); // We only care about the first two items in the array, i.e. beginning and end boundaries.

      // Identifier column should always be 0, if it's missing then the first item in the column_index array will have a value > 0 and therefore must be a rank or synonym(s) column.
      // If Identifier column is missing, that means first column in file is a taxon column, so we add -1 to beginning of column_index array to create a starting point for later taxon columns loop.
      if ($column_index[0] > 0) array_unshift($column_index, -1); 

      // Sanity check on column index, before processing columns.
      if ((is_numeric($column_index[0]) && is_numeric($column_index[1])) && $column_index[0] < $column_index[1]) {
        if ($excel_info['file_structure'] == FILE_STRUCTURE_FULL_HIERARCHY) {
          // Get ranks from column headers for full hierarchy format.
          for ($i = $column_index[0] + 1; $i < $column_index[1]; $i++) {
            $ranks[$i] = $excel_info['header'][$i];
            $rank_term_tid = _classification_import_file_get_rank_tid($excel_info['header'][$i], $relation_types['taxonomic ranks']['vid']);
            if ($rank_term_tid > 0) {
              $ranks_tids[$excel_info['header'][$i]] = $rank_term_tid;
            }
            else {
              drupal_set_message(t('Could not get rank tid.'), 'error', FALSE);
            }
            unset($rank_term_tid);
          }
        }

        // Build taxa array.
        foreach ($data as $index => $row) {

          // Get taxon names and store treepath.
          $taxon = array();
          for ($i = $column_index[1] - 1; $i > $column_index[0]; $i--) {

            // Last populated rank column should be the taxon term. So we iterate columns in reverse, ignoring  empty cells.
            if (empty($taxon['name'])) { // Haven't hit first column of content yet.
              if (empty($row[$excel_info['header'][$i]]['value'])) {
                // Still not hit first column from end with content so continue to next column.
                continue;
              }
              else {
                // We've hit first column from end with content. So this is taxon name.
                $taxon['name'] = $row[$excel_info['header'][$i]]['value'];
                $taxon['column'] = $excel_info['header'][$i];
              }
            }
            else {
              // Taxon name has been added, so everything else we hit is a parent.
              if (empty($row[$excel_info['header'][$i]]['value'])) {
                // Empty cell somewhere in the treepath.
                if ($excel_info['file_structure'] == FILE_STRUCTURE_FULL_HIERARCHY) {
                  // Replace empty cell with "Unassigned", later this will be treated as a 'missing taxon' and added.
                  $taxon['parent_treepath'][$i] = t('Unassigned');
                  if (empty($taxon['parent_column'])) $taxon['parent_column'] = $excel_info['header'][$i];
                }
              }
              else {
                // Parent name exists add it to treepath.
                $taxon['parent_treepath'][$i] = $row[$excel_info['header'][$i]]['value'];
                if (empty($taxon['parent_column'])) $taxon['parent_column'] = $excel_info['header'][$i];
              }
            }
          }
          if (empty($taxon['name'])) {
            // Row could be completely empty. Either way we've got no taxon name so we should skip this row.
            // After foreach is finished $data will still contain this row if we want to do further error handling.
            // TODO: Remove empty rows on file check - why are we getting an empty row here?
            continue;
          }
          
          $taxon['parent_treepath'] = array_reverse($taxon['parent_treepath'], TRUE);
          $taxon['parent_treepath_id'] = implode('', $taxon['parent_treepath']);
          $treepath_id = $taxon['parent_treepath_id'] . $taxon['name'] . $taxon['column'];
          if (!empty($taxon['parent_treepath_id'])) $taxon['parent_treepath_id'] .= $taxon['parent_column'];
          
          $taxa[$treepath_id] = array(
            'name' => $taxon['name'],
            'vid' => $vocab->vid,
            'description' => trim($row[t('Description')]['value']),
          );
          $extras[$treepath_id] = array(
            'id' => $row[t('Identifier')]['value'],
            'column' => $taxon['column'],
            'rank' => ($excel_info['file_structure'] == FILE_STRUCTURE_FULL_HIERARCHY) ? $taxon['column'] : trim($row[t('Rank')]['value']),
            'parent_treepath' => $taxon['parent_treepath'],
            'parent_treepath_id' => $taxon['parent_treepath_id'],
            'parent_column' => $taxon['parent_column'],
            'synonyms' => trim($row[$excel_info['synonym_column']]['value']),
            'vernaculars' => trim($row[t('Vernaculars')]['value']),
            'vernaculars_language' => trim($row[t('VernacularsLanguage')]['value']),
          );

          // No longer need $taxon or $data so get rid.
          unset($taxon);
          unset($data[$index]);
        }
        if (count($data) > 1) {
          // Got more than one 'empty row', or missing identifier, why?
          // TODO: Remove empty rows when $data is generated in file check.
        }
        unset($data);

        // Now need to write taxa to database.
        foreach ($taxa as $treepath_id => $term_record) {
          // Add treepath to vars for feedback messages.
          $msg_vars['%treepath'] = $treepath_id;
          
          // Recursively insert taxon, returns FALSE if failed to insert a taxon or any of it's parents.
          if (!_classification_import_file_insert_taxon_term_full_hierarchy($treepath_id, $taxa, $extras, $excel_info['header'], &$root_terms_count)) {
            $inserted['success'] = FALSE;
            $inserted['partial'] = TRUE; // Problem inserting data, could be a partial import issue.
            $inserted['feedback'] = t('A problem occurred when attempting to import taxa in the classification hierarchy:  %treepath. Please check for a partial data import in the classification editor.', $msg_vars);
            // Transaction rollback.
            $txn->rollback();
            return $inserted;
          }
        }
      }
      else {
        watchdog('classification_import', 'Couldn\'t recognise the column structure of the taxon columns for %file. Suspect incorrect placement of boundary columns Synonym, Synonyms, or Rank.', $msg_vars, WATCHDOG_ERROR);
        $inserted['success'] = FALSE;
        $inserted['partial'] = FALSE; // Problem with column structure, no data was imported.
        $inserted['feedback'] = t('There was a problem converting columns to a classification hierarchy. Your taxon columns must be placed in order from higher to lower classification, and must appear after the Identifier column, if it exists, and before any other columns. The first column to appear after your taxon columns must be one of Synonym, Synonyms or Rank.');
        // Transaction rollback.
        $txn->rollback();
        return $inserted;
      }
      
    
    break;
    default:
      // If we get here then there is a bug in code regarding file structures.
      $inserted['success'] = FALSE;
      $inserted['partial'] = FALSE; // Problem with file structure, no data was imported.
      $inserted['feedback'] = t('There is a problem with classification imports. Please contact your site administrator. No data was imported.');
      return $inserted;  
    break;
  }
  
  // Taxa terms should all be added by now, without error, so insert related data, and build feedback.
  // No return from function within foreach. Until we have transactions in place we can't rollback, so we continue to add as much data as possible. Inserts of related data are independent for each taxon term, so also don't need to worry about dependencies here.
  foreach ($taxa as $id => $term_record) {
    // Add to treepath to feedback vars.
    $msg_vars['%id'] = $id;
    
    // If our term record as a tid then its been successfully added to the database, so we can proceed with adding term relations, otherwise we bail.
    if ($term_record['tid'] > 0) {

      if ($excel_info['file_structure'] == FILE_STRUCTURE_FULL_HIERARCHY && empty($extras[$id]['rank'])) {
        // If don't have rank already then get it from column header.
        $extras[$id]['rank'] = trim($extras[$id]['column']);
      }
      // If we've found a rank then insert it for this term.
      if (!empty($extras[$id]['rank'])) {
        $msg_vars['%rank'] = $extras[$id]['rank'];
        // Get the tid of that rank label record, we might have stored it already, or we go get it.
        if (isset($ranks_tids)) {
          $rank_term_tid = $ranks_tids[$extras[$id]['rank']];
        }
        else {
          $rank_term_tid = _classification_import_file_get_rank_tid($extras[$id]['rank'], $relation_types['taxonomic ranks']['vid']);
        }
        // If we have a rank label tid then go ahead and make the relationship.
        if ($rank_term_tid > 0) {
          if (!_classification_import_file_insert_term_relation($taxa[$id]['tid'], $rank_term_tid, $relation_types['taxonomic ranks']['rtid'])) {
            // Couldn't insert rank relation.
            // TODO: Is this failure worthy of a rollback?
            $inserted['success'] = FALSE;
            $inserted['partial'] = TRUE; // Problem inserting data, could be a partial import issue.
            $feedback_items[] = array('data' => t('Rank relation was not added for taxon with classification hierarchy, or identifier %id, in file %file.', $msg_vars), 'class' => 'error');
            // Don't return straight away as other rank data might be inserted correctly and names have already been put in anyway, and can't rollback until we get transactions in place.
          }
        }
        else {
          // Couldn't get rank tid.
          // TODO: Is this failure worthy of a rollback?
          $inserted['success'] = FALSE;
          $inserted['partial'] = TRUE; // Problem inserting data, could be a partial import issue.
          $feedback_items[] = array('data' => t('Could not get rank tid for taxon with classification hierarchy, or identifier %id, in file %file. Rank %rank was not saved.', $msg_vars), 'class' => 'error');
          // Don't return straight away as other rank data might be inserted correctly and names have already been put in anyway, and can't rollback until we get transactions in place.
        }
      }
      
      // Insert vernacular names for this term.
      if (!empty($extras[$id]['vernaculars'])) {
        if (!_classification_import_file_insert_related_terms($extras[$id]['vernaculars'], $taxa[$id]['tid'], $vocab->vid, $vernacular_relationship_term_tid, $relation_types['taxonomic relationships']['rtid'], $extras[$id]['vernaculars_language'], $relation_types['names to vernacular codes'])) {
          // Something bad happened when inserting vernaculars and languages: TODO: Error handling.
          $inserted['success'] = FALSE;
          $inserted['partial'] = TRUE; // Problem inserting data, could be a partial import issue.
          $feedback_items[] = array('data' => t('Some vernaculars were not added for taxon with classification hierarchy, or identifier %id, in file %file.', $msg_vars), 'class' => 'error');
        }
      }
      
      // Insert synonyms for this term.
      if (!empty($extras[$id]['synonyms'])) {
        if (!_classification_import_file_insert_related_terms($extras[$id]['synonyms'], $taxa[$id]['tid'], $vocab->vid, $synonym_relationship_term_tid, $relation_types['taxonomic relationships']['rtid'])) {
          // Something bad happened when inserting synonyms.
          $inserted['success'] = FALSE;
          $inserted['partial'] = TRUE; // Problem inserting data, could be a partial import issue.
          $feedback_items[] = array('data' => t('Some synonyms were not added for taxon with classification hierarchy, or identifier %id, in file %file.', $msg_vars), 'class' => 'error');
        }
      }
    
    }
    else {
      // We have a taxa without a tid, something bad happened that wasn't caught previously.
      $inserted['success'] = FALSE;
      $inserted['partial'] = TRUE; // Problem inserting data, could be a partial import issue.
      $feedback_items[] = array('data' => t('There was a problem inserting taxon with classification hierarchy, or identifier %id, in file %file. Please check for a partial data import in the classification editor.', $msg_vars), 'class' => 'error');
      // Transaction rollback.
      $txn->rollback();
    }
  }
  
  // Check we do not exceed the limit for the number of root values.
  $limit = variable_get("classification_import_copypaste_limit", 1000);
  if ($root_terms_count > $limit) {
    $msg_vars['%root_terms_limit'] = $limit;
    $msg_vars['%root_terms_count'] = $root_terms_count;
    // Too many root terms will cause a problem in the users browser. They won't be able to see the tree, so we roll back if we've got transactions.
    $inserted['partial'] = FALSE;
    $feedback_items[] = array('data' => t('There are %root_terms_count root terms in file %file, due to rendering limits of the classification tree, you may not have more than %root_terms_limit root terms in a single tree. Data were not imported.', $msg_vars), 'class' => 'error');
    // Transaction rollback.
    $txn->rollback();
  }
  // If we have multiple feedback items, theme them as a list and add them to feedback message we will return.
  if (count($feedback_items) > 0) {
    $inserted['feedback'] = theme_item_list($feedback_items, NULL, 'ul', array('class' => 'import_parse_message'));
  }
  // If success parameter has not been set, then we assume no errors and set to TRUE.
  if (!isset($inserted['success'])) {
    $inserted['success'] = TRUE; // If not previously set to false then we assume import has succeeded.
    // If no messages set, then we assume everything went okay and let users know.
    $msg_vars['%classification'] = $vocab->name;
    $msg_vars['!edit'] = l(t('View and Edit'), 'admin/classification/biological/edit/' . $vocab->vid, array('html' => true));
    if (!isset($inserted['feedback'])) $inserted['feedback'] = t('File %file imported successfully into %classification. !edit', $msg_vars);
  }
  return $inserted;
}
/**
 * Helper function to get taxon name and column off end of treepath.
 * @param $treepath
 * @return array
 */
function _classification_import_file_chop_treepath(&$treepath) {
  $taxon = array();
  $column_name = array_slice($treepath, -1, 1, TRUE);
  foreach ($column_name as $column_id => $name) {
    $taxon['name'] = $name;
    $taxon['column_id'] = $column_id;
    unset($treepath[$column_id]);
  }
  return $taxon;
}

/**
 * Helper function to generate data for missing taxon.
 * @param $treepath
 * @param $columns
 * @return array
 */
function _classification_import_file_generate_missing_taxon_data($treepath, $columns) {

  $taxon = _classification_import_file_chop_treepath($treepath);
  $taxon['column'] = $columns[$taxon['column_id']];
  $taxon['parent_treepath'] = $treepath;
  $taxon['parent_treepath_id'] = implode('', $treepath);
  $parent_taxon = _classification_import_file_chop_treepath($treepath);
  $taxon['parent_column'] = $columns[$parent_taxon['column_id']];
  if (!empty($taxon['parent_treepath_id'])) $taxon['parent_treepath_id'] .= $taxon['parent_column'];
  
  return $taxon;
}
/**
 * Helper function to insert taxon terms for full hierarchy format.
 * @param $treepath_id
 * @param $taxa
 * @param $extras
 * @param $columns
 * @return boolean
 */
function _classification_import_file_insert_taxon_term_full_hierarchy($treepath_id, &$taxa, &$extras, $columns) {
  
  global $root_terms_count;

  if ($taxa[$treepath_id]['tid'] > 0) return TRUE; // Already been inserted.

  // Not been inserted so if its got a parent, find it by matching treepath with taxon name and column.
  $parent_treepath_id = $extras[$treepath_id]['parent_treepath_id'];
  if (!empty($parent_treepath_id)) {
    if (empty($taxa[$parent_treepath_id])) {
      // If we can't find a matching parent then we should create one.
      $taxon = _classification_import_file_generate_missing_taxon_data($extras[$treepath_id]['parent_treepath'], $columns);
      $taxa[$parent_treepath_id] = array(
        'name' => $taxon['name'],
        'vid' => $taxa[$treepath_id]['vid'],
      );
      $extras[$parent_treepath_id] = array(
        'column' => $taxon['column'],
        'parent_column' => $taxon['parent_column'],
        'parent_treepath' => $taxon['parent_treepath'],
        'parent_treepath_id' => $taxon['parent_treepath_id'],
      );
      unset($taxon);
    }

    // If we've got this far then we a matching parent should exist somewhere.
    if (!$taxa[$parent_treepath_id]['tid'] > 0) {
      _classification_import_file_insert_taxon_term_full_hierarchy($parent_treepath_id, $taxa, $extras, $columns);
    }
    $parent_tid = $taxa[$parent_treepath_id]['tid'];
  }
  else {
    // Parent treepath id is empty, so we assume this is a root term, assign parent of 0, and increment root count.
    $root_terms_count++;
    $taxa[$treepath_id]['parent'] = 0;
  }
  // Insert taxon record.
  if (!is_numeric($taxa[$treepath_id]['parent'])) $taxa[$treepath_id]['parent'] = $taxa[$parent_treepath_id]['tid'];
  if (!empty($taxa[$treepath_id]['name'])) {
    taxonomy_save_term($taxa[$treepath_id]);
  }
  if ($taxa[$treepath_id]['tid'] > 0) {
   return TRUE;
  }
  else {
   // Unable to save term, why? This is bad and will result in partial import. What should we do?
   return FALSE;
  }

}

/**
 * Helper function to get rank tid.
 * @param $rank string
 * @param $vid id of ranks vocabulary
 * @return integer, if  > 0 then success.
 */
function _classification_import_file_get_rank_tid($rank, $vid) {
  if (stristr($rank, t('Taxon'))) return FALSE; // This is not a proper rank, ignore.
  $rank = strtolower(trim($rank));
  // Get rank tid form database if it exists.
  $rank_term_tid = db_result(db_query("SELECT td.tid FROM {term_data} td WHERE td.vid = %d AND LOWER(td.name) = '%s'", $vid, $rank));
  if ($rank_term_tid > 0) return $rank_term_tid;
  
  // Rank is not found in database, assume its a new rank and insert it. 
  $rank_term = array(
    'name' => strtolower($rank),
    'parent' => 0,
    'vid' => $vid,
  );
  taxonomy_save_term($rank_term);
  
  return $rank_term['tid']; 
}

/**
 * Helper function to recursively insert terms for parent-child array.
 * @param $id
 * @param $taxa
 * @param $extras
 * @return unknown_type
 */
function _classification_import_file_insert_taxon_term_parent_child($id, &$taxa, $extras) {
  
  global $root_terms_count;
  
  if ($taxa[$id]['tid'] > 0) return TRUE; // Already been inserted.
  
  // Not been inserted so see if parent exists if so get its tid.
  if ($extras[$id]['parent_id'] > 0) {
    // If parent doesn't have tid recurse this function to insert parent so can get its tid ($taxa will be updated by reference).
    if (!$taxa[$extras[$id]['parent_id']]['tid'] > 0) {
      _classification_import_file_insert_taxon_term_parent_child($extras[$id]['parent_id'], $taxa, $extras);
    }
  }
  else {
    // This is root term so set parent to 0, and increment $root_terms_count.
    $root_terms_count++;
    $taxa[$id]['parent'] = 0;
  }
  // If not a root term then get parent's tid, which should be set by now.
  if (!is_numeric($taxa[$id]['parent'])) $taxa[$id]['parent'] = $taxa[$extras[$id]['parent_id']]['tid'];
  // Insert taxon record.
  if (!empty($taxa[$id]['name'])) {
    taxonomy_save_term($taxa[$id]);
  } 
  if ($taxa[$id]['tid'] > 0) {
    return TRUE;
  }
  else {
    // Unable to save term, why? This will result in partial import.
    // Returning FALSE will trigger a transaction rollback.
    return FALSE;
  }
}
/**
 * Helper function to insert term_relation and term_relation_has_type records.
 * @param $tid1
 * @param $tid2
 * @param $relationship_type_rtid
 * @return unknown_type
 */
function _classification_import_file_insert_term_relation($tid1, $tid2, $relationship_type_rtid = NULL) {
  
  if (!$tid1 > 0 || !$tid2 > 0) {
    watchdog('classification_import', t('Missing tid1 or tid2, cannot initiate _classification_import_file_insert_term_relation()'), array(), WATCHDOG_ERROR);
    // Missing required variables. Can we have term relation without a term relation type? TODO: Error handling.
    return FALSE;
  }
  
  // Build term relation record, and insert into datbase.
  $term_relation = array(
    'tid1' => $tid1,
    'tid2' => $tid2,
  );
  if (!drupal_write_record('term_relation', $term_relation)) {
    // Unable to insert term relation. TODO: Error handling.
    drupal_set_message(t('Cannot insert term relation record.'), 'error', FALSE);
    return FALSE;
  }
  
  // Use returned term_relation id and generate term_relation_has_type record for writing to database.
  if (!empty($relationship_type_rtid) && $term_relation['trid'] > 0) {
    $term_relation_has_type = array(
      'trid' => $term_relation['trid'], // term_relation record.
      'rtid' => $relationship_type_rtid, // id of relationship type e.g. id of 'taxonomic relationships'.
    );
    if (!drupal_write_record('term_relation_has_type', $term_relation_has_type)) {
      // Unable to insert term relation has type. TODO: Error handling.
      drupal_set_message(t('Cannot insert term relation has type record.'), 'error', FALSE);
      return FALSE;
    }
  }
  return TRUE;
}
/**
 * Helper function to insert multiple children terms given single parent, and any associated relationships.
 * @param $terms array or string | separated.
 * @param $parent_tid
 * @param $vid
 * @param $relationship_term_tid
 * @param $relationship_type_rtid
 * @param $languages
 * @return unknown_type
 */
function _classification_import_file_insert_related_terms($terms, $parent_tid, $vid, $relationship_term_tid, $relationship_type_rtid, $languages = NULL, $language_relation_type = NULL) {
  
  if (empty($terms) || empty($parent_tid) || empty($vid) || empty($relationship_term_tid) || empty($relationship_type_rtid)) {
    watchdog('classification_import', t('One or more required variables are empty, _classification_import_file_insert_term_relation() could not be initiated.'), array(), WATCHDOG_ERROR);
    return FALSE;
  }
  
  if (!is_array($terms)) $terms = explode('|', $terms);
  
  foreach ($terms as $i => $name) {
    $name = trim($name);
    if (!empty($name)) {
      // Create term record for writing to database using taxonomy_save_term();
      $term = array(
        'name' => $name,
        'parent' => $parent_tid,
        'vid' => $vid,
      );
      if (!taxonomy_save_term($term)) {
        drupal_set_message(t('Could not insert a related term.'), 'error', FALSE);
        return FALSE; // Couldn't insert related term. TODO: Error handling.
      }
      
      // Use returned term_data tid and generate term_relation record for writing to database.
      if ($term['tid'] > 0) {

        // Insert term_relation and term_relation_has_type records.
        if (!_classification_import_file_insert_term_relation($term['tid'], $relationship_term_tid, $relationship_type_rtid)) {
           // Couldn't insert term_relation or term_relation_has_type record, why? TODO: Error handling.
           drupal_set_message(t('Could not insert term relation or term relation has type records for related term.'), 'error');
        }
      
        // Check to see if we have secondary relationships to add, i.e. languages.
        if (!empty($languages) && !empty($language_relation_type)) {

          if (!is_array($languages)) $languages = explode('|', $languages);

          $languages[$i] = trim($languages[$i]);
          
          if (!empty($languages[$i])) { // Language code exists for related term.
            // Get term_data id of language code.
            $language_term_tid = db_result(db_query("SELECT tid FROM {term_data} WHERE vid = %d AND RIGHT(name, 2) = '%s'", $language_relation_type['vid'], $languages[$i]));
            if ($language_term_tid > 0) { // If language code is recognised. TODO: What if language code is not in database, should it be added?
              // Insert term_relation and term_relation_has_type records for language codes.
              if(!_classification_import_file_insert_term_relation($term['tid'], $language_term_tid, $language_relation_type['rtid'])) {
                // Couldn't insert language code relationship, why? TODO: Error handling.
                drupal_set_message('Could not add term relation to language code.', 'error');
                return FALSE;
              }  
            }
            else {
              // Language code not in database? TODO: What?
              drupal_set_message(t('Language code %language was not found in database.', array('%language' => $language[$id]), 'error'));
            }
          }
        } // End if languages exist.

      }
      else {
        // Term tid is not > 0. TODO: Error handling.
        return FALSE;
      }
      
    }
  }
  
  return TRUE;
}

/**
 * Helper function to get the header, column count, row count and first five rows from a PHPExcel object.
 *
 * @param $objPHPExcel
 *   The PHPExcel object
 * @return
 *   An array containing header cells, column count, row count, and first five rows in document
 */
function _classification_import_get_excel_info($objPHPExcel) {
  $info = array();

  $active_sheet = $objPHPExcel->getActiveSheet();
  $max_col = $active_sheet->getHighestColumn();

  // Get the row & column count.
  $info['colcount'] = PHPExcel_Cell::columnIndexFromString($max_col);
  $info['rowcount'] = $active_sheet->getHighestRow();

  $header = PHPExcel_Cell::extractAllCellReferencesInRange('A1:' . $max_col . '1');
  foreach($header as $cell) {
    $info['header'][] = $active_sheet->getCell($cell)->getCalculatedValue();
  }

  $first_five = PHPExcel_Cell::extractAllCellReferencesInRange('A2:' . $max_col . '6');
  foreach($first_five as $cell) {
    $info['firstfive'][$active_sheet->getCell($cell)->getRow()][] = $active_sheet->getCell($cell)->getCalculatedValue();
  }

  return $info; 
}

/**
 * Helper function to get all the data from the spreadsheet without the first header row, which is required
 *
 * @param $objPHPExcel
 *   The PHPExcel object
 * @return
 *   A large array containing the content of the spreadsheet with row index and column header as keys
 */
function _classification_import_build_content($objPHPExcel) {
  $content = array();
  foreach ($objPHPExcel->getWorksheetIterator() as $worksheet) {
    foreach ($worksheet->getRowIterator() as $row) {
      $rowindex = $row->getRowIndex();
      $cellIterator = $row->getCellIterator();
      $cellIterator->setIterateOnlyExistingCells(false); // Loop all cells, even if it is not set.
      foreach ($cellIterator as $cell) {
        if (!is_null($cell) && $rowindex > 1) {
          $col_header = $worksheet->getCell($cell->getColumn() . '1')->getCalculatedValue();
          $content[$rowindex][$col_header] = array(
            'type' => $cell->getDataType(),
            'value' => $cell->getCalculatedValue(),
          );
        }
      }
    }
  }
  return $content;
}

/**
 * Helper function to recursively count values in a multidimensional array.
 *
 * Found on http://php.net/manual/en/function.array-count-values.php and modified with array_filter to strip out empty elements
 */
function _classification_import_array_count_values_multidim($a, $out = FALSE) {
  if ($out === FALSE) $out = array();
  if (is_array($a)) {
    $a = array_filter($a);
    foreach($a as $e) {
      $out = _classification_import_array_count_values_multidim($e,$out);
    }
  }
  else {
    if (array_key_exists($a,$out)) {
      $out[$a]++;
    }
    else {
      $out[$a]=1;
    }
  }
  return $out;
}