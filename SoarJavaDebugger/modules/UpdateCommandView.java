/********************************************************************************************
*
* UpdateCommandView.java
* 
* Description:	
* 
* Created on 	Mar 18, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

/************************************************************************
 * 
* This version of the BaseCommandView updates at the end of each run
* and clears each time a command is issued.
 * 
 ************************************************************************/
public class UpdateCommandView extends BaseCommandView
{
	public UpdateCommandView()
	{
		m_ClearComboEachCommand = false ;
		m_ClearEachCommand = true ;
		m_UpdateOnStop = true ;
	}
}
