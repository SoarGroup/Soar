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

import debugger.MainFrame;
import doc.* ;

/********************************************************************************************
* 
* This version of the BaseCommandView does not erase the output window after each command,
* but instead keeps a full trace.
* 
********************************************************************************************/
public class KeepCommandView extends BaseCommandView
{
	public KeepCommandView()
	{
		m_ClearComboEachCommand = false ;
		m_ClearEachCommand = false ;
		m_UpdateOnStop = false ;
	}
}
