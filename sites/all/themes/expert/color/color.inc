<?php
// $Id$

$info = array(

  // Pre-defined color schemes (base color, link color, top header, bottom header, and text color).
  'schemes' => array(
    '#f1f1ee,#149ACC,#ffffff,#ffffff,#333333' => t('LifeDesk (Default)'),
    '#f1f1ee,#149ACC,#6598cb,#6598cb,#333333' => t('Bluemarine'),
    '#f1f1ee,#b43131,#ffffff,#d35445,#333333' => t('Cinnamon'),
    '#f1f1ee,#399d3e,#95d293,#399d3e,#333333' => t('Forest Green'),
    
  ),

  // Images to copy over.
  'copy' => array(
    'images/toc.png',
  ),

  // CSS files (excluding @import) to rewrite with new color scheme.
  'css' => array(
    'css/page-backgrounds.css',
    'css/html-reset.css',
  ),

  // Coordinates of gradient (x, y, width, height).
  'gradient' => array(40, 0, 500, 90),

  // Color areas to fill (x, y, width, height).
  'fill' => array(
    'base' => array(0, 0, 600, 465),
    'link' => array(59, 122, 12, 20),
    //'top' => array(193, 97, 5, 1),
    //'bottom' => array(65, 405, 80, 30),
    'text' => array(193, 97, 5, 1),
  ),

  // Coordinates of all the theme slices (x, y, width, height)
  // with their filename as used in the stylesheet.
  'slices' => array(
     'images/bg_body.png' => array(1, 1, 1, 1),
     'images/bg_header.png' =>  array(40, 0, 1, 90),
     'images/bg_gradient_left.png' =>  array(0, 0, 39, 1),
     'images/bg_gradient_right.png' =>  array(561, 0, 39, 1),
     'images/bg_horizontal_separator.png' => array(40, 90, 1, 6),
     'images/bg_vertical_separator.png' => array(193, 97, 9, 1),
     'images/toc_hover.png' => array(59, 122, 12, 20),
     'logo.png' => array(40, 0, 268, 80),
     'screenshot.png' => array(40, 0, 400, 240),

  ),

  // Reference color used for blending. Matches the base.png's colors.
  'blend_target' => '#ffffff',

  // Preview files.
  'preview_image' => 'color/preview.png',
  'preview_css' => 'color/preview.css',
  
  // Base file for image generation.
  'base_image' => 'color/base.png',
);
