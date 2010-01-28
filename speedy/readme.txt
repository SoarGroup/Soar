Speedy

Author: Nate Derbinsky, nlderbin@umich.edu
Date  : 2010


Abstract
--------
Speedy is a lightweight platform for logging, querying, and visualizing 
experiment data. Much functionality requires no coding, but can extend
to arbitrary SQL querying and reporting/visualizing.


Quick Usage Guide
-----------------
1. Setup the Speedy server
2. Create an experiment
3. Publish experiment data
4. Query, download, and/or visualize data
5. Develop dynamic reporting pages


The Speedy Server
-----------------
The Speedy server is a PHP+MySQL web-based application. First setup 
the database, then deal with the PHP components.

First create a brand new database on your server. Speedy dynamically 
adds/removes tables, so it needs its space and doesn't appreciate 
being lumped in with other tables. Then create a new user, just for 
Speedy. Give it all non-administrative rights (might be overkill, 
but it at least needs data level and the ability to add/remove tables 
and indexes).

There are two possible routes for populating the database: bare or with 
example data. The code-base includes some example reports that will *only* 
work properly if you use the latter. If you want a clean install, import 
"schema.sql" from the "reporting/dbms" folder. Otherwise, "example.sql" 
includes the schema and pre-populated data.

Now copy the entire contents of the "reporting/web" folder to PHP-enabled
web space. Edit the $db_info variable in "common/private/db.inc.php" to 
reflect database connection information. Then edit the SYSTEM_URL constant
in "common/private/start.inc.php" to the web-accessible URL to your Speedy
install (with a final slash). Done!


Experiments
-----------
In Speedy, an "experiment" is defined by a unique id and a static schema. A
schema is just a set of fields (where a field is a name/type pair). Fields can
be of type integer, string, or double (used to improve query performance and
aesthetic formatting).

Load Speedy in your browser and click the "experiments" link at the top. This page
is included with Speedy to allow you to create new experiments, view/query/clear/add 
data in existing experiments, and drop experiments.

To create a new experiment, click the "New" tab. Give the experiment a name (this must
be unique) and define as many fields as desired, clicking save when satisfied.

Adding data is done from the "Existing" tab. You can manually enter data into the form
provided for every experiment. You can also publish data, as discussed in another section
below, from any program on any computer than can access Speedy from the web.

Also on this tab are useful tools for populated experiments. The experiment is listed in
the following format:

[Experiment ID]: [Experiment Name], [Number of Published Data Points]

There are also links to "view" data (discussed more later), "clear" all the data in an
experiment, and "drop" the experiment (and all of its data) all-together.


Publishing Data
---------------
As discussed in the preceding section, you can publish experiment data manually through
a web form. This form calls the experiments.php file with the following format:

experiments.php?cmd=data&exp_id=[Experiment ID]&field1=value1&field2=value2...

URLs have a standard format of "file?key=value" so, in the above example, the web form
executes the "data" command, providing an experiment id and arbitrary key/value pairs, with
respect to the experiment's schema.
 
Any program that calls a URL of the above format can publish data. This could be from any web
browser, or a command-line tool, such as wget or curl. Speedy comes with a command-line (CLI) PHP
script ("automation/speedy_publish.php") to facilitate massive data publishing. It has the following 
syntax:

php speedy_publish.php [Experiment ID]

The program then reads every line from Standard In as a new data point for the experiment. The lines
must be formatted as "key1=value1 key2=value2 ..." (ordering of the pairs is unimportant) and must
adhere to the schema of the experiment. This script will issue the URL and verify receipt, delivering
success/failure for each data point to Standard Out. Before first use, set the SPEEDY_BASE_URL constant
at the top of this file. Note: this script uses the PHP-curl library which is standard on Darwin, but
requires a package on Ubuntu (sudo apt-get install php5-curl).

The "automation/blocks-world" folders provide example programs in C++ and Java that produce timing/memory
experiment data from running an infinite Soar blocks-world agent. To build, run the "build.sh" script. To
run, use the "run.sh" script. These scripts depend upon the dynamic library path environmental variables, 
so non-Darwin platforms need to change DYLD_LIBRARY_PATH in the build/run scripts of each to LD_LIBRARY_PATH.
The "loop.php" script runs one of these connection platforms a set number of times, piping results to the
publishing script described above, thus providing an example of massive data publishing to Speedy.


Querying Data
-------------
Once data is published to an experiment, click the "view" link next to the experiment in the "Existing"
tab of the "experiments" page on the Speedy website. This page provides an HTML table displaying all
published data. It also has a link ("csv") to download the displayed data in comma-separated value format
(which is Excel compatible).

This page can also perform arbitrary SQL selection queries. Rather than exposing internal table and field
names, use the variables described on the page, but otherwise, any selection is valid.

If the resulting query has two columns, Speedy has the potential to automatically visualize the data. If the
columns are named "x" and "y" then Speedy will create a line chart. If they are "bin" and "y" it will create
a bar chart.


Reporting
---------
All the web-based query and visualization tools described thus far build off Speedy experiment library calls,
combined with PHP and MySQL query calls. The Speedy index links to sample reports that issue queries, display
data in tabular format (as well as downloadable CSV), and visualize data sub-parts. These form the basis for
arbitrarily complex dynamic reporting pages.

Page tabs are created using jQuery. The visualizations use the Google Chart API.

