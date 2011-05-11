<?php

/**
 * @file taxonpage_attachments.tpl.php
 * Display attachments on the taxon page
 *
 * Variables available:
 * - $attachments: An array of attachments on published nodes that includes:
 *                 type (node type); filepath; filemime; filename; filesize; timestamp
 * filemimes with icons:
 * Images: image/jpeg, image/png, image/gif, image/tiff, image/svg+xml
 * Video: video/x-msvideo, video/x-ms-wmv, video/mpeg
 * Audio: audio/mpeg, audio/x-ms-wma
 * Office: application/msword, application/vnd.ms-powerpoint, application/vnd.openxmlformats-officedocument.wordprocessingml.document, application/vnd.openxmlformats-officedocument.presentationml.presentation, application/vnd.openxmlformats-officedocument.spreadsheetml.sheet
 * Documents: application/pdf, text/plain
 * Other: application/zip, application/vnd.google-earth.kml+xml, application/vnd.google-earth.kmz, application/x-stuffit 
 */

if(!empty($attachments)) {
  global $base_path;
  $module_path = drupal_get_path('module', 'taxonpage') .'/';
  $header = array(
    array('data' => t('File')),
    array('data' => t('Size (kb)')),
    array('data' => t('Uploaded'))
  );

  foreach ($attachments as $attachment) {
    switch($attachment['filemime']) {
      case 'image/jpeg':
      case 'image/png':
      case 'image/tiff':
      case 'image/svg+xml':
      case 'image/gif':
        $icon = 'page_white_picture.gif';
      break;

      case 'video/x-msvideo':
      case 'video/x-ms-wmv':
      case 'video/mpeg':
        $icon = 'film.gif';
      break;

      case 'audio/mpeg':
      case 'audio/x-ms-wma':
        $icon = 'music.gif';
      break;

      case 'application/msword':
      case 'application/vnd.openxmlformats-officedocument.wordprocessingml.document':
        $icon = 'page_white_word.gif';
      break;

      case 'application/vnd.ms-powerpoint':
      case 'application/vnd.openxmlformats-officedocument.presentationml.presentation':
        $icon = 'page_white_powerpoint.gif';
      break;

      case 'application/vnd.ms-excel':
      case 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet':
        $icon = 'page_white_excel.gif';
      break;
 
      case 'application/pdf':
        $icon = 'page_white_acrobat.gif';
      break;

      case 'text/plain':
        $icon = 'page_white_text.gif';
      break;

      case 'application/zip':
      case 'application/x-stuffit':
        $icon = 'page_white_compressed.gif';
      break;

      case 'application/vnd.google-earth.kml+xml':
      case 'application/vnd.google-earth.kmz':
        $icon = '';
      break;

      default:
        $icon = 'page_white_text.gif';
    }

      $rows[] = array(
	    l($attachment['filename'], $attachment['filepath'], array('attributes' => array('style' => 'background-repeat:no-repeat;background-image:url(' . $base_path . $module_path . 'images/' . $icon . ');padding: 1px 5px 1px 24px;', 'target' => '_blank'))),
	    round($attachment['filesize']/1000,1),
	    gmdate("Y-m-d\TH:i:s\Z", $attachment['timestamp'])
      );

  }

  $output = theme('table', $header, $rows);

  print $output;
?>


<?php
}
?>