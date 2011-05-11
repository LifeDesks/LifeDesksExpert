$Id: README.txt,v 1.1.4.11 2008/07/19 13:13:08 swentel Exp $

What?
The PicLens module for Drupal makes it easy for you to provide your visitors 
with an immersive slideshow experience for rich media on your website. It supports
the media RSS feeds which is needed by the 3D plugins. The PicLens Lite is also supported
enabling a really slick slideshow on your own website.

See the PicLens Lite in action on http://realize.be/image-galleries/drupalcon-boston-2008

For more information about PicLens and browser 3D plugins, go to http://www.piclens.com/

------------------------------------------------------------------------------------------

Supported Modules
The image gallery and taxonomy + image are supported out of the box.
Go to http://example.com/admin/settings/piclens for settings.

------------------------------------------------------------------------------------------

API
Several functions are available to create your own mediaRSS feed
or add the link and/or javascript needed for PicLens (Lite).
You can implement hook_mediarss_items() in your own custom module
to pass along media items. The link to the mediarss feed should then look as this:

http://example.com/piclens/example_module/args

- piclens: menu callback in the piclens module. This function will then call module_invoke
to get the items from hook_mediarss_items()
- yourmodule: name of your module
- args: optional arguments needed to pass to your hook

/**
 * Implementation of hook_mediarss_items.
 */
function example_module_mediarss_items($args = array()) {
  $items = '';
  // get all my items
  $my_items = db_query("SELECT * FROM {my_table}");
  while ($my_item = db_fetch_object($my_items)) {
    $item = array(
      'title' => $my_item->title,
      'link' => $my_item->link,
      'thumbnail' => file_create_url($my_item->thumbnail),
      'content' => file_create_url($my_item->content),
    );
    $items .= mediarssapi_format_item($item);
  }
  return $items;
}

------------------------------------------------------------------------------------------

Available functions in piclens.module

* mediarssapi_feed_url($url, $title);
  This function adds the rss feed with the $url to your own menu callback providing the feed
  into the head of your document. $title is optional.

* mediarssapi_format_item($item);
  This function formats one item (image, video) into a valid xml structure for the piclens feed.
  $item is an array with following properties:
  - title : title image or video
  - link : direct link to content
  - thumbnail : thumbnail url image or video
  - content : content url of image or video  

* mediarssapi_rss($items);
  This function outputs the feed. $items is a string formatted xml list of media items.

* mediarssapi_piclens_lite_javascript()
  This function adds the javascript for the PicLens Lite into the head of your document.

* mediarssapi_piclens_lite_link($image_or_text, $drupal_set_message)
  This function returns a link to start the PicLens Lite slideshow.
  $image_or_text, use something different as the standard text and piclens arrow
  If $drupal_set_message = TRUE, the link will be inserted as a drupal message, otherwhise
  the link is simply returned. Inside this function theme('piclens_lite_html_link'); is called
  to return the html. You can override the theming with phptemplate_piclens_lite_html_link() off course. 

------------------------------------------------------------------------------------------

Bug reports, feature requests etc
http://drupal.org/project/piclens
