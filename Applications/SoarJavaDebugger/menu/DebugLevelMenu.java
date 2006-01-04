/********************************************************************************************
*
* DebugLevelMenu.java
* 
* Description:	
* 
* Created on 	Apr 12, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package menu;

import org.eclipse.swt.widgets.Menu;

import debugger.MainFrame;
import doc.Document;

/************************************************************************
 * 
 * The debug level (watch) menu
 * 
 ************************************************************************/
public class DebugLevelMenu
{
	private BaseMenu  m_Menu ;

	private MainFrame m_Frame = null ;
	private Document  m_Document = null ;
	
	private AbstractAction m_WatchStatus 	= new AbstractAction("Show current &watch status")  { public void actionPerformed(ActionEvent e) { watchStatus(e) ; } } ;
	private AbstractAction m_WatchDecisions = new AbstractAction("1. Decisions")  { public void actionPerformed(ActionEvent e) { watchDecisions(e) ; } } ;
	private AbstractAction m_WatchPhases    = new AbstractAction("2. Phases")  { public void actionPerformed(ActionEvent e) { watchPhases(e) ; } } ;
	private AbstractAction m_WatchAllProductions  	= new AbstractAction("3. All productions")  { public void actionPerformed(ActionEvent e) { watchAllProductions(e) ; } } ;
	private AbstractAction m_WatchNoProductions   	= new AbstractAction("3. No productions")  { public void actionPerformed(ActionEvent e) { watchNoProductions(e) ; } } ;
	private AbstractAction m_WatchUserProductions   = new AbstractAction("   3a. User productions")  { public void actionPerformed(ActionEvent e) { watchUserProductions(e) ; } } ;
	private AbstractAction m_WatchChunks   			= new AbstractAction("   3b. Chunks")  { public void actionPerformed(ActionEvent e) { watchChunks(e) ; } } ;
	private AbstractAction m_WatchJustifications   	= new AbstractAction("   3c. Justifications")  { public void actionPerformed(ActionEvent e) { watchJustifications(e) ; } } ;
	private AbstractAction m_WatchWmes   			= new AbstractAction("4. Wmes")  		{ public void actionPerformed(ActionEvent e) { watchWmes(e) ; } } ;
	private AbstractAction m_WatchPreferences   	= new AbstractAction("5. Preferences")  { public void actionPerformed(ActionEvent e) { watchPreferences(e) ; } } ;

	private AbstractAction m_WatchNone   			= new AbstractAction("Watch &nothing")  		{ public void actionPerformed(ActionEvent e) { watchNone(e) ; } } ;
	private AbstractAction m_WatchOne   			= new AbstractAction("Watch level &1 only")  { public void actionPerformed(ActionEvent e) { watchOne(e) ; } } ;
	private AbstractAction m_WatchTwo   			= new AbstractAction("Watch level 1-&2 only")  { public void actionPerformed(ActionEvent e) { watchTwo(e) ; } } ;
	private AbstractAction m_WatchThree   			= new AbstractAction("Watch level 1-&3 only")  { public void actionPerformed(ActionEvent e) { watchThree(e) ; } } ;
	private AbstractAction m_WatchFour   			= new AbstractAction("Watch level 1-&4 only")  { public void actionPerformed(ActionEvent e) { watchFour(e) ; } } ;
	private AbstractAction m_WatchFive   			= new AbstractAction("Watch level 1-&5")  { public void actionPerformed(ActionEvent e) { watchFive(e) ; } } ;

	private AbstractAction m_WmesNone   			= new AbstractAction("Production wme detail - none")  		{ public void actionPerformed(ActionEvent e) { wmesNone(e) ; } } ;
	private AbstractAction m_WmesTimeTags   		= new AbstractAction("Production wme detail - time tags")  { public void actionPerformed(ActionEvent e) { wmesTimeTags(e) ; } } ;
	private AbstractAction m_WmesFull   			= new AbstractAction("Production wme detail - full")  		{ public void actionPerformed(ActionEvent e) { wmesFull(e) ; } } ;

	private AbstractAction m_WatchAliases   		= new AbstractAction("Watch aliases")  								{ public void actionPerformed(ActionEvent e) { watchAliases(e) ; } } ;
	private AbstractAction m_WatchProductionLoading = new AbstractAction("Watch production loading")  					{ public void actionPerformed(ActionEvent e) { watchProductionLoading(e) ; } } ;
	private AbstractAction m_WatchLearnPrint   		= new AbstractAction("Watch chunks/justifications as created")  	{ public void actionPerformed(ActionEvent e) { watchLearning(e) ; } } ;
	private AbstractAction m_WatchBacktracing   	= new AbstractAction("Watch backtracing as chunks created")  		{ public void actionPerformed(ActionEvent e) { watchBacktracing(e) ; } } ;

	/** Create this menu */
	public static DebugLevelMenu createMenu(MainFrame frame, Document doc, String title)
	{
		DebugLevelMenu menu = new DebugLevelMenu() ;
		menu.m_Frame    = frame ;
		menu.m_Document = doc ;
		
		menu.m_Menu = menu.makeMenu(frame.getMenuBar(), title) ;
		
		return menu ;
	}
	
