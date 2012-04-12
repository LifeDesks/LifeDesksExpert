<?php

/**
 * Taxon Export export form
 */
function taxon_export_export_form(&$form_state) {
  global $base_url;
  $module_path = drupal_get_path('module', 'taxon_export') .'/';
  drupal_add_css($module_path .'css/taxon_export.css');

  $form = array(
    '#cache' => FALSE,
    '#tree' => TRUE,
  );

  $chapter = db_fetch_object(db_query("SELECT tid FROM {taxon_export_map} LIMIT 1"));
  
  if(!$chapter->tid) {
    $message = t('Sorry, before you can export your content for the Encyclopedia of Life, you must first map your chapter headings.');
    $form['message'] = array(
      '#type' => 'markup',
      '#value' => '<div class="messages custom_error">' . $message . '</div>',
    );
    return $form;
  }

  if(file_scan_directory(file_create_path('taxon_export'), '.gz')) {
    $message =  t('Regenerate your Encyclopedia of Life content partner endpoint (a URL) or download date-stamped versions. If you have not yet done so, you will need to register your endpoint with the Encyclopedia of Life !here.', array('!here' => l(t('here'), 'https://www.eol.org/content_partner/register?method=get', array('absolute' => true, 'attributes' => array('target' => '_blank')))));
  }
  else {
    $message = t('Generate an Encyclopedia of Life content partner endpoint (a URL) of your site\'s published content. Once you have created this endpoint, you will need to register your LifeDesk with the Encyclopedia of Life !here.', array('!here' => l(t('here'), 'https://www.eol.org/content_partner/register?method=get', array('absolute' => true, 'attributes' => array('target' => '_blank')))));  
  }

  $shortname = variable_get('shortname','');
  if($shortname != '') {

    $endpoint = file_scan_directory(file_create_path('taxon_export'), '.gz') ? '<div id="taxon_export_endpoint">' . t('Your EOL content partner endpoint is:') . '<br /><strong>' . $base_url . '/eol-partnership.xml.gz</strong></div>' : '';
  }

  $form['intro_message'] = array(
    '#type' => 'markup',
    '#value' => '<div class="messages status">' . $message . ' ' . $endpoint . '</div>',
  );

  if(file_scan_directory(file_create_path('taxon_export'), '.gz')) {

    $form['show_xml'] = array(
      '#type' => 'fieldset',
      '#title' => t('Date-stamped versions'),
      '#description' => t('A maximum of !max_files versions will be retained.', array('!max_files' => variable_get('taxon_export_max_files', 10))),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['show_xml']['show_folder'] = array(
      '#value' => '<p>'.taxon_export_get_xml_files().'</p>',
    );

  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Generate'),
  );

  return $form;
}

/**
 * Taxon export form validate
 */
function taxon_export_export_form_validate($form, &$form_state) {
    $unmapped_chapters = taxon_export_unmapped_chapter_check();
    if ($unmapped_chapters) {
      form_set_error('', t('You have !num chapters that are not mapped to what is expected in the EOL Content Partner Export. Some of these may be translated chapter titles. If you wanted this content to appear on EOL, you will need to !mapurl these chapters and regenerate your export.', array('!num' => $unmapped_chapters, '!mapurl' => l(t('map'), 'admin/export/taxon_export/settings'))));
    }
}

/**
 * Taxon export form submit
 */
function taxon_export_export_form_submit($form, &$form_state) {
    $out = taxon_export_eol_export();   
    taxon_export_make_file($out,true);
}

function taxon_export_unmapped_chapter_check() {
    $languages_enabled = implode(",", array_keys(locale_language_list()));
    $unmapped_chapters = db_result(db_query("SELECT COUNT(*) FROM {term_data} td 
    LEFT JOIN {taxon_export_map} tem ON tem.tid = td.tid 
    LEFT JOIN {term_hierarchy} th ON th.tid = td.tid 
    WHERE (tem.sid = 0 OR tem.sid IS NULL) AND th.parent != 0 AND td.vid = %d and td.language IN ('%s')", TAXON_DESCRIPTION_CHAPTERS, $languages_enabled));
    return $unmapped_chapters;
}

function taxon_export_get_lifedesk_name() {
    return variable_get('shortname', 'lifedesk');
}

function taxon_export_get_xml_files(){
  $xml_files = "";
  $xml_prefix = file_create_url('taxon_export');
  $xml_files_array = file_scan_directory(file_create_path('taxon_export'), '.gz');
  sort($xml_files_array);
  foreach ($xml_files_array as $file) {
    $xml_files.= '<a href='.$xml_prefix.'/'.$file->basename.'>'.$file->basename.'</a><br />';
  }
  if ($xml_files == "") $xml_files = t('Sorry, you must first export your site\'s content before a list of exports will be made available for download.');
  return $xml_files;
}

function theme_taxon_export_biblio_entry($node, $base = 'biblio', $style = 'apa', $inline = false) {
  global $base_url;
  $output = theme('biblio_style', $node, $base, $style, $inline);
  $output = str_replace('</u>', '', str_replace('<u>', '', str_replace('<a href="/', '<a href="' . $base_url . '/', $output)));
  return $output;
}

function taxon_export_eol_export() {
  global $conf;

  $lifedesk_name = taxon_export_get_lifedesk_name();

  //get the biblio style
  $biblio_style = biblio_get_style();

  //accepted image mime types
  $accepted_image_mimes = array(
    'image/bmp',
    'image/gif',
    'image/jpeg',
    'image/png',
    'image/svg+xml',
    'image/tiff',
  );

  ob_start();
  echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  echo <<<END
<response
xmlns='http://www.eol.org/transfer/content/0.3'
xmlns:xsd='http://www.w3.org/2001/XMLSchema'
xmlns:dc='http://purl.org/dc/elements/1.1/'
xmlns:dcterms='http://purl.org/dc/terms/'
xmlns:geo='http://www.w3.org/2003/01/geo/wgs84_pos#'
xmlns:dwc='http://rs.tdwg.org/dwc/dwcore/'
xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'
xmlns:lds="http://$lifedesk_name.lifedesks.org/"
xsi:schemaLocation='http://www.eol.org/transfer/content/0.3 http://services.eol.org/schema/content_0_3.xsd'>

END;

    $objects_array = array();
    $objects_data = db_query("SELECT n.nid, n.type, n.title, n.language as language, n.created as created, n.changed AS modified,
    nr.body as body,
    license,
    givenname, surname,
    tn.tid,
    td.name,
    tde.sid as chapter_id, tde.tid_chapter, td5.name as chapter_name, tde.body AS text,
    infoitem,
    il.credit, il.rights, il.exemplar, f.uid AS image_uploader, f.filemime, f.filepath, f.fid,
    td3.tid AS parent_tid,
    td3.name AS parent,
    trt.name AS relation,
    td4.name AS flag
    FROM {node} n 
    INNER JOIN {node_revisions} nr ON n.nid = nr.nid
    LEFT JOIN {creativecommons_lite} c ON c.nid = n.nid
    LEFT JOIN {users} u ON u.uid = n.uid
    LEFT JOIN {term_node} tn ON tn.nid = n.nid
    LEFT JOIN {term_data} td ON td.tid = tn.tid
    LEFT JOIN {taxon_description} tde ON tde.nid = n.nid
    LEFT JOIN {taxon_export_map} tem ON tem.tid = tde.tid_chapter
    LEFT JOIN {taxon_export_spm} spm ON spm.sid = tem.sid
    LEFT JOIN {term_data} td2 ON td2.tid = tde.tid_chapter
    LEFT JOIN {image} i ON i.nid = n.nid
    LEFT JOIN {image_lifedesk} il ON il.nid = n.nid 
    LEFT JOIN {files} f ON i.fid = f.fid
    LEFT JOIN {term_hierarchy} th ON th.tid = td.tid
    LEFT JOIN {term_data} td3 ON td3.tid = th.parent
    LEFT JOIN {term_relation} tr ON tr.tid1 = td.tid
    LEFT JOIN {term_relation_has_type} trht ON trht.trid = tr.trid
    LEFT JOIN {term_relation_types} trt ON trt.rtid = trht.rtid
    LEFT JOIN {term_data} td4 ON td4.tid = tr.tid2 
    LEFT JOIN {term_data} td5 ON td5.tid = tde.tid_chapter 
    WHERE (n.type = 'taxon_description' OR n.type = 'biblio' OR n.type = 'image') 
        AND n.status = 1
        AND tn.tid > 0
        AND (tde.tid_taxon > 0 OR tde.tid_taxon IS NULL)
        AND n.nid > 0 
        AND td.vid = ".CLASSIFICATION_PRIMARY);

$taxa_array = $nid_array = $bib_array = $chapter_array = array();
while ($data = db_fetch_array($objects_data)) {
    $taxa_array[$data['tid']]['taxon_id'] = $data['tid'];
    $taxa_array[$data['tid']]['taxon_name'] = $data['name'];
    $taxa_array[$data['tid']]['taxon_parent_tid'] = $data['parent_tid'];
    $taxa_array[$data['tid']]['taxon_parent'] = $data['parent'];
    $taxa_array[$data['tid']]['taxon_relation'][] = $data['relation'];
    $taxa_array[$data['tid']]['taxon_flag'][] = $data['flag'];
    $tid_nid = $data['tid'] . '_' . $data['nid'];
    $cid_nid = $data['chapter_id'] . '_' . $data['nid'];
    // Prevents duplication if the name has > 1 relations except biblio and taxon_description inherited from parent to children
    if (!in_array($data['nid'], $nid_array) or ($data['type'] == 'biblio' and !in_array($tid_nid, $bib_array)) or ($data['type'] == 'image') or ($data['type'] == 'taxon_description' and !in_array($cid_nid, $chapter_array)))
    {
    if($data['type'] == 'biblio') {
        $biblio_node = node_load($data['nid']);
        $biblio_formatted = theme('taxon_export_biblio_entry', $biblio_node, 'biblio', $biblio_style, false);
        $blbio_clean = strip_tags($biblio_formatted);
    }
    $taxa_array[$data['tid']]['media'][] = array(
        'object_type' => $data['type'],
        'object_nid' => $data['nid'],
        'object_title' => $data['title'],
        'object_body' => $data['body'],
        'object_language' => ($data['language']) ? substr($data['language'], 0, 2) : 'en',
        'object_created' => $data['created'],
        'object_modified' => $data['modified'],
        'license' => $data['license'],
        'user_name' => $data['surname'] . ', ' . $data['givenname'],
        'biblio' => ($data['type'] == 'biblio') ? $biblio_formatted : '',
        'biblio_clean' => ($data['type'] == 'biblio') ? $biblio_clean : '',
        'tid_chapter' => $data['tid_chapter'],
        'chapter_name' => $data['chapter_name'],
        'spm_name' => str_replace(" ","",$data['infoitem']),
        'text' => $data['text'],
        'chapter_authors' => ($data['type'] == 'taxon_description') ? _taxon_export_chapter_authors($data['chapter_id']) : '',
        'chapter_rightsholders' => ($data['type'] == 'taxon_description' && $data['license'] != 'publicdomain') ? (_taxon_export_chapter_rightsholders($data['chapter_id']) ? _taxon_export_chapter_rightsholders($data['chapter_id']) : _taxon_export_chapter_authors($data['chapter_id'])) : '',
        'image' => array(
            $data['nid'] => array(
                'image_credit' => $data['credit'],
                'image_rights' => ($data['type'] == 'image' && $data['license'] != 'publicdomain') ? $data['rights'] : '',
                'image_uploader' => $data['image_uploader'],  
                'image_filemime' => $data['filemime'],
                'image_filepath' => $data['filepath'],
                'image_fid' => $data['fid'],
            ),
        ),
        'image_exemplar' => $data['exemplar'],
        );
    }
    $nid_array[] = $data['nid'];
    if ($data['type'] == 'biblio') $bib_array[] = $tid_nid;
    if ($data['type'] == 'taxon_description') $chapter_array[] = $cid_nid;
    }

foreach ($taxa_array as $tax) {
    if (!in_array("taxonomic relationships", $tax["taxon_relation"])) {
        $scientific_name = $tax["taxon_name"];
        $common_name = "";
        $synonym_name = "";
        }
    else { 
        if (in_array("vernacular name", $tax["taxon_flag"])) {
            $scientific_name = $tax["taxon_parent"];
            $tax["taxon_id"] = $tax["taxon_parent_tid"];
            $common_name = $tax["taxon_name"];
            $synonym_name = "";
        }
        else {
            $scientific_name = $tax["taxon_parent"];
            $tax["taxon_id"] = $tax["taxon_parent_tid"];
            $common_name = "";
            $synonym_name = $tax["taxon_name"];
        }
    }
        
    $st_vocabs = array(
    'taxa',
    'deleted names',
    'taxonomic relationships',
    'taxonomic ranks',
    'vernacular language codes',
    'taxon description chapters',
    'orphan chapters',
    'names to vernacular codes'
    );

    $rel_array = array();
    foreach($tax["taxon_relation"] as $num => $rel) {
        if ($rel && !in_array($rel, $st_vocabs) && $tax["taxon_flag"][$num]) {
            $rel_array[] = $rel . ":" . $tax["taxon_flag"][$num];
        }
    }
    $rel_array_u = array_unique($rel_array);

    echo "\n<taxon>\n";
    echo "<dc:identifier>" . $lifedesk_name.":tid:" . $tax["taxon_id"] . "</dc:identifier>\n";
    echo "<dc:source>" . url("pages/" . $tax["taxon_id"], array("absolute" => TRUE)) . "</dc:source>\n";
    echo "<dwc:ScientificName>" . htmlspecialchars(trim($scientific_name)) . "</dwc:ScientificName>\n"; 
    if ($common_name != "") {
      $language_code = ($tax["taxon_flag"][array_search("names to vernacular codes", $tax["taxon_relation"])] && $tax["taxon_flag"][array_search("names to vernacular codes", $tax["taxon_relation"])] != 'vernacular name') ? preg_replace("/^[^,]*,/", "", $tax["taxon_flag"][array_search("names to vernacular codes", $tax["taxon_relation"])]) : "";
      echo "<commonName xml:lang=\"" . $language_code . "\">" . htmlspecialchars(trim($common_name)) . "</commonName>\n"; 
    }
    if($synonym_name != "") {
        echo "<synonym>" . htmlspecialchars(trim($synonym_name)) . "</synonym>";
    }

    //sort by object type (asc), biblio (asc), image (exemplar asc), then created (desc)
    $object_type = array();
    $image_exemplar = array();
    $created = array();
    $bib_items = array();
    foreach($tax['media'] as $key => $row) {
      $object_type[$key] = $row['object_type'];
      $image_exemplar[$key] = $row['image_exemplar']; 
      $created[$key] = $row['object_created'];
      $bib_items[$key] = $row['biblio_clean'];
    }

    array_multisort($object_type, SORT_ASC, $bib_items, SORT_ASC, $image_exemplar, SORT_ASC, $created, SORT_DESC, $tax['media']);

    $unique_images = array();
    foreach ($tax['media'] as $obj) {
    switch($obj['object_type']) {
        case "biblio":
            echo "\n<reference>";
            echo htmlspecialchars($obj["biblio"]);
            echo "</reference>\n";
            break;
        case "image":
            if(!array_key_exists($obj["object_nid"], $unique_images) && $obj["image"][$obj["object_nid"]]["image_filepath"] && file_exists(getcwd() . '/' . $obj["image"][$obj["object_nid"]]["image_filepath"]) && in_array($obj["image"][$obj["object_nid"]]["image_filemime"], $accepted_image_mimes)) {
                $unique_images[$obj["object_nid"]] = $obj["object_nid"];
                echo "\n<dataObject>\n";
                echo "<dc:identifier>" . $lifedesk_name . ":" . "nid:".$obj["object_nid"] . ":fid:".$obj["image"][$obj["object_nid"]]["image_fid"]."</dc:identifier>\n";
                echo "<dataType>http://purl.org/dc/dcmitype/StillImage</dataType>\n";
                echo "<mimeType>" . $obj["image"][$obj["object_nid"]]["image_filemime"] . "</mimeType>\n";
                echo "<agent role=\"photographer\">" . htmlspecialchars($obj["image"][$obj["object_nid"]]["image_credit"]) . "</agent>\n";
                echo "<agent role=\"publisher\">". htmlspecialchars($obj["user_name"]) . "</agent>\n";
                echo "<dcterms:created>" . gmdate("Y-m-j G:H:s", $obj["object_created"]) . "</dcterms:created>\n";
                echo "<dcterms:modified>" . gmdate("Y-m-j G:H:s", $obj["object_modified"]) . "</dcterms:modified>\n";
                echo "<dc:title xml:lang=\"" . $obj['object_language'] . "\">" . htmlspecialchars($obj["object_title"]) . "</dc:title>\n";
                echo "<license>http://creativecommons.org/licenses/". $obj["license"] . "/3.0/</license>\n";
                if($obj["image"][$obj["object_nid"]]["image_rights"]) echo "<dcterms:rightsHolder>" . htmlspecialchars($obj["image"][$obj["object_nid"]]["image_rights"]) . "</dcterms:rightsHolder>\n";
                echo "<dc:source>" . url("node/" . $obj["object_nid"], array("absolute" => TRUE)) . "</dc:source>\n";
                echo "<dc:description xml:lang=\"" . $obj['object_language'] . "\">" . htmlspecialchars($obj["object_title"]) . "&lt;br&gt;" . htmlspecialchars($obj["object_body"]) . "</dc:description>\n"; //explicitly set as title once again because this what EOL uses for caption
                echo "<mediaURL>" . url("image/view/" . $obj["object_nid"] . "/_original", array("absolute" => TRUE)) . "</mediaURL>\n";
                echo "</dataObject>\n";
            }
            break;
        case "taxon_description":
            if ($obj["spm_name"] == "") break;
            echo "\n<dataObject>\n";
            echo "<dc:identifier>" . $lifedesk_name . ":nid:" . $obj["object_nid"] . ":tid_chapter:" . $obj["tid_chapter"] . "</dc:identifier>\n";
            echo "<dataType>http://purl.org/dc/dcmitype/Text</dataType>\n";
            foreach($obj['chapter_authors'] as $author) {
              echo "<agent role=\"author\">" . htmlspecialchars($author)  . "</agent>\n";   
            }
            echo "<agent role=\"compiler\">". htmlspecialchars($obj["user_name"]) . "</agent>\n";
            echo "<dcterms:created>" . gmdate("Y-m-j G:H:s", $obj["object_created"])  . "</dcterms:created>\n";
            echo "<dcterms:modified>" . gmdate("Y-m-j G:H:s", $obj["object_modified"])  . "</dcterms:modified>\n";
            echo "<dc:title xml:lang=\"" . $obj['object_language'] . "\">" . htmlspecialchars($obj["chapter_name"]) . "</dc:title>\n";
            echo "<license>http://creativecommons.org/licenses/". $obj["license"] . "/3.0/</license>\n";
            if($obj['chapter_rightsholders']) echo "<dcterms:rightsHolder>" . htmlspecialchars(implode("; ", $obj['chapter_rightsholders'])) . "</dcterms:rightsHolder>\n";
            echo "<dc:source>" . url("pages/" . $tax["taxon_id"], array("absolute" => TRUE)) . "</dc:source>\n";
            echo "<subject>http://rs.tdwg.org/ontology/voc/SPMInfoItems#" . $obj["spm_name"] . "</subject>\n";
            echo "<dc:description xml:lang=\"" . $obj['object_language'] . "\">" . htmlspecialchars($obj["text"]) . "</dc:description>\n";
            echo "</dataObject>\n";
            break; 
        }
    }

/*  NOTE: additionalInformation MUST go between references and all DataObjects. So, this will not be easy to fix.

    if (!empty($rel_array_u)) {
        echo "\n<additionalInformation>\n";
            foreach ($rel_array_u as $r) {
                $r_split = split(":", $r);
                $r_vocab = str_replace(' ', '', ucwords($r_split[0]));
                echo "<lds:" . $r_vocab . ">" . htmlspecialchars($r_split[1]) . "</lds:" . $r_vocab . ">\n";
            }           
        echo "</additionalInformation>\n";
    }
*/

    echo "</taxon>\n";
    }
    echo "</response>";
    
    $out = ob_get_contents();
    ob_end_clean();
    $output = _taxon_export_clean_up($out);
    return $output;
}

function _taxon_export_chapter_authors($sid) {
    $authors = array();
    $authors_qry = db_query("
        SELECT 
          tdcd.name
        FROM 
          {taxon_description_contributor_data} tdcd 
        INNER JOIN 
          {taxon_description_contributor} tdc ON (tdcd.cntr_id = tdc.cntr_id)
        WHERE tdc.sid = %d
        ORDER BY
          tdc.rank", $sid);
    while($authors_result = db_fetch_object($authors_qry)) {
        $authors[] = $authors_result->name;
    }
    return $authors;    
}

function _taxon_export_chapter_rightsholders($sid) {
    $rightsholders = array();
    $rightsholders_qry = db_query("
        SELECT 
          tdrd.name
        FROM 
          {taxon_description_rightsholder_data} tdrd 
        INNER JOIN 
          {taxon_description_rightsholder} tdr ON (tdrd.rts_id = tdr.rts_id)
        WHERE tdr.sid = %d
        ORDER BY
          tdr.rank", $sid);
    while($rightsholders_result = db_fetch_object($rightsholders_qry)) {
        $rightsholders[] = $rightsholders_result->name;
    }
    return $rightsholders;  
}

function taxon_export_make_file($out,$message=false) {

    $constant_name = taxon_export_get_lifedesk_name() . '_last_taxonexport.xml.gz';
    $currname = gmdate("Ymd_Gi_") . taxon_export_get_lifedesk_name() . '_taxonexport.xml.gz';
    file_save_data(gzencode($out), 'taxon_export/'.$currname, FILE_EXISTS_REPLACE);
    file_save_data(gzencode($out), 'taxon_export/'.$constant_name, FILE_EXISTS_REPLACE);
    if($message) {
        drupal_set_message(t('Encyclopedia of Life content partner export successfully produced.'), 'status');
        drupal_goto('admin/export/taxon_export/export');    
    }
}

function _taxon_export_clean_up($output) {
    $find = array(
      '&amp;nbsp;',
      '&nbsp;',
      '&amp;shy;',
      '&shy;'
    );
    $replace = array(
      ' ',
      ' ',
      '',
      '',
    );
    $output = str_ireplace($find, $replace, $output);
    $output = preg_replace('#[\r\n\t\x0B\001-\020]#', ' ', $output); //Replace control characters ^A (\001), ^G (\007), ^H (\008), ^P (\020) using octal values http://www.robelle.com/smugbook/ascii.html
    $output = trim($output);
    return $output;
}