package edu.umich.soar.debugger.jmx;

public interface SoarCommandLineMXBean {
    public String getName();
    public String executeCommandLine(String line);
}
