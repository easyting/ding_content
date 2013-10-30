<?php
/**
 * @file
 * Generic functionality.
 */

function _ding_content_relevant_content($type, $tid) {
  if ($type == 'ajax') {
    $nids = ding_content_get_attached_nodes($tid);
    $markup = ding_content_attached_nodes_markup($nids);

    $commands = array();
    $commands[] = array(
      'command' => 'related_content_insert',
      'data' => $markup,
      'tid' => $tid,
    );

    return array('#type' => 'ajax', '#commands' => $commands);
  }
  else {
    //@todo
    // LATER.
  }
}
