<?php
// $Id: page.tpl.php,v 1.18 2008/01/24 09:42:53 goba Exp $
?><!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="<?php print $language->language ?>" lang="<?php print $language->language ?>" dir="<?php print $language->dir ?>">
  <head>
    <title><?php print $head_title ?></title>
    <?php print $head ?>
    <?php print $styles ?>
    <?php print $scripts ?>
    <!--[if lt IE 7]>
      <?php print phptemplate_get_ie_styles(); ?>
    <![endif]-->
  </head>
<body id="home-page"<?php print eol_body_class(); ?>>

  <div id="wrapper">
    <div id="container">

<!-- header -->
      <div id="header">
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
            print '<h1><a style="color:' . $title_color . '" href="'. check_url($front_page) .'" title="'. $site_title .'">';
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
         <?php print $left ?>
        </div>
        <div id="center-page-content">
	        <div id="breadcrumb">
	           <?php if ($breadcrumb): print $breadcrumb; endif; ?>
	           <?php if ($title): print '<h2 id="title" style="margin-top:4px">'. $title .'</h2>'; endif; ?>
          </div>
          <span class="clear-block clear"></span>
        	<?php if ($messages): print $messages; endif; ?>
               <?php if ($tabs): print '<div id="tabs-wrapper">'; endif; ?>
               <?php if ($tabs): print '<ul class="tabs primary">'. $tabs .'</ul></div>'; endif; ?>
               <?php if ($tabs2): print '<ul class="tabs secondary">'. $tabs2 .'</ul>'; endif; ?>
        	<?php print $content ?>
        </div>
        <span class="clear-block"></span>
      </div>
      <span class="clear-block"></span>
      <?php if ($footer): ?>
        <div id="footer">
          <?php print $footer; ?>
        </div>
      <?php endif; ?>
    </div>
  </div>
<?php print $closure ?>
</body>
</html>