<?php

/**
 * Return an array of validation functions for file upload.
 *
 * @return
 *   An array which will be used to validate the file upload
 */
function classification_import_dwc_validators() {
  $validators = array(
    'classification_import_dwc_validator_extensions' => array(array('gz')),
    'classification_import_dwc_validator_filesize' => array(variable_get('classification_import_filesize',2)),
  );

  return $validators;
}

/**
 * Return an array of error statements for failed file extension validation.
 *
 * @param $file
 *   The file object
 * @param $extensions
 *   A space-separated list of acceptable extensions
 * @return
 *   An array of error statements
 */
function classification_import_dwc_validator_extensions($file, $extensions) {
  $errors = array();

  $ext = substr($file->filename, strrpos($file->filename, '.') + 1);

  // Bypass validation for uid  = 1.
  if ($user->uid != 1) {
    if (!in_array($ext, $extensions)) {
      $errors[] = t('Sorry, the accepted file extension is gz.');
    }
  }

  return $errors;
}

/**
 * Return an array of error statements for failed file size.
 *
 * @param $file
 *   The file object
 * @param $size
 *   File size integer in bytes
 * @return
 *   An array of error statements
 */
function classification_import_dwc_validator_filesize($file, $size) {
  global $user;

  $errors = array();
  // Bypass validation for uid  = 1.
  if ($user->uid != 1) {
    if ($file->filesize < $size) {
      $errors[] = t('Sorry, your file did not have any content.');
    }
  }

  return $errors;
}

/**
 * Start the whole process when a gzipped file is submitted for checking
 */
function classification_import_dwc_check($form, &$form_state) {

  $validators = classification_import_dwc_validators();

  if($file = file_save_upload('classification_import_dwc_check', $validators)) {

    //store uploaded file if required
    _classification_import_store_copy($file);
	
    $tmp_path = file_create_path('classification_import') . '/dwc/' . time();
    file_check_directory($tmp_path,1);
    $command = "tar -xzf $file->filepath -C $tmp_path";
    $output = _classification_import_proc($command);

    //execute parsing functions here

    //remove all extracted files
    $command = "rm -rf $tmp_path";
    _classification_import_proc($command);

  }

}

/**
 * Start the whole process when a gzipped file is submitted for importing into the database
 */
function classification_import_dwc_final($form, &$form_state) {

  $validators = classification_import_dwc_validators();
  
  if ($file = file_save_upload('classification_import_dwc_final', $validators)) {

    //store uploaded file if required
    _classification_import_store_copy($file);

    $tmp_path = file_create_path('classification_import') . '/dwc/' . time();
    file_check_directory($tmp_path,1);
    $command = "tar -xzf $file->filepath -C $tmp_path";
    $output = _classification_import_proc($command);

    //execute parsing functions here

    //remove all extracted files
    $command = "rm -rf $tmp_path";
    _classification_import_proc($command);

  }

}

function _classification_import_store_copy($file) {
	if(variable_get('classification_import_retain_copy', 1)) {
	  global $user;
	  $newfile = file_create_path('classification_import') . '/dwc/' . gmdate("Y_m_d_G-i-s") . '_uid' . $user->uid . '.tar.gz';
	  file_copy($file->filepath,$newfile);
	}
}