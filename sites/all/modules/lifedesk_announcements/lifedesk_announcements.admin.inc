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

    // Default inactive message text is also defined in hook_cron() - ideally it could be set once in hook_install but for now update in both places.
    $message = t("Dear !user,\n\nWe noticed that it has been a while since you or your fellow members logged into your LifeDesk, !site_name at !site_url. If you are taking a break, not to worry, your site will still be here for you when you return. However, if you do not need your site anymore instructions on how you can delete it can be found in the LifeDesks FAQ: http://help.lifedesks.org/node/50. If you are the owner of your LifeDesk, you may at any time download your site's underlying database before deleting it. Instructions on downloading your database and files can also be found in the LifeDesks FAQ: http://help.lifedesks.org/node/52.\n\nIf you have any questions please feel free to contact us via our online contact form: http://help.lifedesks.org/contact.\n\nThe LifeDesks Team");

    $form['lifedesk_inactive']['lifedesk_announcements_inactive_message'] = array(
        '#type' => 'textarea',
        '#title' => t('Reminder message'),
        '#description' => t('Body of email message sent to site owner when site inactive longer than inactive period'),
        '#default_value' => variable_get('lifedesk_announcements_inactive_message', $message)
    );

    return system_settings_form($form);
}
