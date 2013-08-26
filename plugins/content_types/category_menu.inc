<?php

$plugin = array(
  'title' => t('Ding category menu'),
  'description' => t('Generates links to category overview pages.'),
  'single' => TRUE,
  'edit form' => 'ding_content_category_menu_content_type_edit_form',
  'render callback' => 'ding_content_category_menu_content_type_render',
  'category' => t('Ding!'),
  'required context' => array(new ctools_context_optional(t('Selected category'), array('term', 'taxonomy_term')), new ctools_context_optional(t('Library'), 'node')),
  'defaults' => array(
    'type' => 'category',
  ),
);

function ding_content_category_menu_content_type_render($subtype, $conf, $panel_args, $context = NULL) {
  $block = new stdClass();
  $library = NULL;
  $selected_term = NULL;
  $items = array();

  if (isset($context[0]->data)) {
    $selected_term = $context[0]->data;
  }

  if (isset($context[1]->data)) {
    $library = $context[1]->data;
    $uri = entity_uri('node', $library);
    $base = $uri['path'] . '/' . $conf['type'];
  }
  else {
    $base = $conf['type'];
  }

  $node = NULL;
  // Only for node landing page to highlight the category in the list.
  if (arg(0) == 'node' && isset($panel_args[0])) {
    $node = node_load($panel_args[0]);
    if (isset($node->field_category['und'][0]['tid'])) {
      $selected_term->tid = $node->field_category['und'][0]['tid'];
    }
  }

  $options = array();

  $vocabulary = taxonomy_vocabulary_machine_name_load('category');
  $terms = taxonomy_get_tree($vocabulary->vid);

  foreach ($terms as $term) {
    if ($selected_term && $selected_term->tid == $term->tid) {
      $options = array('attributes' => array('class' => array('active')));
    }
    else {
      $options = array();
    }


    if (is_object($node)) {
      // Check if there are nodes of the same type attached to this term.
      $count = db_select('field_data_field_category')
        ->condition('field_category_tid', $term->tid)
        ->condition('bundle', $node->type)
        ->countQuery()
        ->execute()
        ->fetchField();
    }
    else {
      // Else check if there is any content marked with the term. This is
      // for pages that are not of a specific content type.
      $count = db_select('field_data_field_category')
        ->condition('field_category_tid', $term->tid)
        ->countQuery()
        ->execute()
        ->fetchField();
    }

    // Skip terms which have no nodes.
    if ($count == 0) {
      continue;
    }
    // @todo: will anything other than "combined" (base = category) be used?
    $use_conf_base = FALSE;
    if ($base == 'category') {
      $term_alias = drupal_lookup_path('alias', 'taxonomy/term/' . $term->tid);
      if ($term_alias) {
        $items[] = l($term->name, $term_alias, $options);
      }
      else {
        $use_conf_base = TRUE;
      }
    }
    else {
      $use_conf_base = TRUE;
    }

    if ($use_conf_base) {
      $items[] = l($term->name, $base . '/' . drupal_strtolower($term->name), $options);
    }
  }
  // If there are no items to be shown hide the block.
  if (count($items) > 0) {
    $block->content = theme('item_list', array('items' => $items));
  }
  return $block;
}

/**
 * 'Edit form' callback for the content type.
 */
function ding_content_category_menu_content_type_edit_form($form, &$form_state) {
  $conf = $form_state['conf'];

  $form['type'] = array(
    '#type' => 'select',
    '#title' => t('Type of list to link to'),
    '#options' => array(
      'news' => t('News'),
      'events' => t('Events'),
      'category' => t('Combined'),
    ),
    '#default_value' => !empty($conf['type']) ? $conf['type'] : 'news',
    '#required' => TRUE,
  );

  return $form;
}

function ding_content_category_menu_content_type_edit_form_submit(&$form, &$form_state) {
  foreach (array_keys($form_state['plugin']['defaults']) as $key) {
    $form_state['conf'][$key] = $form_state['values'][$key];
  }
}

