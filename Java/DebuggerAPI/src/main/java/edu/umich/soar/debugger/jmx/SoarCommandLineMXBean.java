package edu.umich.soar.debugger.jmx;

/**
 * Java JMX interface to simple Soar command line.
 * 
 * @author voigtjr
 * 
 */
public interface SoarCommandLineMXBean
{
    /**
     * Get the name of the command line interface instance, usually agent name.
     * 
     * @return Name of instance (agent name).
     */
    public String getName();

    /**
     * Execute a command line.
     * 
     * Multiple commands can be included in the line. Syntax legal for .soar
     * files is legal here.
     * 
     * @param line
     *            The command line to execute.
     * @return String result, often empty. Null if error.
     */
    public String executeCommandLine(String line);
}
