/********************************************************************************************
*
* NameRegister.java
* 
* Description:	
* 
* Created on 	Mar 20, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package doc;

import java.util.HashMap;

import modules.AbstractView;

/************************************************************************
 * 
 * Records a list of names of modules that are in use.
 * 
 ************************************************************************/
public class NameRegister
{
	private HashMap m_NameMap = new HashMap() ;
	
	public boolean isNameInUse(String name)
	{
		return m_NameMap.get(name) != null ;
	}
	
	public void clear()
	{
		m_NameMap.clear() ;
	}
	
	public AbstractView getView(String name)
	{
		return (AbstractView)m_NameMap.get(name) ;
	}
	
	/** Records that a particular window is using a given name */
	public void registerName(String name, AbstractView view)
	{
		if (m_NameMap.get(name) != null)
			throw new IllegalStateException("Registering a name that is already in use") ;
		
		m_NameMap.put(name, view) ;
	}
	
	public void unregisterName(String name)
	{
		if (m_NameMap.get(name) == null)
			throw new IllegalStateException("Unregistering a name that is not in use") ;
		
		m_NameMap.remove(name) ;
	}
}
