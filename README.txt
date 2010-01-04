$Id$

CONTENTS OF THIS FILE
---------------------

 * Introduction
 * Installation
 * Usage
 * Example Use-cases
 * More Information


INTRODUCTION
------------

The Eval API module provides an API for other modules to utilize that
facilitates the adding of arbitrary criteria to content types and nodes that can
be evaluated as seen fit by the implementing module. Put simply, nodes can be
given a set of arbitrary criteria that can be evaluated for a given purpose.
Other modules can use these criteria and the evaluated results however they see
fit.

The Eval API module facilitates this with two concepts:

1. By building a "truth table" for the node using array key => value
   combinations supplied by other modules and input in the UI.
2. By evaluating this "truth table" using a pre-defined or custom handler.

The "truth table" is referred to throughout the Eval API as the node’s
"evaluation criteria". This table is nothing more than an associative array in
the following format:

<?php

$criteria = array(
  'implementing_module_name' => array(
    'some_arbitrary_value' => TRUE,
    'some_other_value' => FALSE,
  ),
  'another_implementing_module' => array(
    'some_value' => TRUE,
  ),
);

?>


Where the top-level keys is the name of the module adding criteria and its value
is an associative array with its keys as descriptive identifiers with boolean
values. A more recognizable example may be:

<?php

$criteria = array(
  'node' => array(
    'author_is_not_self' => TRUE,
  ),
  'poll' => array(
    'today_is_wednesday' => TRUE,
  ),
);

?>


Here we see two module names: node and poll. Each has defined a single criteria
will be evaluated as a part of the whole. Eval API uses an "evaluation handler"
to determine the final boolean value when all the node’s criteria have been
evaluated.

By default, Eval API’s default evaluation handler simply AND’s all the criteria
values together. By using Eval API’s provided hooks, as well as the ability to
override evaluation criteria and evaluation handlers on a per-content type AND
per-node basis, a great deal of flexibility is offered to accommodate complex
business requirements.

Going back to the example above, we could say that our business requirement is
to disallow a user from voting on a specific poll if that user is the poll’s
author and the current day of the week is Wednesday. Looking at our example
criteria, if we used the default evaluation handler, this node’s criteria would
evaluate to TRUE and the user would be allowed to vote.

On the subject of evaluation handlers, it is very simple for a module to define
its own evaluation handler. There are two parts to this process:

1. Inform the Eval API module that you are defining a new evaluation handler by
   implementing hook_evalapi_handler(). This is simply a function that returns
   an array with information about the handler.

2. Define a callback function that will evaluate a node’s evaluation criteria
   and return a boolean result.

Modules can optionally define as many evaluation handlers as they like.

Below is Eval API’s default evaluation handler:

<?php

/**
 * Implementation of hook_evalapi_handler().
 *
 * @see _evalapi_default_handler()
 */
function evalapi_evalapi_handler() {
  return array(
    'evalapi' => array(
      'name' => t('Default'),
      'callback' => '_evalapi_default_handler',
    ),
  );
}

?>


Its format is as follows below:

<?php

$handler = array(
  'implementing_module_name' => array(
    'name' => t('Some human readable name'),
    'callback' => 'callback_function_for_evaluating_criteria',
  ),
);

?>


Below is Eval API’s default evaluation handler function:

<?php

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

?>


It takes a reference to the evaluation criteria array and returns a boolean.

NOTE: It is not required to pass the evaluation criteria as a parameter to the
      handler function. All that is required of module’s implementing their
      own evaluation handlers is that a boolean value is returned from the
      callback function.

It should also be noted that the Eval API does not provide the means for taking
action based on a node’s evaluation. It simply provides the mechanism to define
the criteria to be evaluated and the method of evaluation. Other module’s are
responsible for determining what to do with the evaluated criteria.

Since a node’s evaluation criteria is dynamic, it is not stored in the database,
but rather in the current user’s session and the node object. Each node that is
Eval API enabled will store an array in the user’s session that looks like the
following:

evalapi_eval_value_<NID> => <EVALUATED VALUE>

The property’s value that is attached to the node object follows the same format
and will look like the following:

$node->evalapi['evalapi_eval_value_<NID>] = <EVALUATED VALUE>

Where NID is this nid of the node being evaluated, and EVALUATED VALUE is the
returned boolean result of the called evaluation handler.


INSTALLATION
------------

See INSTALL.txt for install instructions.


USAGE
-----

Included is the Eval API Example module that demonstrates how another module can
use the Eval API. It creates a new content type: Eval API Example that is
automatically configured to use the Eval API module. The example module contains
and simple implementation of each hook the Eval API module provides and also
includes a sample evaluation handler and callback function.


EXAMPLE USE-CASES
-----------------

* Eligible to vote?
  You have a poll node type with voting and you want to easily be able to setup custom rules
  for determining a user’s eligibility to vote. You want the ability to setup default rules that
  apply to all polls, in addition to being able to very easily add new rules on a per-poll basis.
  For example, you want all polls to require that a user have filled out a specific profile field
  before they are considered eligible to vote, but on one specific poll, you want to disable this
  feature so that anonymous users (users without a profile) can vote.

* Node/Node field access
  You have a company intranet blog post with a file attachment for download, but you only want users
  viewing your site from a specific range of ip addresses to be able to view and download the file attachment.

* Custom notification trigger
  You want the site administrator to receive an email notification whenever a user belonging to a certain role
  edits and saves any node during a specific range of time during the day.


MORE INFORMATION
----------------

See evalapi.api.php for example Eval API hook implementations.

github homepage: http://github.com/bobmarchman/evalapi
github repo:     git://github.com/bobmarchman/evalapi.git

This project is funded by South Central Media http://southcentralmedia.com