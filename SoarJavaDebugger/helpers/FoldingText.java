/********************************************************************************************
*
* FoldingText.java
* 
* Description:	
* 
* Created on 	May 1, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package helpers;

import java.util.ArrayList;

import org.eclipse.swt.widgets.*;
import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.layout.*;

/************************************************************************
 * 
 * A widget that consists of a scrolling text window and a small bar with
 * icons for 'folding' the text (i.e. expanding and contracting) sections
 * of the text.
 * 
 * This is very similar to a view offered by Eclipse, but only depends on SWT
 * and may be higher performance if we get it right.
 * 
 ************************************************************************/
public class FoldingText
{
	protected Text		m_Text ;
	protected Canvas 	m_IconBar ;
	protected Composite m_Container ;
	protected FoldingTextDoc m_FoldingDoc = new FoldingTextDoc(this) ;

	public static class FoldingTextDoc
	{
		protected	ArrayList	m_TextBlocks = new ArrayList() ;
		protected	int			m_ShowFilter ;
		protected	int			m_HideFilter ;
		
		protected 	FoldingText	m_FoldingText ;
		protected	Block		m_LastBlock ;
		
		public FoldingTextDoc(FoldingText text)
		{
			m_FoldingText = text ;
		}
		
		public void addBlock(Block block)
		{
			block.setIndex(m_TextBlocks.size()) ;
			m_TextBlocks.add(block) ;
			m_LastBlock = block ;
		}
		
		/** Returns the block (if any) which starts at the given line number. */
		public Block getBlockStartsAtLineNumber(int lineNumber)
		{
			int size = m_TextBlocks.size() ;
			for (int b = 0 ; b < size ; b++)
			{
				// NOTE: Depending on how often this is called we could hash these values
				Block block = (Block)m_TextBlocks.get(b) ;
				if (block.getStart() == lineNumber)
					return block ;
				
				// As soon as we reach a block after the target line we know we have
				// no match
				if (block.getStart() > lineNumber)
					return null ;
			}
			
			return null ;
		}
		
		/** Returns the index of the first block which starts at lineNumber or greater.  -1 if none. */
		public int getBlockAfterLineNumber(int lineNumber)
		{
			// NOTE: Depending on how often this is called we could hash these values
			int size = m_TextBlocks.size() ;
			for (int b = 0 ; b < size ; b++)
			{
				Block block = (Block)m_TextBlocks.get(b) ;
				if (block.getStart() >= lineNumber)
					return b ;
			}
			
			return -1 ;
		}
		
		public Block getBlock(int index)
		{
			return (Block)m_TextBlocks.get(index) ;
		}
		
		public int getNumberBlocks()
		{
			return m_TextBlocks.size() ;
		}
		
		/** Returns the character positions for the start and end of a block -- so we can use these to set the selection to the block */
		public Point getBlockSelectionRange(Block block)
		{
			int start = 0 ;

			for (int b = 0 ; b < block.getIndex() ; b++)
			{
				int chars = ((Block)m_TextBlocks.get(b)).getVisibleCharCount() ;
				start += chars ;
			}
			
			int end = start + block.getVisibleCharCount() ;
			
			return new Point(start, end) ;
		}
		
		public void expandBlock(Block block, boolean state)
		{
			if (block.isExpanded() == state || !block.canExpand() || block.getSize() == 1)
				return ;

			Point range = getBlockSelectionRange(block) ;
			int delta = 0 ;
			
			//boolean selected = (m_FoldingText.m_Text.getSelectionCount() > 1) ;
			
			m_FoldingText.m_Text.setSelection(range) ;
			
			// For debugging show selection and then update it
			//if (!selected)
			//	return ;
			
			if (state)
			{
				// Expanding
				m_FoldingText.m_Text.insert(block.getAll()) ;
				delta = block.getSize() - 1 ;
			}
			else
			{
				m_FoldingText.m_Text.insert(block.getFirst()) ;
				delta = 1 - block.getSize() ;
			}
			
			// Update the remaining block position info
			int size = getNumberBlocks() ;
			for (int b = block.getIndex()+1 ; b < size ; b++)
			{
				Block update = getBlock(b) ;
				update.setStart(update.getStart() + delta) ;
			}
			
			block.setExpand(state) ;
		}
	}
	
	/** Represents a section of text that is a single unit for expanding/collapsing. */
	public static class Block
	{
		protected int		m_Index ;
		protected boolean	m_CanExpand ;
		protected boolean	m_IsExpanded ;
		protected int		m_Start ;				// The first line where this block appears in the text widget

		protected ArrayList m_Lines = new ArrayList() ;
		protected StringBuffer m_All = new StringBuffer() ;
		
		public Block(boolean canExpand) { m_CanExpand = canExpand ; }
		
		public void setIndex(int index)	{ m_Index = index ; }
		public int getIndex()			{ return m_Index ; }
		
		public void setStart(int start)			{ m_Start = start ; }
		public int  getStart() 					{ return m_Start ; }
		public void setCanExpand(boolean state)	{ m_CanExpand = state ; }
		public boolean canExpand()				{ return m_CanExpand && m_Lines.size() > 1 ; }
		public void setExpand(boolean state)	{ m_IsExpanded = state ; }
		public boolean isExpanded()				{ return m_IsExpanded ; }
		
		public int  getSize()  				{ return m_Lines.size() ; }
		public void appendLine(String text) { m_Lines.add(text) ; m_All.append(text) ; }
		
