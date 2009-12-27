<?php
// $Id$

/**
 * @file
 *   Core Eval API functions.
 */





/**
 * Construct and return a node’s evaluation criteria array.
 *
 * This function serves as a dispatcher for the following hooks:
 *   – hook_evalapi()
 *   – hook_CONTENT_TYPE_NAME_evalapi()
 *   – hook_evalapi_alter()
 *   – hook_CONTENT_TYPE_NAME_evalapi_alter()
 *
 * The evaluation criteria array is constructed in the following manner:
 *   – If it exists, retrieve and store the return value of this node’s
 *     evaluation code.
 *   – Invoke hook_CONTENT_TYPE_NAME_evalapi() and store its return
 *     value.
 *   – Invoke hook_evalapi() and store its return value.
 *   – Merge the values of steps 1-3 into one array.
 *   – Invoke hook_CONTENT_TYPE_NAME_evalapi_alter(), passing in the
 *     array from step 4.
 *   – Invoke hook_evalapi_alter(), passing in the array from step 4.
 *   – Return the fully constructed array.
 *
 * This method allows other modules better flexibility for adding to, deleting
 * from, and modifying a node’s evaluation criteria. We use specificity in
 * allowing criteria to be set on a per-node basis first, then a per-content
 * type basis, and, finally, on a global basis. By calling drupal_alter() on our
 * two hooks, we allow modules to easily modify the now fully constructed
 * evaluation criteria array.
 *
 * The Eval API provides two methods of retrieving/evaluating a node’s
 * evaluation criteria array:
 *   – By explicitly calling _evalapi_evaluate() and passing the node
 *       object as the only parameter.
 *   – A provided Drupal action: evalapi_evaluate_action().
 *
 * NOTE: The implementation of the action is simply a call to
 *       _evalapi_evaluate(), but we explicitly set the second
 *       parameter, $return, to FALSE, which tells the function NOT to return
 *       the evaluation criteria array, but to evaluate it instead and store
 *       the results in the current user’s session.
 *
 * @param $node
 *   A node object.
 * @return
 *   An associative array of evaluation criteria where the array keys are an
 *   arbitrary unique string identifier for a given condition and the array 
 *   values are booleans.
 * @see _evalapi_evaluate()
 * @see _evalapi_get_node_eval_code()
 * @see drupal_alter()
 * @see hook_evalapi()
 * @see hook_CONTENT_TYPE_NAME_evalapi()
 * @see hook_evalapi_alter()
 * @see hook_CONTENT_TYPE_NAME_evalapi_alter()
 */
function _evalapi(&$node) {
  $eval_criteria = array();

  // retrieve this specific node’s evaluation code
  $node_eval_code = _evalapi_get_node_eval_code($node);

  if (!empty($node_eval_code) && user_access('use php for evaluation criteria')) {
    // can’t use drupal_eval() here because it is for printing PHP output and we
    // need to actually assign the returned array to a variable, and we can’t
    // call eval() just straight up because the returned array comes from user
    // input. The solution used below comes from CCK’s content_allowed_values()
    // which itself comes from Views’ handling of ‘views arguments PHP code’.
    $per_node_eval = array();
    
    ob_start();
      $per_node_eval = eval($node_eval_code);
    ob_end_clean();
    
    if (!empty($per_node_eval) && is_array($per_node_eval)) {
      $eval_criteria['node'] = $per_node_eval;
    }
  }
    
  $per_node_type_criteria = array();
  $per_node_criteria = array();
  $modules = array();
  
  // call any modules that implement hook_CONTENT_TYPE_NAME_evalapi()
  foreach (module_implements($node->type .'_evalapi') as $module) {
    $module_criteria = call_user_func($module .'_'. $node->type .'_evalapi', $node);
    
    if (isset($module_criteria) && is_array($module_criteria)) {  
      foreach (array_keys($module_criteria) as $rule) {
        $modules[$module][$rule] = $module_criteria[$rule];
      }
      
      $per_node_type_criteria = array_merge($per_node_type_criteria, $modules);
    }
  }
  
  // call any modules that implement hook_evalapi()
  foreach (module_implements('evalapi') as $module) {
    $module_criteria = call_user_func($module .'_evalapi', $node);
    
    if (isset($module_criteria) && is_array($module_criteria)) {
      foreach (array_keys($module_criteria) as $rule) {
        $modules[$module][$rule] = $module_criteria[$rule];
      }

      $per_node_criteria = array_merge($per_node_criteria, $modules);
    }
  }

  // merge all collected evaluation criteria together into one master array
  $eval_criteria = array_merge($eval_criteria, $per_node_type_criteria, $per_node_criteria);

  // give modules one last chance to alter the evaluation criteria array:
  // invoke hook_CONTENT_TYPE_NAME_evalapi_alter()
  drupal_alter($node->type .'_evalapi', $eval_criteria, $node);

  // invoke hook_evalapi_alter()
  drupal_alter('evalapi', $eval_criteria, $node);


  return $eval_criteria;
}










