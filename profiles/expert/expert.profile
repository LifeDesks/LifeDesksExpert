<?php
/*
  You must include the crud.inc file in your profile.
  The file is part of the install_profile_api module.
*/
include_once('includes/locale.inc');
include_once('includes/crud.inc');
require_once('profiles/default/default.profile');

/**
 * Details about the profile.
 */
function expert_profile_details() {
    return array (
        'name' => 'Expert',
        'description' => 'Installation profile for Expert LifeDesks.',
      );
}

/*
 * List of modules to install, in order of dependency.
 */
function expert_profile_modules() {
    return array (
        //Drupal core required modules are automatically installed
        //Drupal core optional modules
        'color',
        'comment',
        'dblog',
        'locale',
        'menu',
        'php',
        'profile',
        'search',
        'taxonomy',
        'translation',
        'trigger',
        'upload',
        //LifeDesk required modules
        'ajax_logo_preview',
        'administerusersbyrole',
        'apachesolr',
        'apachesolr_image',
        'apachesolr_search',
        'biblio',
        'biblio_pm',
        'captcha',
        'classification',
        'classification_biblio',
        'classification_export',
        'classification_import',
        'classification_notifications',
        'classification_settings',
        'classification_slickgrid',
        'cmf',
        'compare_schema_required',
        'config_perms',
        'cooliris_lifedesk_gallery',
        'creativecommons_lite',
        'ctm',
        'data_monitoring',
        'excel_reader',
        'front_page',
        'i18n',
        'image',
        'image_gallery',
        'image_lifedesk',
        'invite',
        'invite_stats',
        'jquery_ui',
        'jquery_update',
        'jstree',
        'lifedesk_alter',
        'lifedesk_announcements',
        'lifedesk_biblio',
        'lifedesk_foot',
        'lifedesk_front_page',
        'lifedesk_greenbar',
        'lifedesk_support',
        'manage_site',
        'members',
        'menu_admin_per_menu',
        'modify_account',
        'oai2',
        'ocmf',
        'opensearch',
        'opensearch_token',
        'override_node_options',
        'path_redirect',
        'permissions_api',
        'piclens',
        'protect_critical_users',
        'recaptcha',
        'role_delegation',
        'simplemappr',
        'smtp',
        'taxon_description',
        'taxon_desc_form_fold',
        'taxon_export',
        'taxon_list',
        'taxon_search',
        'taxonpage',
        'term_display',
        'token',
        'transaction',
        'transliteration',
        'update',
        'views',
        'wysiwyg',
      );
}

/**
 * Tasks to perform during installation.
 */