		public String getFirst() 			{ return (String)m_Lines.get(0) ; }
		public String getAll()				{ return m_All.toString() ; }
		
		public int getFirstLineCharCount()	{ return getFirst().length() + 1 ; }			// Add one because \n in text becomes \r\n in Text control (BUGBUG: Windows only?)
		public int getAllCharCount()		{ return m_All.length() + m_Lines.size() ; }	// Have to add one newline char per line to make this correct (as above)
		public int getVisibleCharCount()	{ return m_IsExpanded ? getAllCharCount() : getFirstLineCharCount() ; }
	}
	
	public FoldingText(Composite parent)
	{
		m_Container = new Composite(parent, 0) ;
		m_IconBar	= new Canvas(m_Container, 0) ;
		m_Text      = new Text(m_Container, SWT.MULTI | SWT.V_SCROLL | SWT.READ_ONLY) ;
		
		GridLayout layout = new GridLayout() ;
		layout.numColumns = 2 ;
		m_Container.setLayout(layout) ;
		
		GridData data1 = new GridData(GridData.FILL_VERTICAL) ;
		data1.widthHint = 15 ;
		m_IconBar.setLayoutData(data1) ;

		GridData data2 = new GridData(GridData.FILL_BOTH) ;
		m_Text.setLayoutData(data2) ;
		
		int lineCount = 0 ;
		for (int b = 0 ; b < 3 ; b++)
		{
			Block block = new Block(true) ;
			block.setStart(lineCount) ;
			
			for (int i = 0 ; i < 50 ; i++)
			{		
				lineCount++ ;
				String line = "This is line " + i + " in block " + b + "\n" ;
				block.appendLine(line) ;
			}
			m_Text.append(block.getAll()) ;
			block.setExpand(true) ;
			
			m_FoldingDoc.addBlock(block) ;
		}
		
		m_IconBar.addPaintListener(new PaintListener() { public void paintControl(PaintEvent e) { paintIcons(e) ; } } ) ;
		m_IconBar.setBackground(m_IconBar.getDisplay().getSystemColor(SWT.COLOR_WHITE)) ;

		m_IconBar.addMouseListener(new MouseAdapter() { public void mouseUp(MouseEvent e) { iconBarMouseClick(e) ; } } ) ;
		
		// Think we'll need this so we update the icons while we're scrolling
		m_Text.getVerticalBar().addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { m_IconBar.redraw() ; } }) ;
	}
	
	public void appendText(String text, boolean redraw)
	{
		
	}
	
	public void appendSubText(String text, boolean redraw)
	{
		
	}
	
	public Composite getWindow() 	 { return m_Container ; }
	public Text      getTextWindow() { return m_Text ; }
	
	protected void iconBarMouseClick(MouseEvent e)
	{
		// Make sure the text control is properly initialized
		if (m_Text.getLineHeight() == 0)
			return ;

		int topLine = m_Text.getTopIndex() ;
		int lineHeight = m_Text.getLineHeight() ;

		int line = (e.y / lineHeight) + topLine ;
		
		Block block = m_FoldingDoc.getBlockStartsAtLineNumber(line) ;
		
		if (block == null)
			return ;
		
		m_FoldingDoc.expandBlock(block, !block.isExpanded()) ;
		m_IconBar.redraw() ;
	}
	
	protected void paintIcons(PaintEvent e)
	{
		GC gc = e.gc;
		
		int scrollPosition = m_Text.getVerticalBar().getSelection() ;
		
		Canvas canvas = m_IconBar ;
		Rectangle client = canvas.getClientArea ();

		// Make sure the text control is properly initialized
		if (m_Text.getLineHeight() == 0)
			return ;

		// Get all the information about which part of the text window is visible
		int topLine = m_Text.getTopIndex() ;
		int lineHeight = m_Text.getLineHeight() ;
		int visibleLines = m_Text.getClientArea().height / lineHeight ;
		int lastLine = Math.min(m_Text.getLineCount(),m_Text.getTopIndex() + visibleLines) ;
		
		// Start with the first block that appears at or after "topLine"
		int blockIndex = m_FoldingDoc.getBlockAfterLineNumber(topLine) ;
		int blockCount = m_FoldingDoc.getNumberBlocks() ;

		// Go through each block in turn until we're off the bottom of the screen
		// or at the end of the list of blocks drawing icons
		while (blockIndex != -1 && blockIndex < blockCount)
		{
			Block block = m_FoldingDoc.getBlock(blockIndex) ;
			
			int line = block.getStart() ;
			
			// Once we drop off the bottom of the screen we're done
			if (line > lastLine)
				break ;
		
			int pos = line - topLine ;
			int y = pos * lineHeight ;
				
			boolean expanded = block.isExpanded() ;
			
			if (block.canExpand())
			{
				gc.setBackground(canvas.getDisplay().getSystemColor(expanded ? SWT.COLOR_YELLOW : SWT.COLOR_GREEN)) ;				
				gc.drawRectangle(0, y, client.width-1, lineHeight-1) ;
				
				int y1 = y + lineHeight/2 ;
				gc.drawLine(0, y1, client.width-1, y1) ;

				if (!expanded)
				{
					int x1 = (client.width-1) / 2 ;
					gc.drawLine(x1, y, x1, y + lineHeight-1) ;
				}				
			}
			blockIndex++ ;				
		}
	}
}
