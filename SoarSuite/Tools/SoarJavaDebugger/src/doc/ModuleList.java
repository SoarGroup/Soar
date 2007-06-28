/********************************************************************************************
*
* ModuleList.java
* 
* Created on 	Nov 22, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package doc;

import java.util.*;

/********************************************************************************************
* 
* Used to store a list of modules -- which are window classes that can be instantiated to
* create windows in the debugger.
* 
* This list is generally populated through reflection on the "modules" package.
* 
********************************************************************************************/
public class ModuleList
{
	private List	m_List = new ArrayList() ;
	
	public int	size()				{ return m_List.size() ; }
	public void	add(Module m)		{ m_List.add(m) ; }
	public Module get(int index)	{ return (Module)m_List.get(index) ; }
	public Object[] toArray()		{ return m_List.toArray() ; }
}