function expert_profile_tasks(&$task, $url) {
  if($task == 'profile') {
    /**
     * include default profile tasks to install default content types and variables: 
     * use with care and override as necessary.
     */
    default_profile_tasks($task, $url);
  
    /**
     * Install node types.
     */
    //no custom node types to add here (default page added from default profile).
  
    /**
     * Set variables.
     */
    variable_set('override_node_options_pocollapsed',0);
    variable_set('clean_url', '1');
    variable_set('css_js_query_string', 'F0000000000000000000');
    variable_set('date_default_timezone', '-18000');
    variable_set('file_directory_temp', '/tmp');
    variable_set('filter_html_1', 1);
    variable_set('install_task', 'done');
    variable_set('user_picture_dimensions', '200x200');
    variable_set('javascript_parsed', array ());
    variable_set('menu_default_node_menu', 'primary-links');
    variable_set('menu_primary_links_source', '');
    variable_set('menu_secondary_links_source', 'secondary-links');
    variable_set('node_options_forum', array (
      'status',
    ));
    variable_set('node_options_page', array (
      'status',
    ));
    variable_set('user_email_verification', true);
  
    //creativecommons settings
    variable_set('creativecommons_lite_default_license','by-nc');
    variable_set('creativecommons_lite_mandatory',1);
    variable_set('creativecommons_lite_jurisdiction',0);
    variable_set('creativecommons_lite_icon_style','buttons_small');
    variable_set('creativecommons_lite_no_text',1);
    variable_set('creativecommons_lite_license_options',array (
      'by' => 'by',
      'by-nc' => 'by-nc',
      'by-nc-sa' => 'by-nc-sa',
      'by-sa' => 'by-sa',
      'publicdomain' => 'publicdomain',
    ));
  
    //performance variables
    variable_set('cache',1);
    variable_set('page_compression',1);
    variable_set('preprocess_css',1);
    variable_set('preprocess_js',1);
  
    //error reporting & logging
    variable_set('site_404','');
    variable_set('site_403','');
    variable_set('error_level',0);
    variable_set('dblog_row_limit',10000);
  
    //file upload limits & user quota for file upload
    variable_set('upload_uploadsize_default', '5');  //default maximum file size per upload is set to 5MB
    variable_set('upload_usersize_default', '10240');  //default total file size per user is det to 10GB

    variable_set('image_max_upload_size', '5120'); //image file upload limit as 5MB
    variable_set('image_sizes', array ( '_original' => array ( 'label' => 'Original', 'operation' => 'scale', 'width' => '', 'height' => '', 'link' => '2', ), 'thumbnail' => array ( 'label' => 'Thumbnail', 'operation' => 'scale', 'width' => '100', 'height' => '75', 'link' => '1', ), 'preview' => array ( 'label' => 'Preview', 'operation' => 'scale', 'width' => '460', 'height' => '345', 'link' => '1', ), ));
    variable_set("creativecommons_lite_image", 1);

    //simplemappr settings
    variable_set('simplemappr_sizes', array ( '_original' => array ( 'label' => 'Original', 'operation' => 'scale', 'width' => '', 'height' => '', 'link' => '2', ), 'thumbnail' => array ( 'label' => 'Thumbnail', 'operation' => 'scale', 'width' => '300', 'height' => '150', 'link' => '1', ), 'preview' => array ( 'label' => 'Preview', 'operation' => 'scale', 'width' => '600', 'height' => '300', 'link' => '1', ), 'svg' => array ( 'label' => 'svg', 'operation' => '', 'width' => '600', 'height' => '300', 'link' => '2', ), ));
    variable_set("creativecommons_lite_simplemappr", 1);
    
    //image & image gallery settings
    $image_node_types  = variable_get('image_node_types', array());
    if (empty($image_node_types['image'])) {
      $image_node_types['image'] = array(
    'title' => 'Image',
    'fieldname' => 'images',
    'image_selection' => 'preview', // best choice I think; can be changed in admin menu
    'imagecache_preset' => '',
      );
      variable_set('image_node_types', $image_node_types);
    }
  
    //jquery update 
    variable_set('jquery_update_replace',true);
    variable_set('jquery_update_compression_type', 'min');

    //filters and allowed html
    variable_set('filter_html_1','1');
    variable_set('filter_default_format','1');
    variable_set('filter_html_help_1',0);
    variable_set('allowed_html_1','<a> <em> <strong> <cite> <code> <ul> <ol> <li> <dl> <dt> <dd> <p> <img> <sub> <sup> <p> <br> <table> <thead> <tbody> <tr> <td>');
    
    //comment variables for content types
    $content_types = array('image','taxon_description','simplemappr');
  
    foreach($content_types as $content_type) {
      variable_set('comment_'.$content_type,'2');
      variable_set('comment_default_mode_'.$content_type,'3');
      variable_set('comment_default_order_'.$content_type,'1');
      variable_set('comment_default_per_page_'.$content_type,'30');
      variable_set('comment_controls_'.$content_type,'3');
      variable_set('comment_anonymous_'.$content_type,0);
      variable_set('comment_subject_field_'.$content_type,'1');
      variable_set('comment_preview_'.$content_type,'1');
      variable_set('comment_form_location_'.$content_type,'1');
    }
    
    //path redirect default settings
    variable_set('path_redirect_redirect_warning',0);
    variable_set('path_redirect_allow_bypass',0);
    variable_set('path_redirect_auto_redirect',1);
    variable_set('path_redirect_purge_inactive',0);
    variable_set('path_redirect_default_status',301);

    //frontpage variables
    $welcome_anonymous = '<h3 class="h">' . t('Welcome') . '</h3>' . "\n";
    $welcome_anonymous .= '<p>' . t('This is a new LifeDesk where participants will edit and maintain their classification, upload images, maintain bibliographic resources, among other activities.') . '</p>';

    variable_set('lifedesk_front_page_welcome_anonymous' ,array('body' => $welcome_anonymous, 'format' => 1));

    $welcome_authenticated = '<h3 class="h">' . t('Welcome') . '</h3>' . "\n";
    $welcome_authenticated .= '<p>' . t('You are free to edit this welcome message for your new LifeDesk. You have two different welcome messages: one for anonymous visitors and a second for authenticated participants.') . '</p>' . "\n";
    $welcome_authenticated .= '<p>' . t('Names management and a hierarchical classification are core to this version of LifeDesk. Once you import your working classification, the tree at top-right and on each of your taxon pages may be used as a navigational structure. Recent images and bibliographic entries will also appear on anonymous and authenticated versions of your frontpage. As you create content, tag it with names from your classification to enrich your taxon pages.') . '</p>' . "\n";

    variable_set('lifedesk_front_page_welcome_authenticated',array('body' => $welcome_authenticated, 'format' => 1));

    /**
     * Set outgoing email messages
     */
    $message_approval =  t("A new user with username: !username, and email: !mailto requested an account on your LifeDesk !site. Activation of !username's account is pending your approval.") . "\n\n" . t("To view and activate the account, log in to your LifeDesk: !uri and edit !username's profile (!edit_uri), selecting 'active' under account status.") . "\n\n" . t("For further information on activating new user accounts in your LifeDesk, please visit http://help.lifedesks.org/members/new.");

  variable_set('user_mail_register_pending_approval_admin_subject', t('A new user (!username) has requested an account on your !site LifeDesk.'));
  variable_set('user_mail_register_pending_approval_admin_body',$message_approval);

    /**
     * Set SMTP settings
     */
    variable_set("smtp_on", "1");
    variable_set("smtp_host", "10.19.19.213");
    variable_set("smtp_hostbackup", "128.128.172.10");
    variable_set("smtp_port", "25");
    variable_set("smtp_protocol", "standard");
    variable_set("smtp_username","");
    variable_set("smtp_password", "");
    variable_set("smtp_from", "");
    variable_set("smtp_fromname", "");
    variable_set("smtp_debugging", 0);
    variable_set("smtp_library", "sites/all/modules/smtp/smtp.module");

    /**
     * Apache Solr search & OpenSearch
     */
    variable_set('search_cron_limit', 200);
    variable_set('minimum_word_size', 3);
    variable_set('apachesolr_host', '10.19.19.19');  //change to Solr host for production env't
    variable_set('apachesolr_port', '8080');
    variable_set('apachesolr_path', '/solr/lifedesks');
    variable_set('apachesolr_cron_limit', '100');
    variable_set('apachesolr_rows', '10');
    variable_set('apachesolr_failure', 'show_no_results');
    variable_set('apachesolr_search_make_default', '1');
    variable_set('apachesolr_search_spellcheck', 0);
    variable_set('apachesolr_search_taxonomy_links', '1');
    $apachesolr_search = array(
    'apachesolr_search' => array(),
    );
    variable_set('apachesolr_enabled_facets', $apachesolr_search);

    /**
     * Taxon Search
     */
    variable_set('taxon_search_solr_host', '10.19.19.19');
    variable_set('taxon_search_solr_port', '8080');
    variable_set('taxon_search_solr_path', '/solr/lifedesks_taxa');
    variable_set('taxon_search_solr_rows', '10');

    /**
     * CTM (i.e.Menu Settings per Content Type) settings
     */
    $menu_links = array('primary-links'=>'primary-links');
    $ret[] = variable_set('page_menu_settings', $menu_links);

    /**
     * Invite settings
     */
    variable_set('invite_target_role_default', '3');
    variable_set('invite_target_role_administrator', '3');
    variable_set('invite_target_role_editor', '3');
    variable_set('invite_target_role_owner', '3');
    variable_set('invite_expiry', '30');
    variable_set('invite_subject', '[user-givenname] [user-surname] has sent you an invite!');
    variable_set('invite_subject_editable', '');
    $template = "Your friend, [user-givenname] [user-surname], has invited you to join [site-name] at [site-url].

    To become a member of [site-name], click the link below or paste it into the address bar of your browser.

    [join-link]

    ----------
    [invite-message-raw]";
    variable_set('invite_default_mail_template', $template);
    variable_set('invite_use_users_email', '1');
    variable_set('invite_use_users_email_replyto', '1');
    variable_set('invite_manual_from', '');
    variable_set('invite_manual_reply_to', '');
    variable_set('invite_page_title', t('Invite a friend'));

    /**
     * Biblio settings
     */
    variable_set('biblio_style','apa');
    variable_set('biblio_rss', 1);
    variable_set('lifedesk_biblio_parserurl', 'http://refman.eol.org/cgi-bin/refparser.cgi?');
    variable_set('lifedesk_biblio_pid', 'lifedesk@eol.org');

    //Juggernaut settings
    variable_set("juggernaut_host", "www.lifedesks.org");
    variable_set("juggernaut_port", 8080);
    variable_set("redis_host", "10.19.19.24");
    variable_set("redis_port", 6379);
    
    //set the inactive period check
    variable_set("lifedesk_announcements_inactive_period", 2);
 
    /**
     * Set default roles and permissions.
     */
    $permissions = array();
    //anonymous user permissions
    $permissions['anonymous user'] = array(
      'access content',
      'view revisions',
      'view uploaded files',
      'access user profiles',
      'access biblio content',
      'show export links',
      'show download links',
      'show filter tab',
      'show sort links',
      'view full text',
      'view user content list',
      'view original images',
      'view taxon description',
      'access comments',
      'inspect all votes',
      'vote on polls',
      'search content',
      'view original simplemap'
    );
    $permissions['authenticated user'] = array(
      'post comments',
      'post comments without approval',
      'cancel own vote',
      'change own username'
    );
    foreach($permissions['anonymous user'] as $perm) {
      $permissions['authenticated user'][] = $perm;
    }
    $permissions['contributor'] = array(
      'create page content',
      'delete own page content',
      'edit own page content',
      'revert revisions',
      'upload files',
      'create biblio',
      'edit own biblio entries',
      'import from file',
      'administer biblio',
      'show own download links',
      'access classification pages',
      'create images',
      'edit own images',
      'delete own images',
      'mass upload images',
      'edit captions',
      'filter and manage orphaned content',
      'override image published option',
      'override taxon_description published option',
      'override page published option',
      'override simplemappr published option',
      'create taxon description',
      'edit own taxon description',
      'delete own taxon description',
      'export template',
      'import content',
      'create poll content',
      'delete own poll content',
      'edit own poll content',
      'create simplemap',
      'edit own simplemap',
      'delete own simplemap'
    );
    foreach($permissions['authenticated user'] as $perm) {
      $permissions['contributor'][] = $perm;
    }
    $permissions['editor'] = array(
      'post comments without approval',
      'delete revisions',
      'edit any page content',
      'access administration pages',
      'administer taxonomy',
      'edit all biblio entries',
      'edit biblio authors',
      'create classification',
      'import classification',
      'edit classification',
      'delete classification',
      'export classification',
      'filter and manage site content',
      'edit any images',
      'delete any images',
      'edit any taxon description',
      'edit any poll content',
      'send invitations',
      'send mass invitations',
      'track invitations',
      'withdraw accepted invitations',
      'edit any simplemap',
      'delete any simplemap',
      'administer nodes'
    );
    foreach($permissions['contributor'] as $perm) {
      $permissions['editor'][] = $perm;
    }
    $permissions['administrator'] = array(
      'administer primary-links menu items',
      'delete any page content',
      'settings for classification',
      'share classification',
      'administer modules',
      'administer site information',
      'administer themes',
      'administer users',
      'administer user profile fields',
      'create users',
      'delete users with role contributor',
      'delete users with role editor',
      'edit users with role contributor',
      'edit users with role editor',
      'display site building menu',
      'display site configuration menu',
      'administer frontpage',
      'manage site',
      'perform backup',
      'perform export',
      'assign contributor role',
      'assign editor role',
      'delete any taxon description',
      'administer taxon description',
      'administer taxon pages',
      'export taxon content',
      'delete any poll content',
      'administer languages',
      'assign administrator role',
      'delete users with role administrator',
      'edit users with role administrator'
    );
    foreach($permissions['editor'] as $perm) {
      $permissions['administrator'][] = $perm;
    }
    $permissions['owner'] = array(
      'edit users with role owner',
      'delete site'
    );
    foreach($permissions['administrator'] as $perm) {
      $permissions['owner'][] = $perm;
    }
    
    foreach($permissions as $role => $perms) {
      $rid = install_add_role($role);
      install_set_permissions($rid, $perms);
    }
    
/************************************************************
*                            MENU                           *
************************************************************/

    // Add new menu items to action-links menu
    $items[] = array(
      'link_title' => 'My Account',
      'link_path' => 'user/%',
      'menu_name' => 'action-links',
      'module' => 'lifedesk_alter',
      'weight' => -10,
    );
    $items[] = array(
      'link_title' => 'Create Content',
      'link_path' => 'node/add',
      'menu_name' => 'action-links',
      'module' => 'lifedesk_alter',
      'weight' => -9,
    );
    $items[] = array(
      'link_title' => 'Manage Classification',
      'link_path' => 'admin/classification/biological',
      'menu_name' => 'action-links',
      'module' => 'lifedesk_alter',
      'weight' => -8,
    );
    $items[] = array(
      'link_title' => t('Invite a Friend'),
      'link_path' => 'invite',
      'menu_name' => 'action-links',
      'module' => 'lifedesk_alter',
      'weight' => -7,
    );
    $items[] = array(
      'link_title' => 'Administration',
      'link_path' => 'admin',
      'menu_name' => 'action-links',
      'module' => 'lifedesk_alter',
      'weight' => -6,
    );
    $items[] = array(
      'link_title' => 'Log out',
      'link_path' => 'logout',
      'menu_name' => 'action-links',
      'module' => 'lifedesk_alter',
      'weight' => -5,
    );

    // Add new menu items to primary-links menu
    $items[] = array(
      'link_title' => 'Image Gallery',
      'link_path' => 'image',
      'menu_name' => 'primary-links',
      'module' => 'lifedesk_alter',
      'weight' => -10,
    );
    $items[] = array(
      'link_title' => 'Taxon Pages',
      'link_path' => 'taxa',
      'menu_name' => 'primary-links',
      'module' => 'lifedesk_alter',
      'weight' => -9,
    );
    $items[] = array(
      'link_title' => 'Bibliography',
      'link_path' => 'biblio',
      'menu_name' => 'primary-links',
      'module' => 'lifedesk_alter',
      'weight' => -8,
    );
    $items[] = array(
      'link_title' => 'Members',
      'link_path' => 'members',
      'menu_name' => 'primary-links',
      'module' => 'lifedesk_alter',
      'weight' => -7,
    );

    install_menu_create_menu_items($items);

/***********************************************************
*                       INSTALL THEME                      *
***********************************************************/
    system_theme_data();
    db_query("UPDATE {system} SET status = 0 WHERE type = 'theme'");
    db_query("UPDATE {system} SET status = 1 WHERE type = 'theme' and name = '%s'", 'expert');
    variable_set('theme_default', 'expert');
    variable_set('theme_expert_settings', array (
      'expert_help' => '0',
      'zen_block_editing' => '0',
      'zen_breadcrumb' => 'yes',
      'zen_breadcrumb_separator' => ' › ',
      'zen_breadcrumb_home' => '1',
      'zen_breadcrumb_trailing' => '0',
      'zen_breadcrumb_title' => '0',
      'zen_rebuild_registry' => '0',
      'zen_wireframes' => '0',
    ));
    variable_set('theme_settings', array (
      'mission' => '',
      'default_logo' => 1,
      'logo_path' => '',
      'default_favicon' => 0,
      'favicon_path' => 'sites/all/themes/expert/favicon.ico',
      'primary_links' => 1,
      'secondary_links' => 1,
      'toggle_logo' => 1,
      'toggle_favicon' => 1,
      'toggle_name' => 1,
      'toggle_search' => 0,
      'toggle_slogan' => 0,
      'toggle_mission' => 1,
      'toggle_node_user_picture' => 0,
      'toggle_comment_user_picture' => 0,
      'toggle_primary_links' => 1,
      'toggle_secondary_links' => 1,
      'toggle_node_info_image' => 1,
      'toggle_node_info_taxon_description' => 1,
      'toggle_node_info_biblio' => 1,
      'toggle_node_info_page' => 0,
      'toggle_node_info_story' => 1,
      'logo_upload' => '',
      'favicon_upload' => '',
      'expert_help' => '0',
      'zen_block_editing' => '0',
      'zen_breadcrumb' => 'yes',
      'zen_breadcrumb_separator' => ' › ',
      'zen_breadcrumb_home' => '1',
      'zen_breadcrumb_trailing' => '0',
      'zen_breadcrumb_title' => '0',
      'zen_rebuild_registry' => '0',
      'zen_wireframes' => '0',
    ));

/************************************************************
*                           BLOCKS                          *
************************************************************/

    install_add_block('user', '0', 'expert', 1, 0, 'sidebar_first', 0, 0, 0, '0', '<none>');
    install_add_block('user', '1', 'expert', 0, 0, '', 0, 0, 0, '0', '');
    install_add_block('lifedesk_announcements', '0', 'expert', 1, -13, 'sidebar_first', 0, 0, 0, '0', '<none>');
    install_add_block('menu', 'primary-links', 'expert', 1, -12, 'sidebar_first', 0, 0, 0, '0', '<none>');
    install_add_block('lifedesk_greenbar', '0', 'expert', 1, 0, 'footer', 0, 0, 0, '0', '<none>');
    install_add_block('lifedesk_foot', '0', 'expert', 1, 0, 'footer', 0, 0, 0, '0', '<none>');
    install_add_block('menu', 'menu-action-links', 'expert', 1, -11, 'sidebar_first', 0, 0, 0, '', '<none>');

/************************************************************
*                         LANGUAGES                         *
************************************************************/

    $availableLanguages = array(
        "en" => array("English"),
        "es" => array("Spanish", "Español"),
        "fr" => array("French", "Français"),
        "pt-pt" => array("Portuguese, Portugal", "Português"),
        "pt-br" => array("Portuguese, Brazil", "Português"),
        "ro" => array("Romanian", "Română"),
        "sw" => array("Swahili", "Kiswahili"),
        "tl" => array("Tagalog", "Tagalog"),
    );
    variable_set('lifedesk_languages', $availableLanguages);
    variable_set('language_content_type_taxon_description',1);
    variable_set('i18n_required_node_taxon_description',1);
    variable_set('language_content_type_simplemappr',1);
    variable_set('i18n_required_node_simplemappr',1);
    variable_set('language_content_type_image',1);
    variable_set('i18n_required_node_image',1);
    variable_set('language_content_type_page',1);
    variable_set('i18n_required_node_page',1);
    variable_set('i18n_selection_mode', 'off');

/************************************************************
*                       EXPORTING NODES                     *
************************************************************/
    system_initialize_theme_blocks('expert');


    /**
     * Add path redirect
     */
    //add redirect -what about error handling here/during install?
    update_sql("INSERT INTO path_redirect (source,redirect) VALUES ('user/1', 'members')");
    update_sql("INSERT INTO path_redirect (source,redirect,type) VALUES ('admin/build/themes', 'admin/build/themes/settings/expert', 301)");
    update_sql("INSERT INTO path_redirect (source,redirect,type) VALUES ('admin/build/menu', 'admin/build/menu-customize/primary-links', 301)");
    update_sql("INSERT INTO path_redirect (source,redirect,type) VALUES ('admin/classification', 'admin/classification/biological', 301)");

    /**
     * Assign owner role to site creator
     */
    //get site owner uid
    $owner_uid = variable_get('site_owner_uid',1);
    //get owner role id
    $owner_rid = db_result(db_query("SELECT rid FROM role WHERE name = '%s'",'owner'));
    //update site owner to have owner role
    if($owner_uid > 1) {
      $ret = array();
      $ret[] = db_query("INSERT INTO users_roles (rid,uid) VALUES (%d, %d)", $owner_rid, $owner_uid);
    }

    /**
     * Assign administrator role to support account
     */
     //get the support uid
     $support_uid = variable_get('lifedesk_support',0);
     //get admin role id
     $admin_rid = db_result(db_query("SELECT rid FROM role WHERE name = '%s'",'administrator'));
     //update support account to have admin role
     if($support_uid > 1) {
       $ret[] = db_query("INSERT INTO users_roles (rid,uid) VALUES (%d, %d)", $admin_rid, $support_uid);
     }

/*******************************************************
*                      WYSIWYG Config                  *
*******************************************************/

    $wysiwyg_config_1 = array(
        'default' => 1,
        'user_choose' => 0,
        'show_toggle' => 0,
        'theme' => 'advanced',
        'language' => 'en',
        'buttons' => array(
          'default' => array(
              'bold' => 1,
              'italic' => 1,
              'undo' => 1,
              'redo' => 1,
              'link' => 1,
              'unlink' => 1,
              'sup' => 1,
              'sub' => 1,
              'charmap' => 1,
              'code' => 1,
              'cut' => 1,
              'copy' => 1,
              'paste' => 1,
          ),
          'paste' => array(
              'pasteword' => 1,
          ),
          'table' => array(
              'tablecontrols' => 1,
          ),
          'safari' => array(
              'safari' => 1,
          ),
        ),
        'toolbar_loc' => 'top',
        'toolbar_align' => 'left',
        'path_loc' => 'none',
        'resizing' => 1,
        'verify_html' => 1,
        'preformatted' => 0,
        'convert_fonts_to_spans' => 1,
        'remove_linebreaks' => 1,
        'apply_source_formatting' => 0,
        'paste_auto_cleanup_on_paste' => 1,
        'block_formats' => 'p,address,pre,h2,h3,h4,h5,h6,div',
        'css_setting' => 'theme',
        'css_path' => '',
        'css_classes' => '',
    );

    $wysiwyg_config_2 = array(
      'default' => 0,
      'user_choose' => 0,
      'show_toggle' => 0,
      'theme' => 'advanced',
      'language' => 'en',
      'buttons' => array(
      ),
      'toolbar_loc' => 'top',
      'toolbar_align' => 'left',
      'path_loc' => 'none',
      'resizing' => 1,
      'verify_html' => 1,
      'preformatted' => 0,
      'convert_fonts_to_spans' => 1,
      'remove_linebreaks' => 1,
      'apply_source_formatting' => 0,
      'paste_auto_cleanup_on_paste' => 1,
      'block_formats' => 'p,address,pre,h2,h3,h4,h5,h6,div',
      'css_setting' => 'theme',
      'css_path' => '',
      'css_classes' => '',
    );

      //filtered HTML
      $ret[] = db_query("INSERT INTO wysiwyg (format,editor,settings) VALUES (%d,'%s','%s')",1,'tinymce',serialize($wysiwyg_config_1)); 

      //full HTML
      $ret[] = db_query("INSERT INTO wysiwyg (format,editor,settings) VALUES (%d,'%s','%s')",2,'tinymce',serialize($wysiwyg_config_2));  

/*********************************************
*        CAPTCHA and reCAPTCHA Settings      *
*********************************************/

       variable_set('recaptcha_private_key', '6Lf-OgYAAAAAAE6itXDNjFULu2m8sH2nK73vQJLL');
       variable_set('recaptcha_public_key', '6Lf-OgYAAAAAABuvX1QVI1h1b1DsXRlkCw30UVdI');
       variable_set('recaptcha_theme', 'clean');
       variable_set('recaptcha_secure_connection', FALSE);
       variable_set('recaptcha_tabindex','');

       $ret[] = db_query("UPDATE captcha_points cp SET cp.module='recaptcha', cp.type='reCAPTCHA' WHERE cp.form_id = 'user_register'");
      
    return;
  }
}

?>
