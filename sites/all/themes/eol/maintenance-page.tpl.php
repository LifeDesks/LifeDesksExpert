<?php
// $Id: page.tpl.php,v 1.18 2008/01/24 09:42:53 goba Exp $
$theme_path = base_path() . drupal_get_path('theme', 'eol');
global $conf;
?><!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="<?php print $language->language ?>" lang="<?php print $language->language ?>" dir="<?php print $language->dir ?>">
  <head>
    <title><?php print $head_title ?></title>
    <?php print $head ?>
    <?php print $styles ?>
    <!--[if lt IE 7]>
      <?php print phptemplate_get_ie_styles(); ?>
    <![endif]-->
  </head>
<body id="home-page"<?php print eol_body_class(); ?>>
<?php $domain = 'http://www.' . $conf['_lifedesks_domain'];?>
<!-- header-region -->
<div id="header-region">
<form id="header-search-form" method="get" accept-charset="UTF-8" action="<?php print $domain ?>/search/">
	<table cellspacing="0" cellpadding="0" border="0">
		<tbody>
			<tr>
				<td id="header-eol-logo" valign="middle" nowrap=""><img src="<?php print $theme_path ?>/images/eol.gif" style="vertical-align:middle" alt="Encyclopedia of Life" /></td>
				<td id="header-search" valign="middle" nowrap="">
          <input class="header-search-form-text" type="text" title="<?php print t('Enter the terms you wish to search for.'); ?>" value="" size="15" name="q" maxlength="128"/>
          <input class="header-search-form-submit" type="submit" value="<?php print t('Search'); ?>" />
				</td>
				<td id="header-middle" valign="middle" nowrap=""><a href="/next"><?php print t('Next LifeDesk'); ?> &gt;&gt;</a></td>
				<td id="header-lifedesk" valign="middle" nowrap="" align="right">
				<?php
				  $img = 'pencil_add.gif';
				  $link = $domain . '/create';
				  $link_title = t('Create a LifeDesk');
				  $options = array('html' => true, 'absolute' => true);
				  print l('<img src="' . $theme_path . '/images/' . $img . '" style="vertical-align:middle" alt="' . t('Help') . '" /> ' . $link_title,$link, $options);
				  print " | " . l('<img src="' . $theme_path . '/images/wand.gif" style="vertical-align:middle" alt="' . t("What's New?"). '" /> ' . t("What's New?"), $domain . '/newfeatures', array('html' => TRUE, 'absolute' => TRUE));
				?> | <?php print t('Member of the '); ?><a href="<?php print $domain ?>">LifeDesk</a> Community</td>
			</tr>
		</tbody>
	</table>
</form>
</div>
<!-- header-region -->

    <div id="wrapper">
    <div id="container">

<!-- header -->
      <div id="header">
      	<?php if ($search_box): ?><div id="site_search"><?php print $search_box ?></div><?php endif; ?>
      	<div id="logo-floater">
      	  <?php
          // Prepare header
          $site_fields = array();
          if ($site_name) {
            $site_fields[] = check_plain($site_name);
          }
          if ($site_slogan) {
            $site_fields[] = check_plain($site_slogan);
          }
          $site_title = implode(' ', $site_fields);
          if ($site_fields) {
            $site_fields[0] = '<span>'. $site_fields[0] .'</span>';
          }
          $site_html = implode(' ', $site_fields);

          if ($logo || $site_title) {
	        $title_color = variable_get('title_color','#333');
            print '<h1><a style="color:' . $title_color . '" href="/" title="'. $site_title .'">';
            if ($logo) {
              print '<img src="'. check_url($logo) .'" alt="'. $site_title .'" id="logo" />';
            }
            print $site_html .'</a></h1>';
          }
          ?>
        </div>
      </div>
<!-- /header -->
	<div id="page-separator-general"></div>
	<div id="content">
	<div id="left-page-content">

	</div>
	<div id="center-page-content">
	<?php if ($messages): print $messages; endif; ?>
	<?php print '<div style="margin:25px;font-size:x-large;">' . variable_get('site_name','LifeDesks') . ' ' . t('is currently offline for routine maintenance.') . '</div>'; ?>
	</div>
	<span class="clear-block"></span>
	</div>
	<span class="clear-block"></span>
  </div>
  </div>
<?php print $closure ?>
</body>
</html>