/**
 * Retrieve/Evaluate a node’s evaluation criteria array.
 *
 * @param $node
 *   A node object.
 * @param (bool) $return
 *   - If set to TRUE, retrieve the evaluation criteria array and return it.
 *   - If set to FALSE, do not return the evaluation criteria array, but instead
 *     evaluate it and store the results in the current user’s session.
 * @return
 *   - If $return is TRUE, this node’s evaluation criteria array.
 *   - If $return is FALSE, return nothing.
 * @see _evalapi()
 * @see _evalapi_get_node_eval_handler()
 * @see _evalapi_get_eval_value()
 * @see hook_evalapi()
 * @see hook_CONTENT_TYPE_NAME_evalapi()
 * @see hook_evalapi_alter()
 * @see hook_CONTENT_TYPE_NAME_evalapi_alter()
 * @see hook_evalapi_handler() 
 * @see hook_evalapi_handler_alter() 
 */
function _evalapi_evaluate(&$node, $return = TRUE) {
  if (_evalapi_is_enabled_content_type($node->type)) {
    // call dispatcher function to invoke evalapi hooks
    $eval_criteria = _evalapi($node);

    // if DEBUG mode is enabled, print out the contents of the evaluation criteria array
    $debug_enabled = variable_get('evalapi_enable_debug', FALSE);
    if ($debug_enabled) {
      $message  = '<p><strong>'. t('***** [Eval API: DEBUGGING enabled] *****') .'</strong></p>';
      $message .= '<p>'. t('Evaluation Criteria for Node [nid:!nid]', array('!nid' => $node->nid)) .'</p>';
  
      // check for devel module so we can use krumo goodness, if available:
      if (module_exists('devel') && user_access('access devel information')) {
        dpm($eval_criteria, $message);
      }
      else {
        $message .= '<pre>'. print_r($eval_criteria, TRUE) .'</pre>';
        drupal_set_message($message);
      }
    }

    if ($return) {
      return $eval_criteria;
    }
    else {
      // determine this node’s evaluation value
      
      // retrieve the evaluation handler specified for this node
      $eval_handler = _evalapi_get_node_eval_handler($node);
      
      $eval_value = _evalapi_get_eval_value($eval_criteria, $eval_handler);

      // create a unique session array key for this specific node
      $key = 'evalapi_eval_value_'. $node->nid;

      // store the results of the evaluation criteria in the current user’s session
      $_SESSION[$key] = $eval_value;
      
      if ($debug_enabled) {
        if ($eval_value) {
          $value = 'TRUE';
        }
        else {
          $value = 'FALSE';
        }
        
        $message  = '<p>'. t('Evaluation of this node’s criteria returns: !eval_value', array('!eval_value' => $value)) .'</p>';
        $message .= '<p>'. t('Eval API handler callback for this node: !callback', array('!callback' => $eval_handler)) .'</p>';
        drupal_set_message($message);
      }
    }
  }
}





