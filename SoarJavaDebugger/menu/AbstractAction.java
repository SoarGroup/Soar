/********************************************************************************************
*
* AbstractAction.java
* 
* Description:	
* 
* Created on 	Feb 15, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package menu;

/************************************************************************
 * 
 * My implementation of AbstractAction to bridge the gap from Swing to SWT.
 *
 * I don't believe SWT has an Action concept, but the idea seems sound so
 * I'll implement the class here so I can keep the logic.
 * 
 ************************************************************************/
public abstract class AbstractAction
{
	private String 	m_Label ;
	private boolean m_Enabled ;
	
	public AbstractAction(String label)
	{
		m_Enabled = true ;
		m_Label = label ;
	}
	
	public String getLabel() { return m_Label ; }
	
	public boolean isEnabled()				{ return m_Enabled ; }
	public void   setEnabled(boolean state) { m_Enabled = state ; }
	
	public abstract void actionPerformed(ActionEvent e) ;
}
