/********************************************************************************************
*
* ComboCommandWindowClear.java
* 
* Created on 	Nov 22, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

/********************************************************************************************
* 
* This version of the BaseCommandView does not erase the output window after each command,
* but instead keeps a full trace.
* 
********************************************************************************************/
public class KeepCommandView extends AbstractSingleTextView
{
	public KeepCommandView()
	{
		m_ClearComboEachCommand = false ;
		m_ClearEachCommand = false ;
		m_UpdateOnStop = false ;
		m_ClearComboEachCommand = true ;
	}
	
	/********************************************************************************************
	* 
	* This "base name" is used to generate a unique name for the window.
	* For example, returning a base name of "trace" would lead to windows named
	* "trace1", "trace2" etc.
	* 
	********************************************************************************************/
	public String getModuleBaseName() { return "keep" ; }

}
