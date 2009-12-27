$Id

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

$criteria = array(
  'implementing_module_name' => array(
    'some_arbitrary_value' => TRUE,
    'some_other_value' => FALSE,
  ),
  'another_implementing_module' => array(
    'some_value' => TRUE,
  ),
);

Where the top-level keys is the name of the module adding criteria and its value
is an associative array with its keys as descriptive identifiers with boolean
values. A more recognizable example may be:

$criteria = array(
  'node' => array(
    'author_is_not_self' => TRUE,
  ),
  'poll' => array(
    'today_is_wednesday' => TRUE,
  ),
);

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

It should be noted that the Eval API does not provide the means for taking
action based on a node’s evaluation. It simply provides the mechanism to define
the criteria to be evaluated and the method of evaluation. Other module’s are
responsible for determining what to do with the evaluated criteria.

Since a node’s evaluation criteria is dynamic, it is not stored in the database,
but rather in the current user’s session (for now). Each node that is Eval API
enabled will store an array in the user’s session, and will look like the 
following:

evalapi_eval_value_<NID> => <EVALUATED VALUE>

Where NID is this nid of the node being evaluated, and EVALUATED VALUE is the
returned boolean result of the called evaluation handler.


INSTALLATION
------------

See INSTALL.txt for install instructions.


USAGE
-----

Included is the Eval API Example module that demonstrates how another module can
use the Eval API.


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

See API.txt for Eval API developer documentation and example hook implementations.

github homepage: http://github.com/bobmarchman/evalapi
github repo:     git://github.com/bobmarchman/evalapi.git