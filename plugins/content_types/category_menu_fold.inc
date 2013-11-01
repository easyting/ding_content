<?php
/**
 * @file
 * Foldable category menu plugin.
 */

$plugin = array(
  'title' => t('Ding category foldable menu'),
  'description' => t('Display category menu with most relevant items.'),
  'single' => TRUE,
  'edit form' => 'ding_content_category_foldable_menu_form',
  'render callback' => 'ding_content_category_foldable_menu_render',
  'category' => t('Ding!'),
  'required context' => array(
    new ctools_context_optional(t('Taxonomy term'), 'taxonomy_term'),
  ),
  'defaults' => array(
    'type' => 'category',
  ),
);

/**
 * Plugin config form.
 */
function ding_content_category_foldable_menu_form($form, $form_state) {
  return $form;
}

/**
 * Plugin renderer.
 */
function ding_content_category_foldable_menu_render($subtype, $conf, $panel_args, $context) {
  $arg = $context[0]->get_original_argument();

  if (!is_object($arg)) {
    $arg = taxonomy_term_load($arg);
  }

  $items = array();
  $defaults = variable_get('category_tree', array());
  usort($defaults, '_ding_content_category_sort');

  if (count($defaults) > 0) {
    $node = NULL;
    if (arg(0) == 'node' && isset($panel_args[0])) {
      $node = node_load($panel_args[0]);
    }

    foreach ($defaults as $entry) {
      $expanded_content = '';

      if ($entry['category_disabled']) {
        continue;
      }

      if (!variable_get('category_empty')) {
        $node_hits = db_select('field_data_field_category')
          ->condition('field_category_tid', $entry['category_tid'], '=')
          ->condition('entity_type', 'node')
          ->countQuery()
          ->execute()
          ->fetchField();

        if ($node_hits == 0) {
          continue;
        }
      }

      $options = array('attributes' => array('class' => array('relevant-nodes', 'relevant-nodes-' . $entry['category_tid'])));

      // It's either node page, where node category matches the term,
      // or taxonomy page, where term arg matches the term.
      $active = (is_object($node) && $node->field_category[LANGUAGE_NONE][0]['tid'] == $entry['category_tid']) || (is_object($arg) && $arg->tid == $entry['category_tid']);
      if ($active) {
        $options['attributes']['class'][] = 'active';
      }

      if ($entry['category_expanded']) {
        $options['attributes']['class'][] = 'relevant-nodes-expanded';
        $nids = ding_content_get_attached_nodes($entry['category_tid']);
        $expanded_content = ding_content_attached_nodes_markup($nids);
      }

      $link = l($entry['category_name'], 'ding_content/relevant/nojs/' . $entry['category_tid'], $options);
      $items[] = $link . $expanded_content;
    }
  }

  drupal_add_js(drupal_get_path('module', 'ding_content') . '/js/ding_content.related.js', 'file');
  drupal_add_css('.pane-category-menu-fold .item-list > ul > li > a.relevant-node-loading {
    background-image: url(/' . drupal_get_path('module', 'ding_content') . '/img/loading.gif) !important;
    background-position: 98% 50%;
    background-repeat: no-repeat;
  }', 'inline');
  $plugin = new stdClass();
  $plugin->content = theme('item_list', array('items' => $items));

  return $plugin;
}
