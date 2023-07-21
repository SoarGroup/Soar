## Development

To run in IntelliJ (community edition is fine), first compile the `debugger`
target in the `soar` directory with `python3 scons --verbose debugger`. This will populate
the `soar/out/java` directory with jar files we use here as dependencies. Then, load this
directory as an existing project in IntelliJ.

It's not ideal, but PR's are welcome.
