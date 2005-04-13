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
	private Document m_Document ;
	
	/** At this point we just the command.  But in later versions of the debugger we might use the version
	 *  of Soar to decide what to return (note the "version" command line command returns the Soar version in use) */
	public String getSourceCommand(String arg) 					{ return "source \"" + arg + "\""; }
	public String getPrintCommand(String arg)  					{ return "print " + arg ; }
	public String getPrintDepthCommand(String arg, int depth)  	{ return "print --depth " + depth + " " + arg ; }
	public String getPrintInternalCommand(String arg)  			{ return "print --internal " + arg ; }
	public String getExciseCommand(String arg) 					{ return "excise " + arg ; }
	public String getExciseAllCommand()							{ return "excise --all" ; }
	public String getExciseChunksCommand()						{ return "excise --chunks" ; }
	public String getExciseTaskCommand()						{ return "excise --task" ; }
	public String getExciseUserCommand()						{ return "excise --user" ; }
	public String getPreferencesCommand(String arg) 			{ return "preferences " + arg ; }
	public String getPreferencesNameCommand(String arg)			{ return "preferences " + arg + " --names" ; }
	public String getMatchesCommand(String arg)					{ return "matches " + arg ; }
	public String getMatchesWmesCommand(String arg)				{ return "matches " + arg + " --wmes"; }
	public String getInitSoarCommand()							{ return "init-soar" ; }
	public String setLibraryLocationCommand(String arg)			{ return "set-library-location \"" + arg + "\"" ; }
	public String getLibraryLocationCommand()					{ return "set-library-location" ; }	// No args => get value
	public String getLoadReteCommand(String arg)				{ return "rete-net --load \"" + arg + "\"" ; }
	public String getSaveReteCommand(String arg)				{ return "rete-net --save \"" + arg + "\"" ; }
	public String getLogNewCommand(String arg)					{ return "log \"" + arg + "\"" ; }
	public String getLogAppendCommand(String arg)				{ return "log --append \"" + arg + "\"" ; }
	public String getLogCloseCommand()							{ return "log --close" ; }
	public String getLogStatusCommand()							{ return "log --query" ; }
	public String getWatchStatusCommand()						{ return "watch" ; }
	public String getWatchDecisionsCommand(boolean state)		{ return "watch --decisions" + (state ? "" : " remove") ; }
	public String getWatchPhasesCommand(boolean state)			{ return "watch --phases" + (state ? "" : " remove") ; }
	public String getWatchUserProductionsCommand(boolean state)	{ return "watch --productions" + (state ? "" : " remove") ; }
	public String getWatchChunksCommand(boolean state)			{ return "watch --chunks" + (state ? "" : " remove") ; }
	public String getWatchJustificationsCommand(boolean state)	{ return "watch --justifications" + (state ? "" : " remove") ; }
	public String getWatchWmesCommand(boolean state)			{ return "watch --wmes" + (state ? "" : " remove") ; }
	public String getWatchPreferencesCommand(boolean state)		{ return "watch --preferences" + (state ? "" : " remove") ; }
	public String getWatchLevelCommand(int level)				{ return "watch --level " + level ; }
	public String getWatchWmesNoneCommand()						{ return "watch --nowmes" ; }
	public String getWatchWmesTimeTagsCommand()					{ return "watch --timetags" ; }
	public String getWatchWmesFullCommand()						{ return "watch --fullwmes" ; }
	public String getWatchAliasesCommand(boolean state)			{ return "watch --aliases" + (state ? "" : " remove") ; }
	public String getWatchBacktracingCommand(boolean state)		{ return "watch --backtracing" + (state ? "" : " remove") ; }
	public String getWatchLearnPrintCommand()					{ return "watch --learning print" ; }
	public String getWatchLearnFullCommand()					{ return "watch --learning fullprint" ; }
	public String getWatchLearnNoneCommand()					{ return "watch --learning noprint" ; }
	public String getWatchLoadingCommand(boolean state)			{ return "watch --loading " + (state ? "" : " remove") ; }
	
	public SoarCommands(Document doc, int major, int minor, int build)
	{
		m_Document = doc ;
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
		return m_Document.isRunCommand(command) ;
	}
}
