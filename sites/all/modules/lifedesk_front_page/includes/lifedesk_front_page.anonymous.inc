<?php

/**
 * Include file for authenticated users homepage
 */
function _lifedesk_front_page_anonymous() {
  global $lifedesk_front_page;
  drupal_add_css($lifedesk_front_page .'css/lifedesk_front_page.css');
  drupal_add_js($lifedesk_front_page .'js/lifedesk_front_page.js');

  $output .= '<div id="lifedesk_front_page_wrapper">';
  
  $output .= '<div id="lifedesk_front_page_classification">';
  $output .= '<ul class="lifedesk_front_page_links classification_tree"><li class="classification_toggle classification_show"><a href="#" onclick="javascript:return false;">' . t('Show classification') . '</a></li><li class="classification_toggle  classification_hidden"><a href="#" onclick="javascript:return false;">' . t('Hide classification') . '</a></li></ul>';

  $output .= '<div id="lifedesk_front_page_tree_wrapper">';
  $output .= '<div id="lifedesk_front_page_tree">';

  $output .= '<div id="classification_tree_div" class="classification_tree_div">';
  $output .= '<ul class="lifedesk_front_page_links_expand"><li class="expand"><a href="#" onclick="javascript:return false">&nbsp;</a></li><li class="contract"><a href="#" onclick="javascript:return false">&nbsp;</a></li></ul>';
  $block = module_invoke('classification', 'block', 'view', 0);
  $output .= $block['content'];
  $output .= '</div>';
  
  $output .= '</div>';
  $output .= '</div>';

  $output .= '</div>';

  $output .= '<div id="lifedesk_front_page_welcome">';
  $anon = variable_get('lifedesk_front_page_welcome_anonymous',array('body' => '', 'format' => FILTER_FORMAT_DEFAULT));
  $output .= $anon['body'];
  $output .= '</div>';

  $output .= '<div id="lifedesk_front_page_panels">';
  $output .= '<ul class="lifedesk_front_page_panel_ul">';

  if(variable_get('lifedesk_front_page_image_anon', 1) == 1) {
    $output .= '<li class="lifedesk_front_page_panel_li">';
    $output .= '<h3 class="h">' . t('Latest Image') . '</h3>';
    $block = module_invoke('image', 'block', 'view', 0);

    $output .= (!$block['content'] || strpos($block['content'],'img') < 1) ? '<img src="' . $lifedesk_front_page . 'images/defaultimage.jpg" class="temporary-image" alt="Coming Soon!" title="Coming Soon!">' : $block['content'];

    $output .= '</li>';
  }

  if(variable_get('lifedesk_front_page_online_anon', 1) == 1) {
    $output .= '<li class="lifedesk_front_page_panel_li">';
    $output .= '<h3 class="h">' . t('Who\'s Online') . '</h3>';
    $block = module_invoke('members','block','view', 0);
    $output .= $block['content'];
    $output .= '</li>';
  }

  if(variable_get('lifedesk_front_page_poll_anon', 1) == 1) {
    $block = module_invoke('poll','block','view',0);
    if($block['content']) {
      $output .= '<li class="lifedesk_front_page_panel_li">';
      $output .= '<h3 class="h">' . t('Polls') . '</h3>';
      $output .= $block['content'];
      $output .= '</li>';
    }
  }

  $output .= '</ul>';
  $output .= '<div style="clear:both"></div>';
  $output .= '</div>';

  //Recent biblio block
  if(variable_get('lifedesk_front_page_biblio_anon', 1) == 1) {
    $output .= '<div id="lifedesk_front_page_biblio">';
    $output .= '<h3 class="h">' . t('Recent bibliographic items') . '</h3>';
    $block = module_invoke('biblio', 'block', 'view', 0);
    if($block) {
      $output .= $block['content'];
    }
    else {
      $output .= '<ul><li>' . t('Recently submitted bibliographic items will appear here.') . '</li></ul>';
    }
      $output .= '</div>';
  }
  
  //end wrapper
  $output .= '</div>';
    
  print $output;
}
?>