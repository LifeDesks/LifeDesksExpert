<?php
// $Id: color.inc,v 1.4 2007/12/14 17:00:14 goba Exp $

$info = array(

  // Pre-defined color schemes.
  'schemes' => array(
    '#149ACC,#149ACC,#ffffff,#ffffff,#333333' => t('LifeDesk (Default)'),
    '#149ACC,#149ACC,#6598cb,#6598cb,#333333' => t('Bluemarine'),
    '#149ACC,#b43131,#ffffff,#d35445,#333333' => t('Cinnamon'),
    '#149ACC,#399d3e,#95d293,#399d3e,#333333' => t('Forest Green'),
    
  ),

  // Images to copy over.
  'copy' => array(
    'images/toc.png',
  ),

  // CSS files (excluding @import) to rewrite with new color scheme.
  'css' => array(
    'style.css',
  ),

  // Coordinates of gradient (x, y, width, height).
  'gradient' => array(0, 0, 600, 90),

  // Color areas to fill (x, y, width, height).
  'fill' => array(
    'base' => array(0, 0, 600, 465),
    'link' => array(65, 405, 80, 30),
  ),

  // Coordinates of all the theme slices (x, y, width, height)
  // with their filename as used in the stylesheet.
  'slices' => array(
     'images/bg_header.png'       =>  array(40, 0, 1, 90),
     'images/toc_hover.png' => array(175,410,12,20),
     'logo.png'        => array(40, 0, 268, 80),
     'screenshot.png'                       => array(40, 0, 400, 240),

  ),

  // Reference color used for blending. Matches the base.png's colors.
  'blend_target' => '#ffffff',

  // Preview files.
  'preview_image' => 'color/preview.png',
  'preview_css' => 'color/preview.css',

  // Base file for image generation.
  'base_image' => 'color/base.png',
);
