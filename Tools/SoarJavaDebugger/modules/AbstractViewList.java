/********************************************************************************************
*
* AbstractViewList.java
* 
* Description:	
* 
* Created on 	Mar 9, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import java.util.*;

/************************************************************************
 * 
 * A list of abstract views
 * 
 ************************************************************************/
public class AbstractViewList
{
	protected ArrayList	m_List = new ArrayList() ;
	
	public int size()						{ return m_List.size() ; }
	public void add(AbstractView view)		{ m_List.add(view) ; }
	public boolean remove(AbstractView view){ return m_List.remove(view) ; }
	public AbstractView get(int index)		{ return (AbstractView)m_List.get(index) ; }
	public void clear()						{ m_List.clear() ; }
}
