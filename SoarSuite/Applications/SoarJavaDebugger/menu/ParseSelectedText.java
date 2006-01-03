/********************************************************************************************
*
* ParseText.java
* 
* Description:	
* 
* Created on 	Mar 18, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package menu;

import modules.AbstractView;

import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.MenuItem;
import org.eclipse.swt.events.*;

import sml.Agent;

import doc.Document;

/************************************************************************
 * 
 * This class can be used to parse a user's text selection into a more
 * logical construct.  It only parses the text immediately around a
 * given selection point.  (We use this for the context menu)
 * 
 ************************************************************************/
public class ParseSelectedText
{
	public abstract static class SelectedObject
	{
		/********************************************************************************************
		 * Fills in menu items that are appropriate for this type of object
		 * 		
		 * @param doc			The main document
		 * @param owningView	The view where this menu will be displayed
		 * @param outputView	The view which will execute the command and show the output (not required to be one where menu is displayed)
		 * @param menu			The menu being filled in (usually a context menu)
		 * @param simple		If true only build the most command commands.  If false, build all commands.
		********************************************************************************************/
		public abstract void fillMenu(Document doc, AbstractView owningView, AbstractView outputView, Menu menu, boolean simple) ;
		
		protected void addItem(final AbstractView view, Menu menu, String text, final String command)
		{
			MenuItem item = new MenuItem (menu, SWT.PUSH);
			item.setText (text) ;
			
			item.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) { view.executeAgentCommand(command, true) ; } } ) ;
		}

		// Default version where text == command
		protected void addItem(final AbstractView view, Menu menu, String command)
		{
			addItem(view, menu, command, command) ;
		}
		
		protected void addWindowSubMenu(AbstractView view, Menu menu)
		{
			view.fillWindowMenu(menu, true, true) ;
		}	
	}
	
	public static class SelectedProduction extends SelectedObject
	{
		private String m_Name ;
		
		public SelectedProduction(String name) { m_Name = name ; }
		
		public String toString() { return "Production " + m_Name ; } 
		
		/** Fills in menu items that are appropriate for this type of object */
		public void fillMenu(Document doc, AbstractView owningView, AbstractView outputView, Menu menu, boolean simple)
		{
			addItem(outputView, menu, doc.getSoarCommands().getPrintCommand(m_Name)) ;
			addItem(outputView, menu, doc.getSoarCommands().getEditCommand(m_Name)) ;
			addItem(outputView, menu, doc.getSoarCommands().getMatchesCommand(m_Name)) ;
			addItem(outputView, menu, doc.getSoarCommands().getMatchesWmesCommand(m_Name)) ;
			addItem(outputView, menu, doc.getSoarCommands().getExciseCommand(m_Name)) ;
			
			addWindowSubMenu(owningView, menu) ;
		}
	}
	
	public static class SelectedID extends SelectedObject
	{
		private String m_Name ;
		
		public SelectedID(String id)
		{
			m_Name = id ;
		}
		
		public String toString() { return "ID " + m_Name ; }
		
		/** Fills in menu items that are appropriate for this type of object */
		public void fillMenu(Document doc, AbstractView owningView, AbstractView outputView, Menu menu, boolean simple)
		{
			addItem(outputView, menu, doc.getSoarCommands().getPrintCommand(m_Name)) ;
			addItem(outputView, menu, doc.getSoarCommands().getPrintDepthCommand(m_Name, 2)) ;
			addItem(outputView, menu, doc.getSoarCommands().getPrintInternalCommand(m_Name)) ;
			addItem(outputView, menu, doc.getSoarCommands().getPrintCommand("(* ^* " + m_Name + ")")) ;
			
			addWindowSubMenu(owningView, menu) ;
		}
	}
	
	public static class SelectedWme extends SelectedObject
	{
		// Some may be null
		private String m_ID ;
		private String m_Att ;
		private String m_Value ;
		
		public SelectedWme(String id, String att, String value)
		{
			m_ID = id ;
			m_Att = att ;
			m_Value = value ;
		}
		
		/** Fills in menu items that are appropriate for this type of object */
		public void fillMenu(Document doc, AbstractView owningView, AbstractView outputView, Menu menu, boolean simple)
		{
			addItem(outputView, menu, doc.getSoarCommands().getPreferencesCommand(m_ID + " " + m_Att)) ;
			addItem(outputView, menu, doc.getSoarCommands().getPreferencesNameCommand(m_ID + " " + m_Att)) ;
			addItem(outputView, menu, doc.getSoarCommands().getPrintCommand(m_ID)) ;
			addItem(outputView, menu, doc.getSoarCommands().getPrintCommand(m_Value)) ;
			addItem(outputView, menu, doc.getSoarCommands().getPrintCommand("(* " + m_Att + " " + m_Value + " )")) ;
			addItem(outputView, menu, doc.getSoarCommands().getPrintCommand("(* " + m_Att + " *)")) ;
			
			addWindowSubMenu(owningView, menu) ;
		}
		
		public String toString() { return "WME " + m_ID + " " + m_Att + " " + m_Value ; }
	}
	
	public static class SelectedUnknown extends SelectedObject
	{
		private String m_Name ;
		
		public SelectedUnknown(String token)
		{
			m_Name = token ;
		}
		
		/** Fills in menu items that are appropriate for this type of object */
		public void fillMenu(Document doc, AbstractView owningView, AbstractView outputView, Menu menu, boolean simple)
		{
			owningView.fillWindowMenu(menu, false, true) ;
		}

		public String toString() { return "Unknown " + m_Name ; }
	}
	
	// Soar functions on triplets (ID ^att value) so we often need to parse
	// the tokens before and after the current selection position
	protected final static int kPrevToken = 0 ;
	protected final static int kCurrToken = 1 ;
	protected final static int kNextToken = 2 ;
	protected String[] m_Tokens = new String[3] ;
	protected final char[] kWhiteSpaceChars = new char[] { ' ', '\n', '\r', ')', '(', '{', '}' } ;

	// The raw values
	protected String 	m_FullText ;
	protected int	   	m_SelectionStart ;
	
	public ParseSelectedText(String content, int selectionStart)
	{
		m_FullText 		 = content ;
		m_SelectionStart = selectionStart ;
		
		if (m_FullText != null && m_FullText.length() > 0)
			parseTokens() ;
	}
			
	public String toString()
	{
		StringBuffer buffer = new StringBuffer() ;
		for (int i = 0 ; i < 3 ; i++)
		{
			buffer.append("Token " + i + " is |" + m_Tokens[i] + "| ") ;
		}
		
		return buffer.toString() ;
	}

	/**
	 * Finds the first char from the set of chars to occur in string starting from startPos.
	 * SLOWSLOW: This implementation is based on indexOf so it handles end cases in exactly the
	 * same way as the original but it's slow.  If we ever plan to call this repeatedly it should
	 * be rewritten.
	 */
	protected int indexOfSet(String string, char[] chars, int startPos)
	{
		int min = -1 ;
		for (int i = 0 ; i < chars.length ; i++)
		{
			int index = string.indexOf(chars[i], startPos) ;
			if (index != -1 && (min == -1 || index < min))
				min = index ;
		}
		
		return min ;
	}
	
	protected int lastIndexOfSet(String string, char[] chars, int startPos)
	{
		int max = -1 ;
		for (int i = 0 ; i < chars.length ; i++)
		{
			int index = string.lastIndexOf(chars[i], startPos) ;
			if (index > max)
				max = index ;
		}
		
		return max ;
	}
	
	protected boolean isProductionNameQuickTest(String token)
	{
		if (token == null || token.length() == 0)
			return false ;
		
		// Productions almost always start with a lower case character
		if (token.charAt(0) < 'a' || token.charAt(0) > 'z')
			return false ;
		
		// Production names almost always include multiple *'s
		if (token.indexOf('*') == -1)
			return false ;
		
		return true ;
	}
	
	protected boolean isProductionNameAskKernel(Document doc, Agent agent, String token)
	{
		if (token == null || token.length() == 0)
			return false ;

		return doc.isProductionLoaded(agent, token) ;
	}
	
	protected boolean isAttribute(String token)
	{
		if (token == null || token.length() == 0)
			return false ;
		
		// Attributes start with "^"
		return token.startsWith("^") ;
	}
	
	protected boolean isID(String token)
	{
		if (token == null || token.length() == 0)
			return false ;

		// ID's start with capital letters
		if (token.charAt(0) < 'A' || token.charAt(0) > 'Z')
			return false ;
		
		// ID's end with numbers
		if (token.charAt(token.length()-1) < '0' || token.charAt(token.length()-1) > '9')
			return false ;
		
		return true ;
	}
	
	/********************************************************************************************
	 * 
	 * Returns an object representing the parsed text.  This object has some Soar structure
	 * and the process of deciding which Soar object was clicked on is heuristic in nature.
	 * We make an educated guess based on detailed knowledge of how the output trace is displayed.
	 * Hopefully, this is one of only a few places where this will happen in the debugger with
	 * the new XML representations.
	 * 
	**********************************************************************************************/
	public SelectedObject getParsedObject(Document doc, Agent agent)
	{
		String curr = m_Tokens[kCurrToken] ;

		if (isProductionNameQuickTest(curr))
			return new SelectedProduction(curr) ;
		
		if (isAttribute(curr))
		{
			// Search backwards looking for the ID for this attribute.
			// We do this by looking for (X .... ^att name) or
			// (nnnn: X ... ^att name)
			int startPos = m_SelectionStart ;
			int openParens = m_FullText.lastIndexOf('(', startPos) ;
			int colon      = m_FullText.lastIndexOf(':', startPos) ;
			
			// Move to the start of the ID
			if (openParens != -1) openParens++ ;
			if (colon != -1) colon += 2 ;
			
			int idPos = Math.max(openParens, colon) ;
			
			int endSpace = m_FullText.indexOf(' ', idPos) ;

			if (idPos != -1 && endSpace != -1)
			{
				String id = m_FullText.substring(idPos, endSpace) ;
				
				if (isID(id))
					return new SelectedWme(id, curr, m_Tokens[kNextToken]) ;
			}
			
			// Couldn't find an ID to connect to this wme.
			return new SelectedWme(null, curr, m_Tokens[kNextToken]) ;
		}
		
		if (isID(curr))
		{
			return new SelectedID(curr) ;
		}
		
		// As a final test check the string against the real list of production names in the kernel
		// We do this last so that most RHS clicks this doesn't come up.
		if (isProductionNameAskKernel(doc, agent, curr))
			return new SelectedProduction(curr) ;
		
		return new SelectedUnknown(curr) ;
	}
	
	protected boolean isWhiteSpace(char ch)
	{
		for (int i = 0 ; i < kWhiteSpaceChars.length ; i++)
			if (kWhiteSpaceChars[i] == ch)
				return true ;
			
		return false ;
	}
	
	/** Extract prev, current and next tokens */
	protected void parseTokens()
	{
		int len = m_FullText.length() ;
		
		// Start by skipping over any white space to get to real content
		while (m_SelectionStart < len && isWhiteSpace(m_FullText.charAt(m_SelectionStart)))
			m_SelectionStart++ ;
		
		if (m_SelectionStart == len)
			return ;
		
		int startPos = m_SelectionStart ;
		
		// Move back to the space at the start of the current token
		int back1 = lastIndexOfSet(m_FullText, kWhiteSpaceChars, startPos) ;
		
		// Now move back to the space at the start of the previous token
		int back2 = lastIndexOfSet(m_FullText, kWhiteSpaceChars, back1 - 1) ;

		// Move to the space at the end of the current token
		int fore1 = indexOfSet(m_FullText, kWhiteSpaceChars, startPos) ;

		// Move to the space at the end of the next token
		int fore2 = indexOfSet(m_FullText, kWhiteSpaceChars, fore1 + 1) ;

		// Handle the end correctly
		if (fore1 == -1)
		{
			fore1 = m_FullText.length() ;
		}
		if (fore2 == -1)
		{
			fore2 = m_FullText.length() ;
		}

		// Extract the three tokens
		if (back1 != -1)
			m_Tokens[kPrevToken] = m_FullText.substring(back2+1, back1) ;
		if (fore2 != -1 && fore1 < fore2)
			m_Tokens[kNextToken] = m_FullText.substring(fore1+1, fore2) ;
		if (fore1 != -1)
			m_Tokens[kCurrToken] = m_FullText.substring(back1+1, fore1) ;
		
		//System.out.println(toString()) ;
	}
}


