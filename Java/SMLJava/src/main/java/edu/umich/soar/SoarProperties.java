package edu.umich.soar;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

public class SoarProperties
{
	public SoarProperties()
	{
	}
	
	public String getPrefix()
	{
		String prefix = tryJarLocation();
		if (prefix != null) 
			return prefix;
		
		prefix = trySoarHome();
		if (prefix != null) 
			return prefix;

		// TODO: try looking at relative paths?
		
		return null;
	}
	
	private String tryJarLocation() 
	{
		String codeLoc = SoarProperties.class.getProtectionDomain().getCodeSource().getLocation().getFile();
		
		// Are we being run from a jar?
		if (codeLoc.endsWith("jar")) 
		{
			// need to do three times:
			//     /prefix/share/java/thejar.jar
			// --> /prefix/share/java
			// --> /prefix/share
			// --> /prefix
			for (int i = 0; i < 3; ++i)
			{
				int index = codeLoc.lastIndexOf(File.separator);
				if (index < 0)
				{
					codeLoc = null;
				}
				codeLoc = codeLoc.substring(0, index);
			}
			return codeLoc;
		}
		
		return null;
	}
	
	private String trySoarHome()
	{
		// If not being run from a jar, we should try soar.home
		Properties properties = new Properties();
		
		InputStream is = ClassLoader.getSystemResourceAsStream("soar.properties");
		if (is == null) 
			return null;
		
		try 
		{
		    properties.load(is);
		} catch (IOException e) 
		{
			// No properties. TODO: try some other paths relative to CWD?
		    e.printStackTrace();
		    return null;
		}
		
		return properties.getProperty("soar.home");
	}
	
	public static void main(String[] args)
	{
		SoarProperties p = new SoarProperties();
		System.out.println("tryJarLocation: " + p.tryJarLocation());
		System.out.println("trySoarHome: " + p.trySoarHome());
	}

}
