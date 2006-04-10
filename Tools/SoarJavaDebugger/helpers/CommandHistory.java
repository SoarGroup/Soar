/********************************************************************************************
*
* CommandHistory.java
* 
* Created on 	Nov 22, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package helpers;

import general.* ;

/********************************************************************************************
* 
* Used by other modules to store a history of command issued in this window.
* 
********************************************************************************************/
public class CommandHistory
{	
	/** Max number of items to store in the history */
	public static final int kMaxHistorySize = 20 ;
	
	/** The current history */
	private String[] 	  m_History = new String[kMaxHistorySize] ;
	
	/** The current number of items in the history list */
	private int			  m_HistorySize = 0 ;
	
	/** Returns the array of items in the current history */
	public String[]		getHistory() { return m_History ; }
	
	public CommandHistory()
	{
		// Start with empty strings, not null strings
		for (int i = 0 ; i < m_History.length ; i++)
		{
			m_History[i] = "" ;
		}
	}
	
	// Note: This is not case-sensitive.  Should it be?
	private int FindInHistoryList(String item)
	{
		for (int i = 0 ; i < m_HistorySize ; i++)
			if (m_History[i].equalsIgnoreCase(item))
				return i ;
				
		return -1 ;
	}
	
	/********************************************************************************************
	* 
	* Add a new item to the top of the list.
	* Optionally, if the item is already in the list we just move that item up to the top.
	* It depends on the semantics you'd like to have for the history.
	* 
	* @param	newItem			The new string to add
	* @param	allowMovement	If true and the string is in the list already, move its position in the list
	*
	********************************************************************************************/
	public void UpdateHistoryList(String newItem, boolean allowMovement)
	{
		if (newItem == null)
			throw new IllegalArgumentException("Not allowed to store null as a new value in Combo Box History list") ;

		// Find out if this string is already in our list.		
		int pos = FindInHistoryList(newItem) ;
		if (pos != -1 || !allowMovement)
		{
			// Move this item to the top of the list (nothing is added)
			for (int i = pos ; i > 0 ; i--)
				m_History[i] = m_History[i-1] ;
				
			m_History[0] = newItem ;
		}
		else
		{
			// Add the new item to the top of the list
			m_HistorySize++ ;
			
			if (m_HistorySize > kMaxHistorySize)
				m_HistorySize = kMaxHistorySize ;
				
			for (int i = m_HistorySize-1 ; i > 0 ; i--)
			{
				m_History[i] = m_History[i-1] ;
			}
			
			m_History[0] = newItem ;
		}
	}
	
	public String getMostRecent()
	{
		return m_History[0] ;
	}
	
	// Find the first string that matches command up to the length of command
	public String getMatch(String command)
	{
		int len = command.length() ;
		
		for (int i = 0 ; i < m_HistorySize ; i++)
		{
			if (m_History[i].length() < len)
				continue ;
			
			if (m_History[i].substring(0, len).equalsIgnoreCase(command))
				return m_History[i] ;
		}
		
		// If there's no match, just return the command we're trying to match
		return command ;
	}
	
	/************************************************************************
	* 
	* Converts this object into an XML representation.
	* 
	*************************************************************************/
	public general.JavaElementXML ConvertToXML(String title)
	{
		JavaElementXML element = new JavaElementXML(title) ;
		
		// It's useful to record the class name to uniquely identify the type
		// of object being stored at this point in the XML tree.
		Class cl = this.getClass() ;
		element.addAttribute(JavaElementXML.kClassAttribute, cl.getName()) ;
		
		// Record the size of the array
		element.addAttribute("Size", Integer.toString(this.m_HistorySize)) ;
		
		// Record the array
		for (int i = 0 ; i < this.m_HistorySize ; i++)
		{
			String item = "Item" + Integer.toString(i) ;
			element.addAttribute(item, this.m_History[i]) ;
		}
		
		return element ;
	}
	
	/************************************************************************
	* 
	* Rebuild the object from an XML representation.
	* 
	* @param doc			The document we're rebuilding
	* @param element		The XML representation of this command
	* 
	*************************************************************************/
	public void LoadFromXML(doc.Document doc, general.JavaElementXML element) throws Exception
	{
		m_HistorySize = element.getAttributeIntThrows("Size") ;
		
		if (m_HistorySize < 0) m_HistorySize = 0 ;
		if (m_HistorySize > kMaxHistorySize) m_HistorySize = kMaxHistorySize ;
		
		for (int i = 0 ; i < this.m_HistorySize ; i++)
		{
			String item = "Item" + Integer.toString(i) ;
			m_History[i] = element.getAttributeThrows(item) ;
		}
	}

}