	private BaseMenu makeMenu(Menu parent, String title)
	{
		BaseMenu menu = new BaseMenu(parent, title) ;
		
		menu.add(m_WatchStatus) ;
		menu.addSeparator() ;
		menu.addCheckedItem(m_WatchDecisions, true) ;
		menu.addCheckedItem(m_WatchPhases, false) ;
		menu.add(m_WatchAllProductions) ;
		menu.add(m_WatchNoProductions) ;
		menu.addCheckedItem(m_WatchUserProductions, false) ;
		menu.addCheckedItem(m_WatchChunks, false) ;
		menu.addCheckedItem(m_WatchJustifications, false) ;
		menu.addCheckedItem(m_WatchWmes, false) ;
		menu.addCheckedItem(m_WatchPreferences, false) ;
		menu.addSeparator() ;
		menu.add(m_WatchNone) ;
		menu.add(m_WatchOne) ;
		menu.add(m_WatchTwo) ;
		menu.add(m_WatchThree) ;
		menu.add(m_WatchFour) ;
		menu.add(m_WatchFive) ;
		menu.addSeparator() ;
		menu.addCheckedItem(m_WmesNone, true) ;
		menu.addCheckedItem(m_WmesTimeTags, false) ;
		menu.addCheckedItem(m_WmesFull, false) ;
		menu.addSeparator() ;
//		menu.addCheckedItem(m_WatchAliases, false) ;
//		menu.addCheckedItem(m_WatchProductionLoading, true) ;
		menu.addCheckedItem(m_WatchLearnPrint, false) ;
		menu.addCheckedItem(m_WatchBacktracing, false) ;

		return menu ;
	}

	private void watchStatus(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getWatchStatusCommand();
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void watchAliases(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getWatchAliasesCommand(m_WatchAliases.isChecked());
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void watchBacktracing(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getWatchBacktracingCommand(m_WatchBacktracing.isChecked());
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void watchLearning(ActionEvent e)
	{
		boolean on = m_WatchLearnPrint.isChecked() ;
		String sourceLine = on ? m_Document.getSoarCommands().getWatchLearnPrintCommand() : m_Document.getSoarCommands().getWatchLearnNoneCommand() ;
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void watchProductionLoading(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getWatchLoadingCommand(m_WatchProductionLoading.isChecked());
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void wmesNone(ActionEvent e)
	{
		m_WmesTimeTags.setChecked(false, false) ;
		m_WmesFull.setChecked(false, false) ;
		String sourceLine = m_Document.getSoarCommands().getWatchWmesNoneCommand();
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void wmesTimeTags(ActionEvent e)
	{
		m_WmesNone.setChecked(false, false) ;
		m_WmesFull.setChecked(false, false) ;
		String sourceLine = m_Document.getSoarCommands().getWatchWmesTimeTagsCommand();
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void wmesFull(ActionEvent e)
	{
		m_WmesNone.setChecked(false, false) ;
		m_WmesTimeTags.setChecked(false, false) ;
		String sourceLine = m_Document.getSoarCommands().getWatchWmesFullCommand();
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void updateWatchItems(int level)
	{
		m_WatchDecisions.setChecked(level > 0, false);
		m_WatchPhases.setChecked(level > 1, false);
		m_WatchUserProductions.setChecked(level > 2, false) ;
		m_WatchChunks.setChecked(level > 2, false) ;
		m_WatchJustifications.setChecked(level > 2, false) ;
		m_WatchWmes.setChecked(level > 3, false) ;
		m_WatchPreferences.setChecked(level > 4, false) ;
	}
	
	private void watchNone(ActionEvent e)
	{
		updateWatchItems(0) ;
 		String sourceLine = m_Document.getSoarCommands().getWatchLevelCommand(0);
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void watchOne(ActionEvent e)
	{
		updateWatchItems(1) ;
		String sourceLine = m_Document.getSoarCommands().getWatchLevelCommand(1);
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void watchTwo(ActionEvent e)
	{
		updateWatchItems(2) ;
		String sourceLine = m_Document.getSoarCommands().getWatchLevelCommand(2);
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void watchThree(ActionEvent e)
	{
		updateWatchItems(3) ;
		String sourceLine = m_Document.getSoarCommands().getWatchLevelCommand(3);
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void watchFour(ActionEvent e)
	{
		updateWatchItems(4) ;
		String sourceLine = m_Document.getSoarCommands().getWatchLevelCommand(4);
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void watchFive(ActionEvent e)
	{
		updateWatchItems(5) ;
		String sourceLine = m_Document.getSoarCommands().getWatchLevelCommand(5);
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void watchDecisions(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getWatchDecisionsCommand(m_WatchDecisions.isChecked());
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void watchPhases(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getWatchPhasesCommand(m_WatchPhases.isChecked());
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void watchAllProductions(ActionEvent e)
	{
		m_WatchUserProductions.setChecked(true, true) ;
		m_WatchChunks.setChecked(true, true) ;
		m_WatchJustifications.setChecked(true, true) ;
	}

	private void watchNoProductions(ActionEvent e)
	{
		m_WatchUserProductions.setChecked(false, true) ;
		m_WatchChunks.setChecked(false, true) ;
		m_WatchJustifications.setChecked(false, true) ;
	}

	private void watchUserProductions(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getWatchUserProductionsCommand(m_WatchUserProductions.isChecked());
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void watchChunks(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getWatchChunksCommand(m_WatchChunks.isChecked());
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void watchJustifications(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getWatchJustificationsCommand(m_WatchJustifications.isChecked());
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}
	
	private void watchWmes(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getWatchWmesCommand(m_WatchWmes.isChecked());
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void watchPreferences(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getWatchPreferencesCommand(m_WatchPreferences.isChecked());
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}
	
	
}
