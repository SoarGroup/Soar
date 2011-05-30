=================================
Server-Side Web Learning Demo
Author: Nate Derbinsky
=================================

To install...
1. Copy the agent.soar and index.php file to a web-accessible directory.

2. Copy PHP_sml_ClientInterface.php from the Soar output "lib" directory to the destination in #1.

3. Optional: make the destination in #1 writeable by the web server, in order to save RL rules between requests (example: "chmod 777 <dir>").


Notes...
- The included agent is a slight modification of the water-jug-rl demo agent included with Soar: the initialization application and the goal-detection elaboration rules condition upon input-link structures to dynamically generate water-jug problem instances.
