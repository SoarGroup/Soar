/********************************************************************************************
*
* SoarCommands.java
* 
* Created on 	Nov 23, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package doc;

/********************************************************************************************
* 
* Information about strings used for Soar Commands is stored here.
* 
* This class potentially needs to support different commands for different
* versions of Soar.  The idea is we allow the user to select the version of Soar
* they're working with and then we construct the appropriate instance of this
* class and get back the right command strings to use for that version.
* 
********************************************************************************************/
public class SoarCommands
{
	private int	m_MajorVersion ;		// Soar 8.6.0 -- this is the 8
	private int m_MinorVersion ;		// Soar 8.6.0 -- this is the 6
	private int m_BuildVersion ;		// Soar 8.6.0 -- this is the 0
	
	public SoarCommands(int major, int minor, int build)
	{
		setVersion(major, minor, build) ;
	}
	
	public void setVersion(int major, int minor, int build)
	{
		m_MajorVersion = major ;
		m_MinorVersion = minor ;
		m_BuildVersion = build ;
	}
	
	public boolean isRunCommand(String command)
	{
		String lower = command.toLowerCase() ;
		
		return (lower.startsWith("run")) ;
	}
}
