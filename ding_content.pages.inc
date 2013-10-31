<?php
/**
 * @file
 * Generic functionality.
 */

/**
 * Fetch relevant content attached to a certain term.
 *
 * This is an AJAX responder.
 *
 * @param string $type
 *   Request type, ajax/nojs.
 * @param int $tid
 *   Term id.
 *
 * @return array
 *   Set of ajax commands.
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
