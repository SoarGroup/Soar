/** 
 * AppProperties.java
 *
 * Description:		<describe the AppProperties class here>
 * @author			Doug
 * @version			
 */

package general;

import java.io.* ;

public class AppProperties extends java.util.Properties
{
	protected String m_Filename;
	protected String m_Header ;
	protected final String kVersion = "Version" ;

	/** The filename is just a filename--not a full path.  The file is
	*** stored relative to the user's home directory on Unix or
	*** the user's application data directory on Windows.
	**/
	public AppProperties(String filename, String header)
	{
		m_Filename = filename ;
		m_Header   = header ;
	}
	
	public void setFilename(String filename)
	{
		m_Filename = filename ;
	}
	
	// Key Description of Associated Value for System.getProperty() calls.
	//
	// java.version Java Runtime Environment version 
	// java.vendor Java Runtime Environment vendor 
	// java.vendor.url Java vendor URL 
	// java.home Java installation directory 
	// java.vm.specification.version Java Virtual Machine specification version 
	// java.vm.specification.vendor Java Virtual Machine specification vendor 
	// java.vm.specification.name Java Virtual Machine specification name 
	// java.vm.version Java Virtual Machine implementation version 
	// java.vm.vendor Java Virtual Machine implementation vendor 
	// java.vm.name Java Virtual Machine implementation name 
	// java.specification.version Java Runtime Environment specification version 
	// java.specification.vendor Java Runtime Environment specification vendor 
	// java.specification.name Java Runtime Environment specification name 
	// java.class.version Java class format version number 
	// java.class.path Java class path 
	// java.ext.dirs Path of extension directory or directories 
	// os.name Operating system name 
	// os.arch Operating system architecture 
	// os.version Operating system version 
	// file.separator File separator ("/" on UNIX) 
	// path.separator Path separator (":" on UNIX) 
	// line.separator Line separator ("\n" on UNIX) 
	// user.name User's account name 
	// user.home User's home directory 
	// user.dir User's current working directory 
		
	// Returns the path to the properties file.
	public static File GetSettingsFilePath(String filename)
	{
		String homeDirName = System.getProperty("user.home");
		String osName = System.getProperty("os.name");
	
		File homeDir  = new File(homeDirName);

		// If this is Windows we want to store our settings under
		// the "Application Data" folder inside the user's "home"
		// directory (which is C:\Documents and Settings\<user>).
		if (osName.toLowerCase().startsWith("windows"))
		{
			// Add the Application Data folder name.
			// BUGBUG: Not sure how to do this in a language neutral way.
			homeDir = new File(homeDir, "Application Data") ;
			
			// Create a SoarDebugger subfolder if necessary
			homeDir = new File(homeDir, "SoarDebugger") ;
		}
		else
		{
			// For Linux (and Mac?) we'll store things in a ~/.SoarFiles folders.
			homeDir = new File(homeDir, ".SoarFiles") ;
		}

		if (!homeDir.exists())
		{
			// mkdirs makes the entire path
			// (which could matter if we're on a foreign language system with no
			//  ApplicationData folder)
			homeDir.mkdirs() ;
		}

		return new File(homeDir, filename) ;
	}
	
	private File GetFilePath()
	{
		return GetSettingsFilePath(m_Filename) ;
	}
		
	public boolean Load(String version) throws java.io.IOException
	{
		try {
			File filePath = this.GetFilePath() ;
			FileInputStream input = new FileInputStream(filePath) ;
			this.load(input) ;
			
			// If the versions don't match, clear out all of the old preferences.
			if (this.getProperty(kVersion) == null || !this.getProperty(kVersion).equalsIgnoreCase(version))
			{
				this.clear() ;
			}
			
			// Record the current version.
			this.setProperty(kVersion, version) ;
			
			return true ;
		}
		catch (FileNotFoundException e)
		{
			// File doesn't exist.  No properties to load, so we're done.
			return false ;
		}
		
	}
	
	public void Save() throws java.io.IOException
	{
		File filePath = this.GetFilePath() ;
		FileOutputStream output = new FileOutputStream(filePath) ;
		this.store(output, this.m_Header) ;
	}
	
	public String getFilename()
	{
		return m_Filename;
	}
	
	public void setAppProperty(String property, String value)
	{
		this.setProperty(property, value);
	}

	public void setAppProperty(String property, double value)
	{
		this.setProperty(property, Double.toString(value));
	}

	public void setAppProperty(String property, int value)
	{
		this.setProperty(property, Integer.toString(value));
	}

	public void setAppProperty(String property, boolean value)
	{
		this.setProperty(property, String.valueOf(value));
	}

	public String getAppStringProperty(String property)
	{
		return this.getProperty(property);
	}

	public boolean getAppBooleanProperty(String property, boolean defaultValue)
	{
		String value = this.getProperty(property);

		if (value != null)
		{
			return (value.equalsIgnoreCase("true"));
		}

		return defaultValue;
	}

	public double getAppDoubleProperty(String property)
	{
		try
		{
			String value = this.getProperty(property);
			if (value != null)
			{
				double d = Double.parseDouble(value);
				return d;
			}
		} catch (NumberFormatException e)
		{
		}

		return Double.NaN;
	}

	public int getAppIntegerProperty(String property)
	{
		try
		{
			String value = this.getProperty(property);
			if (value != null)
			{
				int i = Integer.parseInt(value);
				return i;
			}
		} catch (NumberFormatException e)
		{
		}

		return Integer.MAX_VALUE;
	}	
}


/* @(#)AppProperties.java */
