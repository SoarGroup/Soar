# Soar

This is the code base for Soar, a cognitive architecture for developing systems that exhibit intelligent behavior. For more information about this project, please visit:

* [The Soar home page](http://soar.eecs.umich.edu/)
* [The GitHub project page](https://github.com/SoarGroup/Soar)

# Soar Builds

For binary builds of Soar you can get them in two places:

* [Official Releases](http://soar.eecs.umich.edu/articles/downloads/soar-suite)
* [Latest Successful Development Build](http://soar-jenkins.eecs.umich.edu/Nightlies/)
  * Note: If there isn't a build for your platform or if the 7zip archive is older than the others, the build failed.  You can check [here](https://github.com/SoarGroup/Soar/branches) to see if it successfully built on all platforms.

# Soar Performance

Some performance statistics are calculated automatically using the Factorization Stress Tests.  You can see performance on a commit-by-commit basis either in [Performance.md](https://github.com/SoarGroup/Soar/blob/development/Performance.md) or [here](http://soar-jenkins.eecs.umich.edu/Performance/). The raw data used to generate the graphs for each build can be found [here](http://soar-jenkins.eecs.umich.edu/Performance/).

Disclaimer: These are worst case tests.  Average performance is probably much higher.  In addition, these show that even in worst case, Soar beats its goal of 50 msec reactivity (in these tests, the max is ~30msec per decision).

# License

Soar is available under the following [LICENSE](https://github.com/SoarGroup/Soar/blob/development/LICENSE.md).  This license is [BSD](http://opensource.org/licenses/BSD-2-Clause)
