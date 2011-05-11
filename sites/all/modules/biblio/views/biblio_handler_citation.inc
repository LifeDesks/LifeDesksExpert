<?php
// $Id: biblio_handler_citation.inc,v 1.1.2.8 2010/05/12 01:31:36 rjerome Exp $
class biblio_handler_citation extends views_handler_field {

  function init(&$view, $options) {
    parent::init($view, $options);
    $this->additional_fields['vid'] = array('table' => 'biblio', 'field' => 'vid');
  }

  function query() {
    $this->add_additional_fields();
    $this->field_alias = $this->aliases['vid'];
    // add the biblio_type field as tid
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['style_name'] = array('default' => biblio_get_style());
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    module_load_include('inc','biblio','biblio.admin');
    $form['style_name'] = array(
      '#type' => 'radios',
      '#title' => t('Style'),
      '#default_value' => $this->options['style_name'],
      '#options' => _biblio_admin_get_styles(),
      '#description' => t('Define the layout of author lists.')
    );
  }

  function render($values) {
    $node = node_load($values->nid);
    $base = variable_get('biblio_base', 'biblio');
    $inline = false;
    
    if ($node->type == 'biblio') {
      return theme('biblio_entry', $node, $base, $this->options['style_name'], $inline);
    }
  }

}