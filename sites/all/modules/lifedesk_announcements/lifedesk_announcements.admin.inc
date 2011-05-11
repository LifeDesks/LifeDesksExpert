<?php

/**
 * Menu callback; Form builder function for image settings.
 */
function lifedesk_announcements_admin_settings() {
    $form = array();
    
    $form['lifedesk_inactive'] = array(
        '#type' => 'fieldset',
        '#title' => t('Inactive site reminder')
    );
    
    $options = array(
        1 => t('One month'),
        2 => t('Two months'),
        3 => t('Three months')
    );
    
    $form['lifedesk_inactive']['lifedesk_announcements_inactive_period'] = array(
        '#type' => 'select',
        '#title' => t('Inactive period'),
        '#description' => t('Period of time site is inactive prior to sending message to owner'),
        '#options' => $options,
        '#default_value' => variable_get('lifedesk_announcements_inactive_period', 2),
    );
    
        $message = t("Dear !user,\n\nWe noticed that it has been a while since you or your fellow members logged into your LifeDesk, !site_name at !site_url. If you are taking a break, not to worry, your site will still be here for you when you return. However, if you do not need your site anymore, remember that you can delete it. Instructions on how to do this can be found here: http://help.lifedesks.org/node/50. If you are the owner of your site, you may at any time download the code and its underlying database before deleting it.\n\nIf you are still deciding whether or not LifeDesk is the right platform for you, that is okay. To help you decide, our new features and bug fixes can be found here: http://www.lifedesks.org/newfeatures/. If you have any questions, you can use the red feedback tab on the left-hand side of your site when logged in. There is also help documentation available at all times in the upper-right corner of every page on your site.\n\nWe hope to see you back soon,\n\nThe LifeDesks Team");
    
    $form['lifedesk_inactive']['lifedesk_announcements_inactive_message'] = array(
        '#type' => 'textarea',
        '#title' => t('Reminder message'),
        '#description' => t('Body of email message sent to site owner when site inactive longer than inactive period'),
        '#default_value' => variable_get('lifedesk_announcements_inactive_message', $message)
    );
    
    return system_settings_form($form);
}