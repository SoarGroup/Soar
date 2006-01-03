package edu.umich.tanksoar;

import java.io.*;
import java.util.Date;

public class TankSoarLogger
{
  private static Writer output = null;
  
  public static void log(String message)
  {
    if(output == null)
    {
	    try
	    {
	      output = new BufferedWriter(new FileWriter(new File("TSOutLog.txt")));
        output.write("This log started at " + (new Date()).toString() + "\n");
	    }
	    catch(IOException e){}
    }//if 
      
    try
    {
      output.write(message + "\n");
      output.flush();
    }
    catch(IOException e){}
  }//log
  
  public static void close()
  {
    try
    {
      output.close();      
    }
    catch(IOException e){}
  }

}
