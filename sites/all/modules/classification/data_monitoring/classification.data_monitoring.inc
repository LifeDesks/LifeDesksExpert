<?php
// $Id$
/**
 * @file
 * Data monitoring monitors.
 */
/**
 * Implementation of hook_data_monitoring_monitors().
 */
function classification_data_monitoring_monitors() {

  // Define monitors (select queries to run for monitoring purposes).
  $monitors = array();

  // classification_hierarchy_loop
  $monitors[0] = new DataMonitoringMonitor("classification_hierarchy_loop");
  $monitors[0]->setName("Terms where term id is equal to parent id");
  $monitors[0]->setDescription("Under no circumstance should the id of the term equal the term's parent id."
                . " The consequence of this occurring would be an endless loop when trying to build the term hierarchy.");
  $monitors[0]->setQuery("SELECT td.tid, th.parent, v.vid, td.name AS term_name, v.name AS vocabulary_name
                          FROM {term_data} td
                          INNER JOIN {term_hierarchy} th ON td.tid = th.tid
                          INNER JOIN {vocabulary} v ON td.vid = v.vid
                          WHERE th.tid = th.parent");


  // classification_hierarchy_missing
  $monitors[1] = new DataMonitoringMonitor("classification_hierarchy_missing");
  $monitors[1]->setName("Terms that do not have a term hierarchy record");
  $monitors[1]->setDescription("Every term should have a record in the term_hierarchy table, with a parent value >= 0."
                . " The consequence of not having a term_hierarchy record is a ghost name."
                . " A ghost name is one that cannot be rendered in the classification tree, but will appear in auto complete tagging mechanism.");
  $monitors[1]->setQuery("SELECT td.tid, td.vid, td.name, th.parent FROM {term_data} td LEFT JOIN {term_hierarchy} th ON td.tid = th.tid WHERE th.tid IS NULL");


  // classification_parent_missing
  $monitors[2] = new DataMonitoringMonitor("classification_parent_missing");
  $monitors[2]->setName("Term hierarchy entries where parent does not exist");
  $monitors[2]->setDescription("Each term must have a parent/child relationship."
                . " The consequence of a a child's parent no longer existing is a ghost name. "
                . " A ghost name is one that cannot be rendered in the classification tree, but will appear in auto complete tagging mechanism.");
  $monitors[2]->setQuery("SELECT th.tid, th.parent FROM {term_hierarchy} th LEFT JOIN {term_data} td ON th.parent = td.tid WHERE th.parent > 0 AND td.tid IS NULL");


  // classification_parent_elsewhere
  $monitors[3] = new DataMonitoringMonitor("classification_parent_elsewhere");
  $monitors[3]->setName("Terms where parent is in a different vocabulary");
  $monitors[3]->setDescription("Child/parent relationships should only ever occur between terms that are in the same vocabulary."
                . " The consequence of having a parent in another vocabulary, is a ghost name."
                . " A ghost name is one that cannot be rendered in the classification tree, but will appear in auto complete tagging mechanism.");
  $monitors[3]->setQuery("SELECT td.tid, td.name, td.vid, v.module, th.parent, td2.vid as parent_vid FROM {term_data} td LEFT JOIN {vocabulary} v ON td.vid = v.vid LEFT JOIN {term_hierarchy} th ON td.tid = th.tid INNER JOIN {term_data} td2 ON th.parent = td2.tid WHERE td.vid <> td2.vid");


  // classification_term_blank
  $monitors[4] = new DataMonitoringMonitor("classification_term_blank");
  $monitors[4]->setName("Terms where name is blank");
  $monitors[4]->setDescription("Each term has a name value that is visible to the user."
                . " The consequence of a term having an empty name value is that the user will not be able to see that term even though it exists."
                . " In the classification tree, for example, the name may appear as a blank leaf.");
  $monitors[4]->setQuery("SELECT tid, vid, name FROM {term_data} td WHERE td.name = '' OR td.name IS NULL");


  // classification_node_term_node_missing
  $monitors[5] = new DataMonitoringMonitor("classification_node_term_node_missing");
  $monitors[5]->setName("Term node entries, where node does not exist");
  $monitors[5]->setDescription("A node is a content object. Each content object can be tagged with a term."
                . " A taxon page content object, for example, will be tagged with a term which is the taxon name."
                . " This monitor counts the number of tags where the content object does not exist, thus the tag is redundant.");
  $monitors[5]->setQuery("SELECT tn.nid, tn.tid FROM {term_node} tn LEFT JOIN {node} n ON tn.nid = n.nid WHERE n.nid IS NULL");


  // classification_node_term_term_missing
  $monitors[6] = new DataMonitoringMonitor("classification_node_term_term_missing");
  $monitors[6]->setName("Term node entries, where term does not exist");
  $monitors[6]->setDescription("A node is a content object. Each content object can be tagged with a term."
                . " A taxon page content object, for example, will be tagged with a term which is the taxon name."
                . " This monitor counts the number of tags where the term does not exists, thus the tag is redundant.");
  $monitors[6]->setQuery("SELECT tn.nid, tn.tid FROM {term_node} tn LEFT JOIN {term_data} td ON tn.tid = td.tid WHERE td.tid IS NULL");


  // classification_term_relation_missing_terms
  $monitors[7] = new DataMonitoringMonitor("classification_term_relation_missing_terms");
  $monitors[7]->setName("Term relation records where one or both terms do not exist");
  $monitors[7]->setDescription("Term relation records associate two terms."
                 . " The consequence of a relationship between terms where one or both terms do not exist, is a redundant relationship record.");
  $monitors[7]->setQuery("SELECT tr.*, td.name AS term1, td2.name AS term2
                           FROM {term_relation} tr
                           LEFT JOIN {term_data} td ON td.tid = tr.tid1
                           LEFT JOIN {term_data} td2 ON td2.tid = tr.tid2
                           WHERE td.tid IS NULL
                           OR td2.tid IS NULL");


  // classification_term_relation_has_type_term_relation_missing
  $monitors[8] = new DataMonitoringMonitor("classification_term_relation_has_type_term_relation_missing");
  $monitors[8]->setName("Term relation has type entries where the term relation does not exist");
  $monitors[8]->setDescription("A relationship between two terms can be described by assigning the relationship a type."
                . " The consequence of having types describing relationships that do not exist, is redundant data.");
  $monitors[8]->setQuery("SELECT trht.trid, trht.rtid FROM {term_relation_has_type} trht LEFT JOIN {term_relation} tr ON trht.trid = tr.trid WHERE tr.trid IS NULL");


  // classification_term_hierarchy_child_missing
  $monitors[9] = new DataMonitoringMonitor("classification_term_hierarchy_child_missing");
  $monitors[9]->setName("Term hierarchy entries where child term does not exist");
  $monitors[9]->setDescription("Each term has a parent/child relationship."
                . " The consequence of a parent/child relationship existing when the child no longer exists is, redundant data.");
  $monitors[9]->setQuery("SELECT th.tid, th.parent FROM {term_hierarchy} th LEFT JOIN {term_data} td ON th.tid = td.tid WHERE td.tid IS NULL");


   // classification_term_multiple_parents
  $monitors[10] = new DataMonitoringMonitor("classification_term_multiple_parents");
  $monitors[10]->setName("Biological terms, flags, and taxon description chapters with more than one parent");
  $monitors[10]->setDescription("For classification, classification flags and taxon description vocabularies, terms should only have a single parent."
                . " The consequence of one of these terms having more than one parent varies, but would cause an endless loop when attempting to render the term in a tree.");
  $monitors[10]->setQuery("SELECT (COUNT(*) - COUNT(DISTINCT th.tid)) AS Count FROM {term_hierarchy} th INNER JOIN {term_data} td ON td.tid = th.tid INNER JOIN {vocabulary} v ON td.vid = v.vid WHERE v.module IN ('classification', 'classification_flags', 'taxon_description')");


  // classification_term_relation_type_missing
  $monitors[11] = new DataMonitoringMonitor("classification_term_relation_type_missing");
  $monitors[11]->setName("Term relation has type entries where the term relation type does not exist");
  $monitors[11]->setDescription("A relationship between two terms can be described by assigning the relationship a type."
                 . " In this case the relationship type does not exist.");
  $monitors[11]->setQuery("SELECT trht.trid, trht.rtid, td.name AS term1, td2.name AS term2
                           FROM {term_relation_has_type} trht
                           LEFT JOIN {term_relation} tr ON trht.trid = tr.trid
                           LEFT JOIN {term_data} td ON td.tid = tr.tid1
                           LEFT JOIN {term_data} td2 ON td2.tid = tr.tid2
                           LEFT JOIN {term_relation_types} trt ON trht.rtid = trt.rtid
                           WHERE trt.rtid IS NULL
                           AND td.tid IS NOT NULL
                           AND td2.tid IS NOT NULL");


  // classification_terms_missing_vocabulary
  $monitors[12] = new DataMonitoringMonitor("classification_terms_missing_vocabulary");
  $monitors[12]->setName("Terms assigned to a vocabulary that does not exist");
  $monitors[12]->setDescription("Every term should be assigned to a vocabulary."
                 . " The consequence of a term not having a vocabulary, is a redundant name, it will not be accessible to any user.");
  $monitors[12]->setQuery("SELECT td.tid, td.vid, td.name FROM {term_data} td LEFT JOIN {vocabulary} v ON td.vid = v.vid WHERE td.vid IS NULL OR td.vid = 0 OR v.vid IS NULL");


  return $monitors;
}

/**
 * Implementation of hook_data_monitoring_observers().
 * @param DataMonitoringMonitor $monitor
 */
function classification_data_monitoring_observers(DataMonitoringMonitor &$monitor) {

  $trigger_when = array('property' => 'num_rows', 'operator' => '>', 'value' => 0);
  $trigger_mode = DATA_MONITORING_MANUAL_RESPONSE_TRIGGER;
  switch ($monitor->getId()) {
    case 'classification_hierarchy_loop':
      $id = "classification_update_hierarchy";
      $description = "Update term parent to equal root or auto generated term.";
      break;
    case 'classification_hierarchy_missing':
      $id = 'classification_create_hierarchy_move_to_lost_found';
      $description = "Update biological classification term parent to equal root or auto generated term. "
                   . "Move the term, its new parent if created and any descendants to lost and found classification. "
                   . "if it is not already in the deleted names or lost and found classification."
                   . "Take no action on non biological terms, further analysis is required, check detailed monitor log.";
      break;

    case 'classification_parent_missing':
      $id = "classification_update_hierarchy_move_to_lost_found";
      $description = "Update biological classification term parent to equal root or auto generated term. "
                   . "Move the term, its new parent if created and any descendants to lost and found classification "
                   . "if it is not already in the deleted names or lost and found classification."
                   . "Take no action on non biological terms, further analysis is required, check detailed monitor log.";
      break;

    case 'classification_parent_elsewhere':
      $id = "classification_copy_ancestry_move_to_lost_found";
      $description = "Create a copy of parent and update biological classification term parent to equal the new parent copy. "
                   . "Move the term, any newly created ancestors and any descendants to lost and found classification "
                   . "if it is not already in the deleted names or lost and found classification."
                   . "Take no action on non biological terms, further analysis is required, check detailed monitor log.";;
      break;

    case 'classification_term_blank':
      $id = "classification_update_term_name";
      $description = "Update biological classification term if it has children in the same classification or has associated content, "
                   . "relabel term to 'Unassigned'. "
                   . "Delete biological classification term if it does not have children in the same classification nor associated content. "
                   . "Take no action on non biological terms, further analysis is required, check detailed monitor log.";
      break;

    case 'classification_node_term_node_missing':
      $id = "delete_node_term_node_missing";
      $description = "Delete node term records if node does not exist.";
      $trigger_mode = DATA_MONITORING_AUTOMATED_RESPONSE_TRIGGER;
      break;

    case 'classification_node_term_term_missing':
      $id = "delete_node_term_term_missing";
      $description = "Delete node term records if term does not exist.";
      $trigger_mode = DATA_MONITORING_AUTOMATED_RESPONSE_TRIGGER;
      break;

    case 'classification_term_relation_missing_terms':
      $id = "delete_term_relation";
      $description = "Delete term relation records if one or both terms do not exist.";
      $trigger_mode = DATA_MONITORING_AUTOMATED_RESPONSE_TRIGGER;
      break;

    case 'classification_term_relation_has_type_term_relation_missing':
      $id = "delete_term_relation_has_type";
      $description = "Delete term relation has type records if term relation does not exist.";
      $trigger_mode = DATA_MONITORING_AUTOMATED_RESPONSE_TRIGGER;
      break;

    case 'classification_term_hierarchy_child_missing':
      $id = "delete_term_hierarchy_child_missing";
      $description = "Delete term hierarchy records if child term does not exist.";
      $trigger_mode = DATA_MONITORING_AUTOMATED_RESPONSE_TRIGGER;
      break;

    case 'classification_term_relation_type_missing':
    case 'classification_terms_missing_vocabulary':
      $id = 'classification_no_response';
      $description = "Response is not configured, further interpretation is required, check detailed monitor logs.";
      $trigger_mode = NULL;
      break;

    case 'classification_term_multiple_parents':
      $id = 'classification_no_response';
      $description = "Response is not configured, further interpretation is required, check detailed monitor logs.";
      $trigger_when = array('property' => 'Count', 'operator' => '>', 'value' => 0);
      $trigger_mode = NULL;
      break;

  }
  if ($id) $monitor->attachObserver(new DataMonitoringResponseHandler('classification', $id, $description, $trigger_when, $trigger_mode));
}

/**
 * Implementation of hook_data_monitoring_response_pre_process().
 * @param DataMonitoringMonitor $monitor
 * @param DataMonitoringResponseHandler $response
 */
function classification_data_monitoring_response_pre_process(DataMonitoringMonitor &$monitor, DataMonitoringResponseHandler &$response) {
  // Need to check if loop in hierarchy is going to be a problem, before starting any processing that requires term's children using classification_get_tree.
  if (in_array($monitor->getId(), array("classification_parent_elsewhere", "classification_parent_missing", "classification_hierarchy_missing"))) {
    if (db_result(db_query("SELECT COUNT(*) FROM {term_hierarchy} th INNER JOIN {term_data} td ON td.tid = th.tid INNER JOIN {vocabulary} v ON td.vid = v.vid WHERE v.module IN ('classification') AND th.tid = th.parent")) > 0) {
      // We have a hierarchy loop issue in classification, don't start response procedure.
      DataMonitoringFeedbackHandler::getInstance()->addFeedback('Hierarchy loop issue found in a biological classification. Please execute response for <em>terms where term id is equal to parent id</em> first. Response procedure for monitor ' . $monitor->getName() .' was not executed.', 'error');
      $response->cancelProcess();
    }
    elseif (db_result(db_query("SELECT (COUNT(*) - COUNT(DISTINCT th.tid)) FROM {term_hierarchy} th INNER JOIN {term_data} td ON td.tid = th.tid INNER JOIN {vocabulary} v ON td.vid = v.vid WHERE v.module IN ('classification')")) > 0) {
      // We have terms with multiple parents in a classification, don't start response procedure.
      DataMonitoringFeedbackHandler::getInstance()->addFeedback('Terms with multiple parents found. Human intervention is required. Response procedure for monitor ' . $monitor->getName() .' cannot be executed.', 'error');
      $response->cancelProcess();
    }
  }

  // Lock classification - be sure to unlock when process finished or is cancelled.
  $locked = _classification_lock_classification();
  if (is_null($locked) || $locked == FALSE) {
    // Can't lock classification, don't run response.
    DataMonitoringFeedbackHandler::getInstance()->addFeedback('It appears someone is modifying classification data at this time. Response procedure was not activated for ' . $monitor_name, 'error');
    $response->cancelProcess();
  }
}

/**
 * Impementation of hook_data_monitoring_response_post_process().
 * @param DataMonitoringMonitor $monitor
 * @param DataMonitoringResponseHandler $response
 */
function classification_data_monitoring_response_post_process(DataMonitoringMonitor &$monitor, DataMonitoringResponseHandler &$response) {
  // Reset classification lock so that it is ready for edits.
  _classification_unlock_classification();
}

/**
 * Impementation of hook_data_monitoring_response_process().
 * @param DataMonitoringMonitor $monitor
 * @param DataMonitoringResponseHandler $response
 */
function classification_data_monitoring_response_process(DataMonitoringMonitor &$monitor, DataMonitoringResponseHandler &$response) {

  switch ($monitor->getId()) {
    case "classification_node_term_node_missing":
      if ($response->getId() == 'delete_node_term_node_missing') {
        $result = db_query("SELECT tn.*
                            FROM {term_node} tn
                            LEFT JOIN {node} n ON tn.nid = n.nid
                            WHERE n.nid IS NULL");
        $note = ($response->getResponseState() == DATA_MONITORING_STATE_PREVIEW) ? t('Will be deleted.') : t('Record deleted');
        while ($row = db_fetch_array($result)) {
          $response->addModifiedRecordToProcessLog($row, $note);
        }
        if ($response->getResponseState() == DATA_MONITORING_STATE_PREVIEW) {
          $response->setDeletedResultCount(db_result(db_query("SELECT COUNT(tn.tid)
                                                    FROM {term_node} tn
                                                    LEFT JOIN {node} n ON tn.nid = n.nid
                                                    WHERE n.nid IS NULL")));
        }
        elseif ($response->getResponseState() == DATA_MONITORING_STATE_AUTOMATED_IMPLEMENTATION ||
                $response->getResponseState() == DATA_MONITORING_STATE_MANUAL_IMPLEMENTATION) {
          $response->setDeletedResultCount(db_affected_rows(db_query("DELETE tn
                                                           FROM {term_node} tn
                                                           LEFT JOIN {node} n ON tn.nid = n.nid
                                                           WHERE n.nid IS NULL")));
        }
      }
      break;

    case "classification_node_term_term_missing":
      if ($response->getId() == 'delete_node_term_term_missing') {
        $result = db_query("SELECT tn.*
                            FROM {term_node} tn
                            LEFT JOIN {term_data} td ON tn.tid = td.tid
                            WHERE td.tid IS NULL");
        $note = ($response->getResponseState() == DATA_MONITORING_STATE_PREVIEW) ? t('Will be deleted.') : t('Record deleted');
        while ($row = db_fetch_array($result)) {
          $response->addModifiedRecordToProcessLog($row, $note);
        }
        if ($response->getResponseState() == DATA_MONITORING_STATE_PREVIEW) {
          $response->setDeletedResultCount(db_result(db_query("SELECT COUNT(tn.tid)
                                                               FROM {term_node} tn
                                                               LEFT JOIN {term_data} td ON tn.tid = td.tid
                                                               WHERE td.tid IS NULL")));
        }
        elseif ($response->getResponseState() == DATA_MONITORING_STATE_AUTOMATED_IMPLEMENTATION ||
                $response->getResponseState() == DATA_MONITORING_STATE_MANUAL_IMPLEMENTATION) {
          $response->setDeletedResultCount(db_affected_rows(db_query("DELETE tn
                                                                      FROM {term_node} tn
                                                                      LEFT JOIN {term_data} td ON tn.tid = td.tid
                                                                      WHERE td.tid IS NULL")));
        }
      }
      break;

    case "classification_term_hierarchy_child_missing":
      if ($response->getId() == 'delete_term_hierarchy_child_missing') {
        $result = db_query("SELECT th.*
                            FROM {term_hierarchy} th
                            LEFT JOIN {term_data} td ON td.tid = th.tid
                            WHERE td.tid IS NULL");
        $note = ($response->getResponseState() == DATA_MONITORING_STATE_PREVIEW) ? t('Will be deleted.') : t('Record deleted');
        while ($row = db_fetch_array($result)) {
          $response->addModifiedRecordToProcessLog($row, $note);
        }
        if ($response->getResponseState() == DATA_MONITORING_STATE_PREVIEW) {
          $response->setDeletedResultCount(db_result(db_query("SELECT COUNT(th.tid)
                                                    FROM {term_hierarchy} th
                                                    LEFT JOIN {term_data} td ON td.tid = th.tid
                                                    WHERE td.tid IS NULL")));
        }
        elseif ($response->getResponseState() == DATA_MONITORING_STATE_AUTOMATED_IMPLEMENTATION ||
                $response->getResponseState() == DATA_MONITORING_STATE_MANUAL_IMPLEMENTATION) {
          $response->setDeletedResultCount(db_affected_rows(db_query("DELETE th
                                                           FROM {term_hierarchy} th
                                                           LEFT JOIN {term_data} td ON td.tid = th.tid
                                                           WHERE td.tid IS NULL")));
        }
      }
      break;

    case 'classification_term_relation_missing_terms':
      if ($response->getId() == 'delete_term_relation') {
        $result = db_query("SELECT tr.*, td.name AS term1, td2.name AS term2
                            FROM {term_relation} tr
                            LEFT JOIN {term_data} td ON td.tid = tr.tid1
                            LEFT JOIN {term_data} td2 ON td2.tid = tr.tid2
                            WHERE td.tid IS NULL
                            OR td2.tid IS NULL");
        $note = ($response->getResponseState() == DATA_MONITORING_STATE_PREVIEW) ? t('Will be deleted.') : t('Record deleted');
        while ($row = db_fetch_array($result)) {
          $response->addModifiedRecordToProcessLog($row, $note);
        }
        if ($response->getResponseState() == DATA_MONITORING_STATE_PREVIEW) {
          $response->setDeletedResultCount(db_result(db_query("SELECT COUNT(tr.trid)
                                                               FROM {term_relation} tr
                                                               LEFT JOIN {term_data} td ON td.tid = tr.tid1
                                                               LEFT JOIN {term_data} td2 ON td2.tid = tr.tid2
                                                               WHERE td.tid IS NULL
                                                               OR td2.tid IS NULL")));
        }
        elseif ($response->getResponseState() == DATA_MONITORING_STATE_AUTOMATED_IMPLEMENTATION ||
                $response->getResponseState() == DATA_MONITORING_STATE_MANUAL_IMPLEMENTATION) {
          $response->setDeletedResultCount(db_affected_rows(db_query("DELETE tr
                                                                      FROM {term_relation} tr
                                                                      LEFT JOIN {term_data} td ON td.tid = tr.tid1
                                                                      LEFT JOIN {term_data} td2 ON td2.tid = tr.tid2
                                                                      WHERE td.tid IS NULL
                                                                      OR td2.tid IS NULL")));
        }
      }
      break;

    case "classification_term_relation_has_type_term_relation_missing":
      if ($response->getId() == 'delete_term_relation_has_type') {
        $result = db_query("SELECT trht.*
                            FROM {term_relation_has_type} trht
                            LEFT JOIN {term_relation} tr ON trht.trid = tr.trid
                            WHERE tr.trid IS NULL");
        $note = ($response->getResponseState() == DATA_MONITORING_STATE_PREVIEW) ? t('Will be deleted.') : t('Record deleted');
        while ($row = db_fetch_array($result)) {
          $response->addModifiedRecordToProcessLog($row, $note);
        }
        if ($response->getResponseState() == DATA_MONITORING_STATE_PREVIEW) {
          $response->setDeletedResultCount(db_result(db_query("SELECT COUNT(trht.trid)
                                                    FROM {term_relation_has_type} trht
                                                    LEFT JOIN {term_relation} tr ON trht.trid = tr.trid
                                                    WHERE tr.trid IS NULL")));
        }
        elseif ($response->getResponseState() == DATA_MONITORING_STATE_AUTOMATED_IMPLEMENTATION ||
                $response->getResponseState() == DATA_MONITORING_STATE_MANUAL_IMPLEMENTATION) {
          $response->setDeletedResultCount(db_affected_rows(db_query("DELETE trht
                                                           FROM {term_relation_has_type} trht
                                                           LEFT JOIN {term_relation} tr ON trht.trid = tr.trid
                                                           WHERE tr.trid IS NULL")));
        }
      }
      break;

    case "classification_hierarchy_loop":
      if ($response->getId() == 'classification_update_hierarchy') {
        $sql = "SELECT td.tid, td.name, td.vid, v.module, th.parent
                FROM {term_data} td
                INNER JOIN {term_hierarchy} th ON td.tid = th.tid
                INNER JOIN {vocabulary} v ON td.vid = v.vid
                WHERE td.tid = th.parent
                AND td.tid > %d
                ORDER BY td.tid ASC";
        $max = db_result(db_query("SELECT COUNT(td.tid)
                FROM {term_data} td
                INNER JOIN {term_hierarchy} th ON td.tid = th.tid
                INNER JOIN {vocabulary} v ON td.vid = v.vid
                WHERE td.tid = th.parent"));
      }
      break;

    case "classification_hierarchy_missing":
      if ($response->getId() == 'classification_create_hierarchy_move_to_lost_found') {
        $sql = "SELECT td.tid, td.name, td.vid, v.module, th.parent
                FROM {term_data} td
                LEFT JOIN {term_hierarchy} th ON td.tid = th.tid
                INNER JOIN {vocabulary} v ON td.vid = v.vid
                WHERE th.tid IS NULL
                AND td.tid > %d
                ORDER BY td.tid ASC";
        $max = db_result(db_query("SELECT COUNT(td.tid)
                FROM {term_data} td
                LEFT JOIN {term_hierarchy} th ON td.tid = th.tid
                INNER JOIN {vocabulary} v ON td.vid = v.vid
                WHERE th.tid IS NULL"));
      }
      break;

    case "classification_parent_missing":
      if ($response->getId() == 'classification_update_hierarchy_move_to_lost_found') {
        $sql = "SELECT td.tid, td.name, td.vid, v.module, th.parent
                FROM {term_data} td
                INNER JOIN {term_hierarchy} th ON td.tid = th.tid
                INNER JOIN {vocabulary} v ON td.vid = v.vid
                LEFT JOIN {term_data} td2 ON th.parent = td2.tid
                WHERE th.parent > 0
                AND td2.tid IS NULL
                AND td.tid > %d
                ORDER BY td.tid ASC";
        $max = db_result(db_query("SELECT COUNT(td.tid)
                FROM {term_data} td
                INNER JOIN {term_hierarchy} th ON td.tid = th.tid
                INNER JOIN {vocabulary} v ON td.vid = v.vid
                LEFT JOIN {term_data} td2 ON th.parent = td2.tid
                WHERE th.parent > 0
                AND td2.tid IS NULL"));
      }
      break;

    case "classification_parent_elsewhere":
      if ($response->getId() == 'classification_copy_ancestry_move_to_lost_found') {
        $sql = "SELECT td.tid, td.name, td.vid, v.module, th.parent
                FROM {term_data} td
                INNER JOIN {term_hierarchy} th ON td.tid = th.tid
                INNER JOIN {vocabulary} v ON td.vid = v.vid
                LEFT JOIN {term_data} td2 ON th.parent = td2.tid
                WHERE td.vid <> td2.vid
                AND td.tid > %d
                ORDER BY td.tid ASC";
        $max = db_result(db_query("SELECT COUNT(td.tid)
                FROM {term_data} td
                INNER JOIN {term_hierarchy} th ON td.tid = th.tid
                INNER JOIN {vocabulary} v ON td.vid = v.vid
                LEFT JOIN {term_data} td2 ON th.parent = td2.tid
                WHERE td.vid <> td2.vid"));
      }
      break;

    case "classification_term_blank":
      if ($response->getId() == 'classification_update_term_name') {
        $sql = "SELECT td.tid, td.name, td.vid, v.module
                FROM {term_data} td
                INNER JOIN {vocabulary} v ON td.vid = v.vid
                WHERE (td.name = '' OR td.name IS NULL)
                AND td.tid > %d
                ORDER BY td.tid ASC";
        $max = db_result(db_query("SELECT COUNT(td.tid)
                FROM {term_data} td
                INNER JOIN {vocabulary} v ON td.vid = v.vid
                WHERE (td.name = '' OR td.name IS NULL)"));
      }
      break;
  }

  if (!empty($sql) && !empty($max)) {
    $batch = array(
      'title' => t('Processing response procedure'),
      'init_message' => t('The response process could take a while, please do not close your browser...'),
      'error_message' => t('The response process has encountered an error.'),
      'progress_message' => t('Processing... @percentage% complete. Procedures can take several minutes to complete please do not close your browser.'),
      'progressive' => $response->getBatchProgressive(),
      'destination' => $response->getBatchDestination(),
    );
    if ($response->getResponseState() == DATA_MONITORING_STATE_AUTOMATED_IMPLEMENTATION ||
        $response->getResponseState() == DATA_MONITORING_STATE_MANUAL_IMPLEMENTATION) {
      $batch['title'] = t('Implementing response procedure');
    }
    elseif ($response->getResponseState() == DATA_MONITORING_STATE_PREVIEW) {
      $batch['title'] = t('Simulating response procedure');
    }
    $batch['operations'][] = array('_classification_batch_process_monitor_response', array($sql, $max, $monitor, $response));
    $batch['finished'] = '_classification_batch_process_monitor_response_finished';
    $batch['file'] = drupal_get_path('module', 'classification') . '/data_monitoring/classification.data_monitoring.inc';
    batch_set($batch);
    // Prevent progress bar from being used when not explicity set
    if (empty($batch['progressive']) || $batch['progressive'] === FALSE) {
      $batch =& batch_get();
      $batch['progressive'] = FALSE;
      unset($batch['progress_message']);
    }
    if (!isset($batch['destination'])) $batch['destination'] = NULL;
    batch_process($batch['destination']);
  }
}

/**
 * Batch process for multiple data monitoring response routines.
 *
 * @param $sql
 * @param $max
 * @param $monitor
 * @param $response
 * @param $context
 */
function _classification_batch_process_monitor_response($sql, $max, $monitor, $response, &$context) {

  // We set limit for number of terms to process at a time to one term because one term could include
  // thousands of descendants, and we want to give useful feedback to user about progress as soon as we can.
  $limit = 1;
  // Initialise sandbox if doesn't exist.
  if (empty($context['sandbox'])) {
    $context['sandbox']['sql'] = $sql;
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_term'] = 0;
    $context['sandbox']['max'] = $max;
    $context['sandbox']['vocabularies'] = array(); // Stores root count and current arbitrary parent for vocabularies.
    $context['sandbox']['lost_found'] = NULL;
    $context['sandbox']['parents'] = NULL;

    $context['results']['monitor'] = &$monitor;
    $context['results']['response'] = &$response;
    $context['results']['errors'] = array();
  }

  // TODO: Transactions?

  $result = db_query_range($context['sandbox']['sql'], $context['sandbox']['current_term'], 0, $limit);

  while ($term = db_fetch_array($result)) {

    // Update progress.
    $context['sandbox']['progress']++;
    $context['sandbox']['current_term'] = $term['tid'];

    $message_vars = array(
      '%term_name' => $term['name'],
      '@term_tid' => $term['tid'],
      '!count' => $context['sandbox']['progress'],
      '@max' => $context['sandbox']['max'],
    );
    // Add unmodified original term data to process log.
    $context['results']['response']->addOriginalRecordToProcessLog($term);

    // Act depending on type of vocabulary and monitor.
    // If monitor indicates hierarchy loop and classification is not from the taxon description module
    // then give the affected term a new 'arbitrary' parent. Default 'arbitrary' parent is root but if
    // root term limit reached a new root term will be created.
    if ($context['results']['monitor']->getId() == 'classification_hierarchy_loop' &&
        $term['module'] != 'taxon_description') {
      // Implementation.
      if ($context['results']['response']->getResponseState() == DATA_MONITORING_STATE_AUTOMATED_IMPLEMENTATION ||
          $context['results']['response']->getResponseState() == DATA_MONITORING_STATE_MANUAL_IMPLEMENTATION) {
        // Get new parent and update term.
        $created = _classification_reassign_parent($term, $context['sandbox']['vocabularies']);
        $created += $context['results']['response']->getCreatedResultCount();
        $context['results']['response']->setCreatedResultCount($created);
        unset($created);
        _classification_save_term($term);
        $context['results']['response']->addModifiedRecordToProcessLog($term,
                                         t('Parent value was updated to correct hierarchy loop.'));
        $context['message'] = t('Assigned a new parent to %term_name in order to correct hierarchy loop issue. Processed !count of @max terms.', $message_vars);
      }
      // Preview.
      elseif ($context['results']['response']->getResponseState() == DATA_MONITORING_STATE_PREVIEW) {
        $term['parent'] = t('A new parent will be assigned.');
        $context['results']['response']->addModifiedRecordToProcessLog($term,
                                         t('Parent value will be updated to correct hierarchy loop.'));
        $context['message'] = t('Simulated creation of new parent for %term_name which would correct hierarchy loop issue. Processed !count of @max terms.', $message_vars);
      }
      $context['results']['response']->incrementUpdatedResultCount();
    }
    elseif ($term['module'] == 'classification') {
      // Biological classification.
      // Get descendants in same vocabulary.
      $tree = array();
      $tree = classification_get_tree($term['vid'], $term['tid']);
      $message_vars['!descendants'] = $tree_count = count($tree);

      switch ($context['results']['monitor']->getId()) {

        case 'classification_term_blank':

          // If term has children in same vocabulary, or has content, relabel it to "Unassigned".
          // Otherwise delete the term.
          if ($tree_count > 0 || _classification_term_has_content($term['tid']) > 0) {
            // Implementation.
            if ($context['results']['response']->getResponseState() == DATA_MONITORING_STATE_AUTOMATED_IMPLEMENTATION ||
                $context['results']['response']->getResponseState() == DATA_MONITORING_STATE_MANUAL_IMPLEMENTATION) {
              $term['name'] = t('Unassigned');
              _classification_save_term($term);
              $context['results']['response']->addModifiedRecordToProcessLog($term,
                                               t('Term had children or content, name was changed to '
                                               . '<em>Unassigned</em> to correct empty name value.'));
              $context['message'] = t('Renamed term with tid %term_tid to "Unassigned" to correct blank term name issue. '
                                    . 'Processed !count of @max terms.', $message_vars);
            }
            // Preview.
            elseif ($context['results']['response']->getResponseState() == DATA_MONITORING_STATE_PREVIEW) {
              $term['name'] = t("Name will be changed to 'Unassigned'");
              $context['results']['response']->addModifiedRecordToProcessLog($term,
                                               t('Term has children or content, name will be changed to '
                                               . '<em>Unassigned</em>.'));
              $context['message'] = t('Simulated renaming of term with tid %term_tid to "Unassigned" to correct '
                                    . 'blank term name issue. Processed !count of @max terms.', $message_vars);
            }
            $context['results']['response']->incrementUpdatedResultCount();
          }
          else { // Term has no children, and no label, delete it.
            // Implementation.
            if ($context['results']['response']->getResponseState() == DATA_MONITORING_STATE_AUTOMATED_IMPLEMENTATION ||
                $context['results']['response']->getResponseState() == DATA_MONITORING_STATE_MANUAL_IMPLEMENTATION) {
              $context['results']['response']->addModifiedRecordToProcessLog($term,
                                               t('Deleted record. Term did not have children or content, term with empty name '
                                               . 'value was deleted.'));
              $context['message'] = t('Deleted term with blank name and tid %term_tid, term did not have children or '
                                    . 'content. Processed !count of @max terms.', $message_vars);
              taxonomy_del_term($term['tid']); // classification_settings_taxonomy hook deletes term relation has type records.
            }
            // Preview.
            elseif ($context['results']['response']->getResponseState() == DATA_MONITORING_STATE_PREVIEW) {
              $context['results']['response']->addModifiedRecordToProcessLog($term,
                                               t('Record will be deleted. Term does not have children or content, term with empty name '
                                               . 'value will be deleted.'));
              $context['message'] = t('Simulated deletion of term with blank name and tid %term_tid, term does not '
                                    . 'have children or content. Processed !count of @max terms.', $message_vars);
            }
            $context['results']['response']->incrementDeletedResultCount();
          }
          unset($tree);

          break;

        case "classification_parent_elsewhere":
        case "classification_parent_missing":
        case "classification_hierarchy_missing":
          // We get rid of the old parent information if it exists as we're going to give the term a brand new parent.
          if (isset($term['parent'])) unset($term['parent']);
          // Implementation.
          if ($context['results']['response']->getResponseState() == DATA_MONITORING_STATE_AUTOMATED_IMPLEMENTATION ||
              $context['results']['response']->getResponseState() == DATA_MONITORING_STATE_MANUAL_IMPLEMENTATION) {
            if (empty($context['sandbox']['lost_found'])) {
              $context['sandbox']['lost_found'] = _classification_get_lost_found_vocabulary();
            }
            if (!is_numeric($context['sandbox']['lost_found']['vid'])) {
              // Can't get/create lost and found vocabulary, can't move term or test to see if term should be moved.
              $context['results']['response']->addModifiedRecordToProcessLog($term,
                                               t('Error: Could not initialize lost and found vocabulary. No action '
                                               . 'taken on term or its !descendants descendants.'
                                               , $message_vars));
              $context['results']['response']->incrementRemainingResultCount();
              $context['message'] = t('Processed %term_name. No action taken because lost and found vocabulary could '
                                    . 'not be initialised. Processed !count of @max terms.', $message_vars);
            }
            else {
              if ($term['vid'] == CLASSIFICATION_DELETIONS || $term['vid'] == $context['sandbox']['lost_found']['vid']) {
                // Don't move term or descendants as they're already in deleted names or lost and found.
                if ($context['results']['monitor']->getId() == 'classification_parent_elsewhere') {
                  // Get a copy of parent and reassign term to new parent copy.
                  $created = _classification_copy_parent($term, $context['sandbox']['vocabularies'], $context['sandbox']['parents']);
                  $created += $context['results']['response']->getCreatedResultCount();
                  $context['results']['response']->setCreatedResultCount($created);
                  unset($created);
                }
                if (empty($term['parent'])) {
                  // Create a new arbitrary parent.
                  $created = _classification_reassign_parent($term, $context['sandbox']['vocabularies']);
                  $created += $context['results']['response']->getCreatedResultCount();
                  $context['results']['response']->setCreatedResultCount($created);
                  unset($created);
                }
                _classification_save_term($term);
                $context['results']['response']->addModifiedRecordToProcessLog($term,
                                                 t('Updated: Term was assigned a new parent but was not moved. '
                                                 . '!descendants descendants of the term were not modified.'
                                                 , $message_vars));
                $context['results']['response']->incrementUpdatedResultCount();
                $context['message'] = t('Assigned a new parent to %term_name in order to correct a missing parent issue. '
                                      . 'Did not move term as it is in deleted names or lost and found vocabulary already. '
                                      . 'Processed !count of @max terms.', $message_vars);
              }
              else { // Update parent and move term and descendants to lost and found
                // Assign new parent to term and move term and its descendants to lost and found.
                $term['vid'] = $context['sandbox']['lost_found']['vid'];
                if ($context['results']['monitor']->getId() == 'classification_parent_elsewhere') {
                  // Get a copy of parent and reassign term to new parent copy.
                  $created = _classification_copy_parent($term, $context['sandbox']['vocabularies'], $context['sandbox']['parents']);
                  $created += $context['results']['response']->getCreatedResultCount();
                  $context['results']['response']->setCreatedResultCount($created);
                  unset($created);
                }
                if (empty($term['parent'])) {
                  // Create a new arbitrary parent.
                  $created = _classification_reassign_parent($term, $context['sandbox']['vocabularies']);
                  $created += $context['results']['response']->getCreatedResultCount();
                  $context['results']['response']->setCreatedResultCount($created);
                  unset($created);
                }
                _classification_save_term($term);
                $context['results']['response']->addModifiedRecordToProcessLog($term,
                                                 t('Updated: Term was assigned a new parent and moved to lost and found. '
                                                 . 'Term has !descendants descendants which should also have been moved.'
                                                 , $message_vars));
                $context['results']['response']->incrementUpdatedResultCount();
                // Move descendants.
                _classification_move_terms_to_lost_found_vocabulary($tree, $context['sandbox']['lost_found'], $context['results']['monitor'], $context['results']['response']);
                $context['message'] = t('Assigned a new parent to %term_name in order to correct a missing parent issue. '
                                      . 'Moved term and !descendants descendant(s) to lost and found vocabulary. '
                                      . 'Processed !count of @max terms.', $message_vars);
              }
            }
          }
          // Preview.
          elseif ($context['results']['response']->getResponseState() == DATA_MONITORING_STATE_PREVIEW) {
            if ($term['vid'] == CLASSIFICATION_DELETIONS ||
                $term['vid'] == $context['sandbox']['lost_found']['vid']) {
              // New parent will be assigned to affected term but term nor its descendants will be moved.
              $term['parent'] = t('A new parent will be assigned.');
              $context['results']['response']->addModifiedRecordToProcessLog($term,
                                               t("Updated: Term will be assigned a new parent. Term will not "
                                               . "be moved to lost and found. Term has !descendants descendants which "
                                               . "will not be modified."
                                               , $message_vars));
              $context['results']['response']->incrementUpdatedResultCount();
              $context['message'] = t('Simulated creation of new parent for %term_name in order to correct a missing parent issue. '
                                      . 'Term will not be moved as it is in deleted names or lost and found vocabulary already. '
                                      . 'Processed !count of @max terms.', $message_vars);
            }
            else { // New parent will be assigned to affected term and it and its descendants will be moved to lost and found.
              $term['parent'] = t('A new parent will be assigned.');
              $term['vid'] = t('Term will be moved to lost and found vocabulary');
              $context['results']['response']->addModifiedRecordToProcessLog($term,
                                               t("Updated: Term will be assigned a new parent and moved to lost and "
                                               . "found classification. Term has !descendants descendants which will also be moved."
                                               , $message_vars));
              $context['results']['response']->incrementUpdatedResultCount();
              // Move descendants to lost and found.
              _classification_move_terms_to_lost_found_vocabulary($tree, NULL, $context['results']['monitor'], $context['results']['response']);
              $context['message'] = t('Simulated creation of new parent for %term_name in order to correct a missing parent issue. '
                                    . 'Simulated move of term and !descendants descendant(s) to lost and found vocabulary. '
                                    . 'Processed !count of @max terms.', $message_vars);
            }
          }
          break;

        default:
          // Can't process record.
          $context['results']['response']->addModifiedRecordToProcessLog($term,
                                           t('Monitor is not recognised term could not be corrected. '
                                           . 'No action taken.'));
          $context['results']['response']->incrementRemainingResultCount();
          $context['message'] = t('Processed %term_name. No action was taken because the monitor was not recognised. '
                                . 'Processed !count of @max terms.', $message_vars);
          break;
      }

    }
    else {
      $context['results']['response']->addOriginalRecordToProcessLog($term);
      // Can't process record, its not in a classification nor is it a hierarchy loop issue in a taxon description vocabulary.
      $context['results']['response']->addModifiedRecordToProcessLog($term,
                                       t('Term does not match response criteria and could not be corrected. No action taken.'));
      $context['results']['response']->incrementRemainingResultCount();
      $context['message'] = t('Processed %term_name. No action was taken because the term did not match any of the response criteria. '
                            . 'Processed !count of @max terms.', $message_vars);
    }
  }

  if (!is_object($result) && $context['sandbox']['max'] > $context['sandbox']['progress']) {
    $context['results']['errors'][] = t('Batch process response query returned empty result set before reaching maximum count.');
    $context['finished'] = 1;
  }

  if ($context['sandbox']['max'] > 0) {
    // Assign proportion completed to finished - anything less than 1 will continue processing, else process will finish.
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
  else {
    $context['finished'] = 1;
  }
}

/**
 * Batch processing completed.
 *
 * @param $success
 * @param $results
 * @param $operations
 */
function _classification_batch_process_monitor_response_finished($success, $results, $operations) {
  if (!$success || $results['errors']) {
    $error_operation = reset($operations);
    $message = t('Response procedure encountered an error while processing %error_op.', array('%error_op' => $error_operation[0]));
    if (isset($results['errors'])) $message .= ' ' . t('With errors: !errors', array('!errors' => implode('; ', $results['errors'])));
    drupal_set_message($message);
    if (is_object($results['response'])) $results['response']->setCancelProcess(TRUE);
  }
  if (is_object($results['response'])) $results['response']->batchProcessComplete($results['monitor']);
}

/**
 * Pauses current monitor response batch process, if it exists, and starts new one to move terms to the lost and found vocabulary
 * @param $terms
 *     Array of terms to move.
 * @param $vocabulary
 *     Array destination vocabulary.
 * @param $response
 *     DataMonitoringResponseHandler Object
 * @param $monitor
 *     DataMonitoringMonitor Object
 */
function _classification_move_terms_to_lost_found_vocabulary(&$terms, $vocabulary, &$monitor, &$response) {

  if (!is_array($terms)) $terms = array($terms);

  // Pause current batch, if one exists, and restart at end of this function
  $batch =& batch_get();
  if (!empty($batch)) {
    $ref = $batch;
    $batch = NULL;
  }

  // Start new batch for moving descendants.
  $batch = array(
    'title' => t('Moving terms'),
    'operations' => array(array('_classification_batch_move_terms', array($terms, $vocabulary, $monitor, $response))),
    'finished' => '_classification_batch_move_terms_finished',
    'file' => drupal_get_path('module', 'classification') . '/data_monitoring/classification.data_monitoring.inc',
    'progressive' => FALSE,
  );
  if (isset($batch['operations'])) {
    batch_set($batch);
    unset($terms);
    unset($vocabulary);
    batch_process();
  }

  $sh = new DataMonitoringSessionHandler();
  $monitor = $sh->getObject('classification_batch_move_results_monitor');
  $response = $sh->getObject('classification_batch_move_results_response');
  $sh->unregisterObject('classification_batch_move_results_monitor');
  $sh->unregisterObject('classification_batch_move_results_response');

  // Restart original batch.
  if (!empty($ref)) {
    $batch = $ref;
  }

}

/**
 * Helper function to move terms to a new vocabulary.
 * @param $terms
 *     Array containing term data.
 * @param $vocabulary
 *     Array destination vocabulary.
 * @param $response
 *     DataMonitoringResponseHandler Object
 * @param $monitor
 *     DataMonitoringMonitor Object
 *
 */
function _classification_batch_move_terms($terms, $vocabulary, $monitor, $response, &$context) {

  // Set limit for number of terms to move at a time
  $limit = 1000;
  // Initialise sandbox if doesn't exist.
  if (empty($context['sandbox'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['max'] = count($terms);
    $context['sandbox']['vocabulary'] = &$vocabulary;
    $context['sandbox']['terms'] = &$terms;

    $context['results']['monitor'] = &$monitor;
    $context['results']['response'] = &$response;

  }

  $counter = 0;
  foreach ($context['sandbox']['terms'] as $i => $term) {

    // Standardise objects to array.
    // Note we may not have vid for term.
    if (is_object($term)) $term = (array) $term;

    if (isset($term['childcount'])) unset($term['childcount']);
    if (isset($term['depth'])) unset($term['depth']);
    $context['results']['response']->addOriginalRecordToProcessLog($term);

    // Implementation.
    if ($context['results']['response']->getResponseState() == DATA_MONITORING_STATE_AUTOMATED_IMPLEMENTATION ||
        $context['results']['response']->getResponseState() == DATA_MONITORING_STATE_MANUAL_IMPLEMENTATION) {
      if (empty($context['sandbox']['vocabulary']['vid']) || !is_numeric($context['sandbox']['vocabulary']['vid'])) {
        $context['results']['response']->addModifiedRecordToProcessLog($term, t('Error: Lost and found vocabulary data is missing or invalid. Term could not be moved.'));
        $context['results']['response']->incrementRemainingResultCount();
      }
      else {
        $term['vid'] = $context['sandbox']['vocabulary']['vid'];
        _classification_save_term($term);
        $context['results']['response']->addModifiedRecordToProcessLog($term, t('Updated: Term vid value was updated - term was moved to lost and found classification.'));
        $context['results']['response']->incrementUpdatedResultCount();
      }
    }
    // Preview.
    elseif ($context['results']['response']->getResponseState() == DATA_MONITORING_STATE_PREVIEW) {
      $term['vid'] = t('vid will be changed to the lost and found vocabulary');
      $context['results']['response']->addModifiedRecordToProcessLog($term, t('Updated: Term vid value will be updated - term will be moved to lost and found classification.'));
      $context['results']['response']->incrementUpdatedResultCount();
    }

    // Remove processed term.
    unset($context['sandbox']['terms'][$i]);
    // Update progress
    $counter++;
    $context['sandbox']['progress']++;
    $context['message'] = t('Processing %term, %progress of %max...', array('%term' => $term['tid'] .': ' . $term['name'], ' %progress' => $context['sandbox']['progress'], '%max' => $context['sandbox']['max']));
    // Stop when reach limit.
    if ($counter == $limit) break;
  }

  if (empty($context['sandbox']['terms']) && $context['sandbox']['max'] > $context['sandbox']['progress']) {
    $context['results']['errors'][] = t('Terms array is empty before reaching maximum count.');
    $context['finished'] = 1;
  }

  if ($context['sandbox']['max'] > 0) {
    // Assign proportion completed to finished - anything less than 1 will continue processing, else process will finish.
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
  else {
    $context['finished'] = 1;
  }

}

/**
 * Batch finished function for movng terms.
 * @param $success
 * @param $results
 * @param $operations
 * @return unknown_type
 */
function _classification_batch_move_terms_finished($success, $results, $operations) {

  if (!$success || $results['errors']) {
    $message = t('There was an error moving or previewing move of terms: !errors', array('!errors' => implode('; ', $results['errors'])));
    drupal_set_message($message, 'error', FALSE);
  }

  $sh = new DataMonitoringSessionHandler();
  $sh->registerObject($results['monitor'], 'classification_batch_move_results_monitor');
  $sh->registerObject($results['response'], 'classification_batch_move_results_response');
}

/**
 * Helper function to return lost and found vocabulary.
 *
 * If vocabulary does not exist, it will be created.
 * @return
 *     Array of vocabulary data.
 */
function _classification_get_lost_found_vocabulary() {

  static $lost_found_vocabulary;

  if (!isset($lost_found_vocabulary)) {
    $vid = unserialize(db_result(db_query("SELECT value FROM {variable} WHERE name = '%s'", 'classification_lost_found')));
    // Check to see if vocabulary exists, $lost_found_vocabulary will be FALSE if vocabulary does not exist for a numeric vid.
    if (is_numeric($vid)) $lost_found_vocabulary = taxonomy_vocabulary_load($vid, TRUE);

    if (is_object($lost_found_vocabulary)) $lost_found_vocabulary = (array) $lost_found_vocabulary;

    if (!isset($lost_found_vocabulary['vid'])) {
      // We need to create a lost and found vocabulary.
      $lost_found_vocabulary = array(
        'name' => t('Lost and found'),
        'description' => t('Temporary vocabulary containing terms that have been programmatically misplaced, and from which manual handling of these terms can be carried out.'),
        'module' => 'classification',
        'weight' => 98,
      );
      taxonomy_save_vocabulary($lost_found_vocabulary);
      variable_set('classification_lost_found', $lost_found_vocabulary['vid']);
    }
  }

  return $lost_found_vocabulary;

}

/**
 * Helper function to modify term_data and term_hierarchy without messing with term_relations etc.
 *
 * @param $term
 *     Array of term data.
 * @return
 *     Status, what action was taken on term_data, insert or update.
 */
function _classification_save_term(&$term) {

  if (!empty($term['tid'])) {
    drupal_write_record('term_data', $term, 'tid');
    $hook = 'update';
    $status = SAVED_UPDATED;
  }
  elseif (empty($term['tid']) && !empty($term['name'])) {
    drupal_write_record('term_data', $term);
    $hook = 'insert';
    $status = SAVED_NEW;
    // New record so set parent, if it's not already set.
    if (!isset($term['parent'])) $term['parent'] = 0;
  }
  // Only update parent if it has been set.
  if (isset($term['parent']) && !empty($term['tid'])) {
    db_query('DELETE FROM {term_hierarchy} WHERE tid = %d', $term['tid']);
    if (is_array($term['parent'])) {
      foreach ($term['parent'] as $parent) {
        if (is_array($parent)) {
          foreach ($parent as $tid) {
            db_query('INSERT INTO {term_hierarchy} (tid, parent) VALUES (%d, %d)', $term['tid'], $tid);
          }
        }
        else {
          db_query('INSERT INTO {term_hierarchy} (tid, parent) VALUES (%d, %d)', $term['tid'], $parent);
        }
      }
    }
    else {
      db_query('INSERT INTO {term_hierarchy} (tid, parent) VALUES (%d, %d)', $term['tid'], $term['parent']);
    }
  }

  // Invoke hook_taxonomy should others want to do something with $term.
  if (isset($hook)) {
    module_invoke_all('taxonomy', $hook, 'term', $term);
  }

  cache_clear_all();

  return $status;
}

/**
 * Helper function to create arbitrary parent if number of root terms has been exceeded.
 *
 * @param $term
 *     Array term data.
 * @param $vocabularies
 *     Array
 *       - counter
 *           Integer count of root/immediate children of arbitrary parent.
 *       - arbitrary_parent
 *           Integer tid of current arbitrary parent for a vocabularly.
 * @param $limit
 *     Integer number of root/immediate children allowed for anyone parent - determined by browser limitations.
 */
function _classification_reassign_parent(&$term, &$vocabularies, $limit = 200) {
  $created = 0;
  if (!isset($vocabularies[$term['vid']])) {
    $vocabularies[$term['vid']]['counter'] = _classification_count_roots($term['vid']);
    $vocabularies[$term['vid']]['arbitrary_parent'] = array('tid' => 0);
  }
  $vocabularies[$term['vid']]['counter']++;
  if ($vocabularies[$term['vid']]['counter'] > $limit) {
    // Number of immediate child terms in arbitrary parent exceeds limit for root terms, create new arbitrary parent.
    $vocabularies[$term['vid']]['arbitrary_parent'] = array('name' => t('Unassigned'), 'vid' => $term['vid']);
    _classification_save_term($vocabularies[$term['vid']]['arbitrary_parent']);
    // Reset counter - this is the count of immediate children (roots) in the arbitrary parent.
    // When limit is reached a new parent will be needed.
    $vocabularies[$term['vid']]['counter'] = 0;
    $created++;
  }
  // Assign parent.
  $term['parent'] = $vocabularies[$term['vid']]['arbitrary_parent']['tid'];
  return $created;
}

/**
 * Helper function to count number of root terms in a vocabulary.
 * @param $vid
 * @return Integer
 *     Count of terms with parent = 0 in a vocabulary.
 */
function _classification_count_roots($vid) {
  return db_result(db_query("SELECT COUNT(*)
                             FROM {term_data} td
                             INNER JOIN {term_hierarchy} th ON th.tid = td.tid
                             WHERE td.vid = %d AND th.parent = %d", $vid, 0));
}

/**
 * Helper function to get node count for a single term.
 * @param $tid
 *     Term data tid.
 * @return
 *     Integer, count of nodes.
 */
function _classification_term_has_content($tid) {
  return db_result(db_query("SELECT COUNT(*) FROM {term_node} tn WHERE tn.tid = %d AND tn.n > 0", $tid));
}
/**
 * Helper function to create a copy of a term's parent and reassign the term's parent value to the new parent.
 * @param $term
 *     Array of term data.
 * @param $parents
 *     Array of previously copied parents.
 */
function _classification_copy_parent(&$term, &$vocabularies, &$parents = NULL) {
  $created = 0;
  $parent = db_fetch_array(db_query('SELECT td.tid, td.name, td.description FROM {term_data} td INNER JOIN {term_hierarchy} th ON th.parent = td.tid WHERE th.tid = %d', $term['tid']));

  if (isset($parent['tid']) && !empty($parent['tid'])) {
    if (!is_null($parents) && isset($parents[$parent['tid']])) {
      // We've already created this parent so lets not create it again lets just update term.
      $term['parent'] = $parents[$parent['tid']];
    }
    else {
      // We don't have a copy of this parent so lets create one.
      $old_tid = $parent['tid'];
      unset($parent['tid']);
      $parent['vid'] = $term['vid'];
      $created = _classification_reassign_parent($parent, $vocabularies);
      _classification_save_term($parent);
      $parents[$old_tid] = $parent['tid'];
      $term['parent'] = $parent['tid'];
      $created++;
    }
  }
  return $created;
}
