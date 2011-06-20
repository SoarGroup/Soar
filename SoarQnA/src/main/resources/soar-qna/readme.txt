SoarQnA

Author: Nate Derbinsky, nlderbin@umich.edu
Date  : 2011


Abstract
--------
SoarQnA facilitates agent access to external 
knowledge stores via the io system.

The proposal presentation for SoarQnA (same
directory as this file) covers desired 
objectives and a system overview in greater
detail (though is inaccurate as to the IO
datamap).


Quick Usage Guide
-----------------
All of these steps are discussed in more
detail below:

1. build SoarQnA
2. develop desired data source drivers
3. describe driver instances in configuration
   files
4. create a Soar kernel instance accessible
   remotely (such as a debugger)
5. from a terminal, run...
   java SoarQnA <host> <port> <agent> <config>
   (where host/port/agent correspond to step #4
   and config to step #3)
6. run your agent


Building SoarQnA
----------------
SoarQnA is a Java application so should work
well on any platform (note: it was developed 
and has only been tested using JDK6).

The class path should include the lib folder 
and Soar SML java components in addition to any 
other classes (such as JDBC drivers).

A SQLite JDBC driver has been included in the
lib directory for demonstration/demo purposes.


Developing Data Source Drivers
------------------------------
If you wish to develop custom data source
drivers, see included demos for guidance.
The big picture is as simple as implementing 
a set of interfaces:

DataSourceDriver: creates instance connections
                  to a data source.
                  
DataSourceConnection: creates QueryState instances
                      given query parameters and 
                      disconnects an instance
                      connection when appropriate.

QueryState: maintains all necessary state for an
            executed query and allows for
            incremental access to results.
 
 
Data Source Instance Configuration
----------------------------------
SoarQnA requires two levels of configuration:
  1) distinct instance enumeration
  2) individual instance description
  
Instance enumeration (the main configuration
file) associates unique ids to data source
instances (as described in separate files).
A sample is provided in qna.ini.
 
Instances are described in their own files.
These require a driver name, instance parameters, 
and enumeration of all available queries. Samples
are enumerated in qna.ini.
 
 
Soar Systems
------------
SoarQnA simply registers for callbacks and thus Soar
systems require no modification, aside from being
accessible remotely.
 
 
Running SoarQnA
---------------
The program arguments include standard information to
find a remote Soar instance, as well as the main
configuration file (described above, such as qna.ini).
It is the user's responsibility to provide proper access
to Soar SML components (such as via environmental
variables).
 
The program will provide status information to standard
out as to each of the data source instances in 
configuration. It then registers for callbacks and waits
for the attached Soar kernel to shutdown, at which point
it quits.


Soar Agents
-----------
Once running, SoarQnA will create on the agent input-link
a "registry" of all available data source instances and
their associated queries. Agents should condition upon this
structure for issuing queries.

An example agent is provided (qna-test.soar), which runs and
validates a [non-comprehensive] set of unit tests based upon
available sample data source instances/queries.

The basic input/output-link structures are as follows:

1.  An agent issues a "qna-query" that specifies the source,
    query name, query parameters, and how results should be
    provided (all at once or incrementally).
 
2a. If the query is unsucessful, the output command will be 
    augmented with a "^status error" WME. 
   
2b. If the query is successful, the output command will be
    augmented with a "^status success" WME as well as an "id"
    integer-valued WME (which is useful for debugging purposes,
    as well as if a next command is issued for incremental
    results). The first "result" will also augment the command.
    The result identifier has a "features" identifier, with
    key/value pairs below, an integer-valued "num" WME, which 
    indicates the result number, and a "next" WME. The next
    augmentation is either "nil" (no further results), "pending"
    (additional results, incremental), or a recursive identifier
    (additional results, all). 
   
3. Agents can get additional incremental results via the
   "qna-next" output command, which requires a "query" WME,
   with integer value referring to the "id" of a qna-query,
   and receives a status augmentation. If successful, the 
   "result" of the associated query will be changed in-place,
   including the augmentations of the "features" WME, the
   "num" WME, and, if appropriate, the "next" WME.

Note that results, by default, come back during a single output phase. 
Care should be taken to avoid costly queries, which will impact Soar 
reactivity. There is an experimental asynchronous SML module that 
addresses this issue, which can be activated via an extra command-line
parameter. As an example, the test agent executes a "sleep" query, which
will stop synchronous Soar decisions for 5s, whereas asynchronous Soar
decisions will continue unaffected. In this mode, status for qna-query
and qna-next commands will be "pending" until the next result is ready,
at which point it will reflect "complete."