/**
 * Calculate and return the boolean result of a given evaluation criteria array.
 *
 * @param $eval_criteria
 *   An associative array of with boolean values as keys.
 * @param $callback
 *   Optional name of a callback function that calculates the evaluation 
 *   criteria array. Arguments for the callback function can be passed in as 
 *   extra parameters to this function.
 * @param $args
 *   Any additional arguments to pass to the callback function, if one is
 *   set.
 * @return
 *   - TRUE if the number of array records equals the sum of all the array values.
 *   - FALSE otherwise.
 * @see _evalapi_evaluate()
 */
function _evalapi_get_eval_value(&$eval_criteria, $callback = NULL) {
  $eval_value = NULL;

  if (empty($callback)) {
    $callback = variable_get('evalapi_default_handler', '_evalapi_default_handler');
  } 

  $args   = array();
  $args[] = $eval_criteria;
  
  $func_args = func_get_args();        
  unset($func_args[0], $func_args[1]);
  
  $args = array_merge($args, $func_args);

  $eval_value = call_user_func_array($callback, $args);
  
  return $eval_value;
}





/**
 * Default Eval API handler callback.
 *
 * Calculation is performed by counting the number of records in the
 * array and then getting the sum of all boolean values, effectively AND’ing 
 * them together. If the array is empty, we assume there are no rules to check 
 * and thusly return TRUE.
 * 
 * @param $eval_criteria
 *   A node’s evaluation criteria array.
 * @return
 *   - TRUE if the number of array records equals the sum of all the array values.
 *   - FALSE otherwise.
 * @see evalapi_evalapi_handler()
 * @see hook_evalapi_handler()
 * @see hook_evalapi_handler_alter() 
 */
