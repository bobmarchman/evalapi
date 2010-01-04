<?php
// $Id$

/**
 * @file
 * Eval API admin settings form and submit handler.
 */





/**
 * Admin settings form.
 */
function evalapi_admin_settings_form() {
  $form = array();

  // generate a list of content type names to choose from
  $options = array();

  if ($types = node_get_types()) {
    foreach ($types as $type) {
      $options[$type->type] = $type->name;
    }
  }

  $form['evalapi_content_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Enable Content Types'),
    '#description' => t('Choose which content types should be enabled to use the Eval API.'),
    '#options' => $options,
    '#default_value' => variable_get('evalapi_content_types', array()),
  );

  // retrieve list of valid implemented evaluation handlers
  $handlers = _evalapi_process_handlers();
  
  if (module_exists('advanced_help')) {
    $help = theme('advanced_help_topic', 'evalapi', 'evaluation-handlers');
  }
  else {
    $help = '';
  }

  $form['evalapi_default_handler'] = array(
    '#type' => 'select',
    '#title' => t('Default Evaluation Method'),
    '#description' => $help . t('Choose the default method for evaluating a node’s Eval API criteria.'),
    '#options' => $handlers,
    '#default_value' => variable_get('evalapi_default_handler', '_evalapi_default_handler'),
  );

  $form['evalapi_enable_debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Debugging'),
    '#description' => t('When enabled, prints out debugging information when viewing Eval API-enabled nodes.'),
    '#default_value' => variable_get('evalapi_enable_debug', FALSE),
  );

  if (module_exists('admin')) {
    $form['evalapi_use_admin_sidebar'] = array(
      '#type' => 'checkbox',
      '#title' => t('Place Eval API PHP code in node add/edit form sidebar.'),
      '#description' => t('If you have enabled the Admin module, you can optionally choose to place the textarea field for a node’s Eval API PHP code in the sidebar of node add/edit forms. When disabled, the textarea will still appear as a separate fieldset on the form.'),
      '#default_value' => variable_get('evalapi_use_admin_sidebar', FALSE),
    );
  }

  $form['#submit'][] = 'evalapi_admin_settings_form_submit';

  return system_settings_form($form);
}





/**
 * Implementation of hook_submit().
 */
function evalapi_admin_settings_form_submit($form, &$form_state) {
  // clean up the array of content types for easier retrieval
  $types = $form_state['values']['evalapi_content_types'];

  $enabled = array();

  foreach ($types as $type) {
    if ($type) {
      $enabled[] = $type;
    }
  }

  $form_state['values']['evalapi_content_types'] = $enabled;
}