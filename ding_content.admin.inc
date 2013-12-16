<?php

/**
 * @file
 * Admin page callbacks for the ding_content module.
 */

/**
 * AJAX Callback function to return a list of media files
 *
 * More or less a copy of the media_browser_list callback.
 * @see media_browser_list()
 */
function ding_content_media_browser_list() {
  module_load_include('inc', 'media', 'includes/media.browser');
  global $user;
  $params = drupal_get_query_parameters();
  // How do we validate these?  I don't know.
  // I think PDO should protect them, but I'm not 100% certain.
  array_walk_recursive($params, '_media_recursive_check_plain');

  $types = isset($params['types']) ? $params['types'] : NULL;
  $url_include_patterns = isset($params['url_include_patterns']) ? $params['url_include_patterns'] : NULL;
  $url_exclude_patterns = isset($params['url_exclude_patterns']) ? $params['url_exclude_patterns'] : NULL;

  $start = isset($params['start']) ? $params['start'] : 0;
  $limit = isset($params['limit']) ? $params['limit'] : media_variable_get('browser_pager_limit');

  $query = db_select('file_managed', 'f');
  $query->fields('f', array('fid'));
  $query->range($start, $limit);
  $query->orderBy('f.fid', 'DESC');

  // Add conditions based on file type *or* allowed extensions.
  $condition = $query;
  if (!empty($types) && !empty($params['file_extensions'])) {
    $condition = db_or();
  }
  if (!empty($types)) {
    $condition->condition('f.type', $types, 'IN');
  }
  if (!empty($params['file_extensions'])) {
    $extensions = array_filter(explode(' ', $params['file_extensions']));
    foreach ($extensions as $extension) {
      $condition->condition('f.uri', '%' . db_like('.' . $extension), 'LIKE');
    }
  }
  if ($condition instanceof DatabaseCondition) {
    $query->condition($condition);
  }

  if ($url_include_patterns) {
    $query->condition('f.uri', '%' . db_like($v) . '%', 'LIKE');
    // Insert stream related restrictions here.
  }
  if ($url_exclude_patterns) {
    $query->condition('f.uri', '%' . db_like($v) . '%', 'NOT LIKE');
  }

  // @todo Implement granular editorial access: http://drupal.org/node/696970.
  //   In the meantime, protect information about private files from being
  //   discovered by unprivileged users. See also media_view_page().
  if (!user_access('administer media')) {
    $query->condition('f.uri', db_like('private://') . '%', 'NOT LIKE');
  }

  $query->condition('f.status', FILE_STATUS_PERMANENT);
  $query->condition('f.uid', $user->uid);

  foreach (array_keys(media_get_hidden_stream_wrappers()) as $name) {
    $query->condition('f.uri', db_like($name . '://') . '%', 'NOT LIKE');
  }

  $fids = $query->execute()->fetchCol();
  dd('test');
  dd($fids);
  $files = file_load_multiple($fids);
  foreach ($files as $file) {
    media_browser_build_media_item($file);
  }

  drupal_json_output(array('media' => array_values($files)));
  exit();
}

/**
 * Category foldable menu settings form.
 */
function ding_content_foldable_menu_admin($form, $form_state) {
  $voc = taxonomy_vocabulary_machine_name_load('category');

  if (is_object($voc)) {
    $tree = taxonomy_get_tree($voc->vid);
    $range = range(1, 10);

    $form['category_hits'] = array(
      '#type' => 'select',
      '#title' => t('Number of nodes shows in expanded area'),
      '#options' => array_combine($range, $range),
      '#default_value' => variable_get('category_hits', 4),
    );

    $form['category_empty'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show empty categories'),
      '#default_value' => variable_get('category_empty', FALSE),
      '#description' => t('Show also the categories with no nodes attached to it.'),
    );

    $form['category_tree'] = array(
      '#tree' => TRUE,
      '#theme' => 'ding_content_category_table',
    );

    $defaults = variable_get('category_tree', array());
    $c = count($tree);
    for ($i = 0; $i < $c; $i++) {
      $form['category_tree'][$i]['category_name'] = array(
        '#type' => 'textfield',
        '#value' => $tree[$i]->name,
        '#disabled' => TRUE,
      );

      $form['category_tree'][$i]['category_expanded'] = array(
        '#type' => 'checkbox',
        '#default_value' => isset($defaults[$i]['category_expanded']) ? $defaults[$i]['category_expanded'] : FALSE,
      );

      $form['category_tree'][$i]['category_disabled'] = array(
        '#type' => 'checkbox',
        '#default_value' => isset($defaults[$i]['category_disabled']) ? $defaults[$i]['category_disabled'] : FALSE,
      );

      $form['category_tree'][$i]['category_weight'] = array(
        '#type' => 'weight',
        '#delta' => 10,
        '#default_value' => isset($defaults[$i]['category_weight']) ? $defaults[$i]['category_weight'] : 0,
        '#attributes' => array(
          'class' => array('category-weight'),
        )
      );

      $form['category_tree'][$i]['category_tid'] = array(
        '#type' => 'hidden',
        '#value' => $tree[$i]->tid,
      );
    }
  }
  else {
    drupal_set_message(t('Seems there is no vocabulary with machine name `category`.'), 'error');
  }

  return system_settings_form($form);
}

/**
 * Custom renderer for categroy menu admin form.
 *
 * Create a drag'n'drop weighted table.
 *
 * @see ding_content_foldable_menu_admin()
 */
function theme_ding_content_category_table($vars) {
  $element = $vars['element'];

  $rows = array();

  $ch = array();
  foreach (element_children($element) as $key) {
    $ch[] = $element[$key];
  }

  usort($ch, '_ding_content_category_sort');

  foreach ($ch as $ele) {
    $rows[] = array(
      'data' => array(
        array('data' => render($ele['category_name'])),
        array('data' => render($ele['category_expanded'])),
        array('data' => render($ele['category_weight'])),
        array('data' => render($ele['category_disabled'])),
      ),
      'class' => array('draggable'),
    );
  }

  $header = array(
    t('Category'),
    t('Expanded'),
    t('Weight'),
    t('Hidden'),
  );

  drupal_add_tabledrag('ding-content-category-foldable', 'order', 'sibling', 'category-weight');

  return theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'ding-content-category-foldable')));
}
