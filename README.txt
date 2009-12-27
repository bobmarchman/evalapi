$Id

CONTENTS OF THIS FILE
---------------------

 * Introduction
 * Installation
 * Usage
 * Eval API hooks
 * Examples/Recipes
 * TODO


INTRODUCTION
------------

The Eval API module provides an API for other modules to utilize that 
facilitates the adding of arbitrary criteria to entities that can be evaluated
as seen fit by the implementing module. Put simply, nodes can be given a set of 
arbitrary criteria that can be evaluated for a given purpose. Other modules can 
use these criteria and the evaluated results however they see fit. 

The module provides a field that is added to an Eval API-enabled node’s add/edit form
that accepts custom PHP code that is evaluated. The returned result stored in the database. 
The Eval API uses this code and the returned results from other modules that use the Eval API
to constuct an eval criteria array for that given node. You can think of it as a "truth table", of sorts, 
for nodes.


INSTALLATION
------------

1. Visit admin/build/modules and enable the Eval API module.

2. Visit admin/settings/evalapi to set which content types should be enabled to
   use the Eval API module.

3. Visit admin/user/permissions#module-evalapi to configure permissions.


USAGE
-----

The Eval API provides four hooks that modules can implement.

The first two hooks operate on a per-node basis:
  hook_evalapi(&node)
  hook_evalapi_alter(&node)
  
The last two hooks operate on a per-content type basis:
  hook_CONTENT_TYPE_NAME_evalapi(&node, $eval_criteria)
  hook_CONTENT_TYPE_NAME_evalapi_alter(&node, $eval_criteria)
  

hook_evalapi() and hook_CONTENT_TYPE_NAME_evalapi() each accept 
a reference to a node object and returns a keyed array of boolean values.

Here is a simple example implementation:

<?php

/**
 * Implementation of hook_evalapi().
 */
function MODULENAME_evalapi(&$node) {
  return array('some_unique_identifier' => TRUE);
}

?>

This array will be merged into all other arrays returned from modules 
implementing hook_evalapi() and hook_CONTENT_TYPE_NAME_evalapi().

The array that results from the merging is referred to as the node’s eval 
criteria array. Module’s have the ability to override or modify any part of a 
node’s eval criteria array by implementing either of the hooks:

hook_evalapi_alter()
hook_CONTENT_TYPE_NAME_evalapi_alter()

In addition to accepting a node object as the first parameter, these two hooks 
are also passed the eval criteria array as the second parameter. As these
two hooks utilize drupal_alter() to modify the eval criteria array, there
is no need to explicitly return it.


EXAMPLE USE-CASES AND HOW-TO
----------------------------

Use-case: Eligible to vote?

  You have a poll node type with voting and you want to easily be able to setup custom rules
  for determining a user’s eligibility to vote. You want the ability to setup default rules that
  apply to all polls, in addition to being able to very easily add new rules on a per-poll basis.
  For example, you want all polls to require that a user have filled out a specific profile field
  before they are considered eligible to vote, but on one specific poll, you want to disable this 
  feature so that anonymous users (users without a profile) can vote.
  
How-to:  
  
  
  
****************************    
  
  
Use-case: Node/Node Field View Access
 
  You have a company intranet blog post with a file attachment for download, but you only want users 
  viewing your site from a specific range of ip addresses to be able to view and download the file attachment.

How-to:



****************************



Use-case: Notification Trigger
 
  You want the site administrator to receive an email notification whenever a user belonging to a certain role 
  edits and saves any node during a specific range of time during the day.
  
How-to:



****************************


TODO
----


- Add support for eval criteria to be associated with entities other than nodes: i.e. users, comments, forums, views, panels, etc…
- Add the eval criteria array to the node object on load. Test it works when using on node view and the node page has a form submission (i.e. polls).


