<?php

/**
* Implementation of hook_settings().
*/
function classification_notifications_settings() {

  $form['classifications_notifications_mail'] = array(
	'#type' => 'textarea',
	'#title' => t('Classification notifications message'),
	'#default_value' => "Dear !user,\n\nIn a regular, automated check of your LifeDesk !site_name at !site_url, we have detected at least one deleted taxonomic name that has associated content. This may or may not be a problem.\n\nYou have several options to resolve this conflict:\n\n1. If the name was deleted in error and you have permission to access the classification editor located at !classification_editor, you can restore the name by dragging/dropping from the Alternative \"Deleted Names\" classification on the right to the main classification on the left. You can also visit your site's classification logs at !classification_logs\n\n2. You can move (re-associate) the content with a taxonomic name that has not been deleted. Use your account interface's \"Orphaned Content\" tool at !ocmf\n\n3. If you no longer need the content, you can use this same \"Orphaned Content\" tool to delete it.\n\nYou will periodically receive this notice as long as there are deleted taxonomic names in your Deleted Names classification for which you have associated content. To disable this notification, visit your account at !user_url and deselect \"Receive email notification when content you submitted becomes orphaned when its associated taxonomic name is deleted\" checkbox under \"Notifications\".\n\nTo ask a question about this notification, please visit !help_url.\n\nThe deleted taxonomic names are as follows:\n\n!taxa\n\n- The LifeDesks Team",
  );

  $form['classification_notifications_cron_interval'] = array(
    '#type' => 'select',
    '#title' => t('Classification notifications frequency'),
    '#description' => t('Adjust the frequency in days notifications are sent to affected users.'),
    '#default_value' => variable_get('classification_notifications_cron_interval', 7),
    '#options' => array(7 => 7, 14 => 14, 21 => 21),
  );

  return system_settings_form($form);
}