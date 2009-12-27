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


INSTALLATION
------------

See INSTALL.txt for install instructions.


USAGE
-----

Included is the Eval API Example module that demonstrates how another module can
use the Eval API.


EXAMPLE USE-CASES
-----------------

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


MORE INFORMATION
----------------

See API.txt for Eval API developer documentation and example hook implementations.

github homepage: http://github.com/bobmarchman/evalapi
github repo:     git://github.com/bobmarchman/evalapi.git