function _evalapi_default_handler(&$eval_criteria) {
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






/**
 * Retrieve Eval API handlers from modules implementing hook_evalapi_handler().
 *
 * This function gathers all handler information from modules and then checks
 * to ensure that each defined callback actually exists as a function. If
 * the handler’s defined callback is not a function, then it is removed from
 * the array of handlers.
 *
 * @return
 *   An array of valid Eval API handlers.
 * @see evalapi_evalapi_handler()
 * @see hook_evalapi_handler()
 * @see hook_evalapi_handler_alter()
 */
function _evalapi_get_handlers() {
  $handlers = array();

  // call any modules that implement hook_evalapi_handler().
  $handlers = module_invoke_all('evalapi_handler');
  
  foreach ($handlers as $handler => $value) {
    if (!isset($value['callback']) || !function_exists($value['callback'])) {
      unset($handlers[$handler]);
    }
  }
  
  // allow modules to alter Eval API handlers before returning them
  drupal_alter('evalapi_handler', $handlers);
  
  return $handlers;
}





/**
 * Format an array of Eval API handlers for use in FAPI select lists.
 *
 * @return
 *   A formatted array of implemented Eval API handlers.
 * @see _evalapi_get_handlers()
 * @see hook_evalapi_handler()
 * @see hook_evalapi_handler_alter() 
 */
function _evalapi_process_handlers() {
  $handlers = _evalapi_get_handlers();
  
  $options = array();
  
  foreach ($handlers as $handler) {
    $options[$handler['callback']] = $handler['name'];
  }
  
  $options = array_reverse($options, TRUE);
    
  return $options;
}





/**
 * Retrieve a node’s evaluation code.
 *
 * @param $node
 *   A node object.
 * @return
 *  A string set to the node’s evaluation code, or NULL if none set.
 * @see _evalapi()
 */
function _evalapi_get_node_eval_code(&$node) {
  $code = NULL;
  
  if (!empty($node->nid)) {
    $code = db_result(db_query(('SELECT e.code FROM {evalapi} e WHERE e.nid = %d'), $node->nid));
    
    if (empty($code)) {
      $code = variable_get('evalapi_code_'. $node->type, NULL);    
    }
  }
  else {
    $code = variable_get('evalapi_code_'. $node->type, NULL);
  }
  
  return $code;
}





/**
 * Retrieve a node’s evaluation handler, if set.
 *
 * If the specified node does not have a handler set, it will default
 * to the node type’s default handler. If the node’s type does not have
 * a handler set, it will default to the global default handler.
 *
 * @param $node
 *   A node object.
 * @return
 *   The key for this node’s specified evaluation handler.
 * @see evalapi_evalapi_handler()
 * @see hook_evalapi_handler()
 * @see hook_evalapi_handler_alter() 
 */
function _evalapi_get_node_eval_handler(&$node) {
  $default_handler = variable_get('evalapi_default_handler', '_evalapi_default_handler');
  
  if (!empty($node->nid)) {
    $handler = db_result(db_query(('SELECT e.handler FROM {evalapi} e WHERE e.nid = %d'), $node->nid));
    
    if (empty($handler)) {
      $handler = variable_get('evalapi_handler_'. $node->type, $default_handler);    
    }
  }
  else {
    $handler = variable_get('evalapi_handler_'. $node->type, $default_handler);
  }
  
  if (empty($handler)) {
    $handler = $default_handler;    
  }  
  
  return $handler;
}





/**
 * Submit handler for node add/edit forms.
 *
 * Processes and stores Eval API evaluation criteria overrides for a node.
 * @see evalapi_form_alter()
 */
function _evalapi_process_node_overrides($form, &$form_state) {
  dpm(&$form_state);
}





/**
 * Retrieve Eval API evaluation criteria overrides for a node.
 *
 * @param
 *   The nid of the node to retrieve overrides for.
 * @return
 *   An associative array of overrides.
 * @see _evalapi_process_node_overrides()
 * @see evalapi_form_alter()
 */
function _evalapi_get_node_overrides($nid = 0) {  
  if (empty($nid)) {
    return array();
  }

  $overrides = array();

  $result = db_result(db_query('SELECT e.overrides FROM {evalapi} e WHERE e.nid = %d', (int) $nid));
  
  if (!empty($result)) {
    $overrides = unserialize($result);
  }

  return $overrides;
}





/**
 * Determine if a given content type is enabled to utilize evaluation checking.
 *
 * @param $type
 *   The machine-name of the content type to check.
 * @return
 *   - TRUE if a content type is enabled for evaluation checking.
 *   - FALSE otherwise.
 */
function _evalapi_is_enabled_content_type($type = '') {
  if (empty($type)) {
    return FALSE;
  }
  
  $enabled_types = variable_get('evalapi_content_types', array());

  if (in_array($type, $enabled_types)) {
    return TRUE;
  }

  return FALSE;
}





/**
 * Return a hash. 
 *
 * Implemented the way Drupal hashes user passwords, but hashes the user’s name
 * instead of their password.
 *
 * @return
 *   An md5() hash.
 * @see user_pass_rehash()
 */
function _evalapi_make_hash() {
  global $user;
  
  if ($user->uid == 0) {
    $anonymous = variable_get('anonymous', t('Anonymous'));
    return md5(time() . $anonymous);
  }
  
  return md5(time() . $user->name . $user->login);
}





/**
 * Move a module’s position in the evaluation criteria array to be BEFORE
 * another module’s entry.
 *
 * @param $source
 *   The source array the module is an element of.
 * @param $module
 *   The machine-name of the module to move.
 * @param $target
 *   The machine-name of the module that $module will be placed before.
 * @return
 *   The modified source array.
 * @see _evalapi_move_module_after()
 * @see _evalapi_insert_before()
 * @see _evalapi_insert_after()
 * @see _evalapi()
 * @see hook_evalapi()
 * @see hook_CONTENT_TYPE_NAME_evalapi()
 * @see hook_evalapi_alter()
 * @see hook_CONTENT_TYPE_NAME_evalapi_alter()
 */
function _evalapi_move_module_before($source = array(), $module = '', $target = '') {
  if (empty($source) || !is_array($source) || 
      empty($module) || empty($target) || 
      !array_key_exists($module, $source) || !array_key_exists($target, $source)) {
    return;
  }
  
  $hash = _evalapi_make_hash();
  
  $tmp[$hash] = $source[$module];
  $source = _evalapi_insert_before($source, $tmp, $target);
  
  unset($source[$module]);
  
  $new[$module] = $tmp[$hash];
  $source = _evalapi_insert_after($source, $new, $hash);
  
  unset($source[$hash]);
  
  return $source;
}





/**
 * Move a module’s position in the evaluation criteria array to be AFTER another
 * module’s entry.
 *
 * @param $source
 *   The source array the module is an element of.
 * @param $module
 *   The machine-name of the module to move.
 * @param $target
 *   The machine-name of the module that $module will be placed after.
 * @return
 *   The modified source array.
 * @see _evalapi_module_move_before()
 * @see _evalapi_insert_before()
 * @see _evalapi_insert_after()
 * @see _evalapi()
 * @see hook_evalapi()
 * @see hook_CONTENT_TYPE_NAME_evalapi()
 * @see hook_evalapi_alter()
 * @see hook_CONTENT_TYPE_NAME_evalapi_alter()
 */
function _evalapi_move_module_after($source = array(), $module = '', $target = '') {
  if (empty($source) || !is_array($source) || 
      empty($module) || empty($target) || 
      !array_key_exists($module, $source) || !array_key_exists($target, $source)) {
    return;
  }
  $hash = _evalapi_make_hash();
  
  $tmp[$hash] = $source[$module];
  $source = _evalapi_insert_after($source, $tmp, $target);
  
  unset($source[$module]);
  
  $new[$module] = $tmp[$hash];
  $source = _evalapi_insert_before($source, $new, $hash);
  
  unset($source[$hash]);
  
  return $source;
}





/**
 * Insert an element into an associative array BEFORE a given key.
 *
 * @param $source
 *   The source array the element is to be inserted into.
 * @param $element
 *   The element to insert into the source array.
 * @param $offset
 *   The name of an existing key in the source array.
 * @return
 *   The modified source array.
 * @see _evalapi_insert_after()
 * @see _evalapi_move_module_before()
 * @see _evalapi_move_module_after()
 * @see _evalapi() 
 * @see hook_evalapi()
 * @see hook_CONTENT_TYPE_NAME_evalapi()
 * @see hook_evalapi_alter()
 * @see hook_CONTENT_TYPE_NAME_evalapi_alter() 
 */
function _evalapi_insert_before($source = array(), $element = '', $offset = '') {
  if (empty($source) || !is_array($source) || 
      empty($element) || empty($offset) || 
      !array_key_exists($offset, $source)) {
    return;
  }
  
  $array = array();
  
  if (is_int($offset)) {
    $array = array_merge(array_slice($source, 0, $offset), $element, array_slice($source, $offset));
  }
  else {
    foreach ($source as $key => $value) {
      if ($key == $offset) {        
        $array = array_merge($array, $element);
      }

      $array[$key] = $value;            
    }
  }

  return $array;
}





/**
 * Insert an element into an associative array AFTER a given key.
 *
 * @param $source
 *   The source array the element is to be inserted into.
 * @param $element
 *   The element to insert into the source array.
 * @param $offset
 *   The name of an existing key in the source array.
 * @return
 *   The modified source array.
 * @see _evalapi_insert_before()
 * @see _evalapi_move_module_before()
 * @see _evalapi_move_module_after()
 * @see _evalapi()  
 * @see hook_evalapi()
 * @see hook_CONTENT_TYPE_NAME_evalapi()
 * @see hook_evalapi_alter()
 * @see hook_CONTENT_TYPE_NAME_evalapi_alter() 
 */
function _evalapi_insert_after($source = array(), $element = '', $offset = '') {
  if (empty($source) || !is_array($source) || 
      empty($element) || empty($offset) || 
      !array_key_exists($offset, $source)) {
    return;
  }
  
  if (is_int($offset)) {
    $array = array_merge(array_slice($source, 0, $offset + 1), $element, array_slice($source, $offset + 1));
  }
  else {
    foreach ($source as $key => $value) {
      $array[$key] = $value;
      
      if ($key == $offset) {
        $array = array_merge($array, $element);
      }
    }
  }
  
  return $array;
}