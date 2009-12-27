<?php
// $Id$

/**
 * @file
 * Eval API example hook implementations.
 */





/**
 * Implementation of hook_evalapi().
 *
 * @param $node
 *   A node object.
 * @return
 *   An associative array of evaluation criteria where the array keys are an
 *   arbitrary unique string identifier for a given condition and the array
 *   values are booleans.
 */
function hook_evalapi(&$node) {
  $eval_criteria = array(
    'foo' => TRUE,
    'bar' => FALSE,
  );

  return $eval_criteria;
}





/**
 * Implementation of hook_CONTENT_TYPE_NAME_evalapi().
 *
 * @param $node
 *   A node object.
 * @return
 *   An associative array of evaluation criteria where the array keys are an
 *   arbitrary unique string identifier for a given condition and the array
 *   values are booleans.
 */
function hook_CONTENT_TYPE_NAME_evalapi(&$node) {
  $eval_criteria = array(
    'foo' => TRUE,
    'bar' => FALSE,
  );

  return $eval_criteria;
}





/**
 * Implementation of hook_evalapi_alter().
 *
 * @param $eval_criteria
 *   An associative array of evaluation criteria where the array keys are an
 *   arbitrary unique string identifier for a given condition and the array
 *   values are booleans.
 * @param $node
 *   A node object.
 */
function hook_evalapi_alter(&$eval_criteria, &$node) {
  if ($eval_criteria['foo'] === TRUE) {
    $eval_criteria['foo'] = FALSE;
  }
}





/**
 * Implementation of hook_CONTENT_TYPE_NAME_evalapi_alter().
 *
 * @param $eval_criteria
 *   An associative array of evaluation criteria where the array keys are an
 *   arbitrary unique string identifier for a given condition and the array
 *   values are booleans.
 * @param $node
 *   A node object.
 */
function hook_CONTENT_TYPE_NAME_evalapi_alter(&$eval_criteria, &$node) {
  if ($eval_criteria['bar'] === FALSE) {
    $eval_criteria['bar'] = TRUE;
  }
}





/**
 * Implementation of hook_evalapi_handler().
 *
 * @return
 *   An associative array with one key that is the implementing module’s name
 *   whose value is also an associative array with the following keys:
 *     "name": The human-readable name for this evaluation handler.
 *     "callback": The name of a callback function that does the evaluation.
 */
function hook_evalapi_handler() {
  return array(
    'foo' => array(
      'name' => t('Foo'),
      'callback' => '_evalapi_example_evalapi_handler_callback',
    ),
  );
}





/**
 * Implementation of hook_evalapi_handler_alter().
 *
 * @param
 *   An associative array of valid Eval API evaluation handler implementations.
 */
function hook_evalapi_handler_alter(&$handlers) {
  if (isset($handlers['foo'])) {
    $handlers['foo']['callback'] = 'mymodule_evalapi_handler_callback';
  }
}





/**
 * Example implementation of an Eval API handler callback.
 *
 * @param $eval_criteria
 *   The evaluation criteria array for the node being operated on.
 * @return
 *   Boolean
 * @see _evalapi_default_handler()
 */
function _evalapi_example_evalapi_handler_callback(&$eval_criteria) {
  if (empty($eval_criteria)) {
    return TRUE;
  }

  $all_rules  = array();

  foreach ($eval_criteria as $rules) {
    $all_rules = array_merge($all_rules, $rules);
  }

  if (count($all_rules) == array_sum($all_rules)) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}