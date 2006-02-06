package edu.umich.JavaBaseEnvironment;

import java.io.*;

public class Logger
{
  private Writer output = null;
  private String logPath = null;
  private boolean failSilently = true;//Pretend to log but don't really
  private boolean clearLog = false;
  private int logVerbosity = 0;
  
  public Logger(String path,boolean clLog,int verbosity)
  {
  	clearLog = clLog;
  	logPath = path;
  	failSilently = false;
  }
  
  public Logger(String path)
  {
  	logPath = path;
  	failSilently = false;
  }
  
  public Logger() { }
  
  public void log(String message,int minVerbosity)
  {
	  if (logVerbosity >= minVerbosity)
		  log(message);
  }
  
  public void log(String message)
  {
  	if (!failSilently)
  	{
	    if(output == null)
	    {
		    try
		    {
		      output = new BufferedWriter(new FileWriter(new File(logPath),!clearLog));
		    }
		    catch(IOException e){}
	    }//if 
	      
	    try
	    {
	      output.write(message);
	      output.flush();
	    }
	    catch(IOException e){}
	  }//log
  }
  
  public void close()
  {
  	if (!failSilently)
  	{
  		try
	    {
	      output.close();      
	    }
  		catch(IOException e){}
  	}
  }
}
