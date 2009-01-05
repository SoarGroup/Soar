/********************************************************************************************
*
* ChooseFontDialog.java
* 
* Created on 	Nov 9, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package general;

import javax.swing.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;

/********************************************************************************************
* 
* Dialog class for picking a font.
* 
* This dialog (like most I write) is designed as a panel so it can be used in a dialog or
* placed on another window if desired.
* 
********************************************************************************************/
public class ChooseFontDialog extends JPanel
{
	/** The parent dialog (if there is one) */
	private JDialog m_ParentDialog = null ;

	private FontDisplay[] 	m_AllFontDisplays ;		// The list of all font names
	private FontDisplay[]	m_FixedFontDisplays ;	// The list of just those that are fixed width

	private boolean m_Inited = false ;				// Has this dialog been initialized
	private boolean	m_InitedFixedWidth = false ;	// The list of fixed width fonts is initialized later
	
	private JList 	m_FontList ;
	private JList	m_StyleList ;
	private JTextField m_SizeField ;
	private JTextArea  m_SampleField ;
	private JCheckBox  m_FixedWidthOnly ;
	
	private FontStyleDisplay[] m_FontStyles = new FontStyleDisplay[] {
		new FontStyleDisplay("Plain", Font.PLAIN),
		new FontStyleDisplay("Bold", Font.BOLD),
		new FontStyleDisplay("Italic", Font.ITALIC),
		new FontStyleDisplay("Bold & Italic", Font.BOLD | Font.ITALIC),
		 } ;
	
	private Font	m_Font ;
	
	/********************************************************************************************
	* 
	* This class is used to display a font name on screen and store whether it is fixed width or not.
	* 
	* The key function here is "toString()" which allows us to control how to display the font on screen.
	* 
	********************************************************************************************/
	private class FontDisplay
	{
		private Font m_Font ;
		private boolean m_IsFixedWidth ;
		
		public FontDisplay(Font font)
		{
			m_Font = font ;
		}
		
		/* Be careful -- this is only valid once we've called InitFixedWidthInfo */
		public boolean isFixedWidth() 				{ return m_IsFixedWidth ; }
		public void    setFixedWidth(boolean state) { m_IsFixedWidth = state ; }
		
		public String toString()
		{
			return m_Font.getFontName() ;
		}
	}
	
	/********************************************************************************************
	* 
	* This class is used to display a font style on screen (e.g. bold).
	* It allows us to associate the name of the style with its numeric equivalent.
	* 
	********************************************************************************************/
	private class FontStyleDisplay
	{
		private int m_Style ;
		private String m_Name ;
		
		public FontStyleDisplay(String name, int style)
		{
			m_Name = name ;
			m_Style = style ;
		}
		
		public String toString()
		{
			return m_Name ;
		}
	}

	/**
	 * Called when this window is added to a parent
	 */
	public void addNotify()
	{
		super.addNotify();
		
		Init() ;
	}
	
	/********************************************************************************************
	* 
	* Decide if a font is fixed width.
	* 
	* There seems to be no clean way to do this in Java and also it seems this will be expensive
	* as the component is involved.
	* 
	* @param	font	The font
	*
	* @return	True if this font is fixed width (i.e. all chars occupy same horizontal space)
	* 
	********************************************************************************************/
	private boolean isFixedWidthFont(Font font)
	{
		// The default fonts from "getAllFonts()" are size 1 so I'm not confident
		// about the char width info we'll get from them, so let's derive a decent
		// sized font before checking the character widths.
		FontMetrics fm = this.getFontMetrics(font.deriveFont(24.0f)) ;
		
		return fm.charWidth('i') == fm.charWidth('w') ;
	}
	
	/********************************************************************************************
	* 
	* This function examines the list of fonts and decides which ones are fixed width.
	* 
	* This is a slow process in Java as we need to construct each font in turn and test whether
	* two characters in the font have the same width.
	* 
	* Because it's slow, we wait to do this intialization until the user asks for it.
	* 
	********************************************************************************************/
	private void InitFixedWidthInfo()
	{
		if (m_InitedFixedWidth)
			return ;
		
		m_InitedFixedWidth = true ;
		
		// Initialize the fixed width font information
		int fixedCount = 0 ;
		for (int i = 0 ; i < m_AllFontDisplays.length ; i++)
		{
			m_AllFontDisplays[i].setFixedWidth(isFixedWidthFont(m_AllFontDisplays[i].m_Font)) ;
			
			if (m_AllFontDisplays[i].isFixedWidth())
				fixedCount++ ;
		}
		
		// Fill in the list of all fixed width fonts in the system
		int j = 0 ;
		m_FixedFontDisplays = new FontDisplay[fixedCount] ;
		for (int i = 0 ; i < m_AllFontDisplays.length ; i++)
		{
			if (m_AllFontDisplays[i].isFixedWidth())
				m_FixedFontDisplays[j++] = m_AllFontDisplays[i] ;
		}		
	}
	
	/********************************************************************************************
	* 
	* Initialize all of the windows and controls in the dialog.
	* 
	********************************************************************************************/
	private void Init()
	{
		if (m_Inited)
			return ;
		m_Inited = true ;
		
		GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
		Font[] allFonts = ge.getAllFonts();
		
		m_AllFontDisplays = new FontDisplay[allFonts.length] ;
		
		// Fill in the list of all fonts in the system
		for (int i = 0 ; i < allFonts.length ; i++)
		{
			FontDisplay d = new FontDisplay(allFonts[i]) ;
			
			m_AllFontDisplays[i] = d ;
		}

		// We always start with fixed width not selected and only create
		// the list of fixed list fonts if the user selects the check box as this
		// process is slow.		  
  		m_FixedWidthOnly = new JCheckBox("Fixed width only") ;
		m_FixedWidthOnly.setSelected(false) ;

  		m_FontList  = new JList(m_AllFontDisplays) ;
  		m_StyleList = new JList(m_FontStyles) ;
  		m_SizeField = new JTextField(5) ;

		m_FontList.addListSelectionListener(new ListSelectionListener() { public void valueChanged(ListSelectionEvent e)  { selectionChanged() ; } } ) ;
		m_StyleList.addListSelectionListener(new ListSelectionListener() { public void valueChanged(ListSelectionEvent e) { selectionChanged() ; } } ) ;
		m_SizeField.getDocument().addDocumentListener(new TextFieldListener() { public void textUpdate(DocumentEvent e)   { selectionChanged() ; } } );
		m_FixedWidthOnly.addActionListener(new ActionListener() { public void actionPerformed(ActionEvent e) { clickedFixedWidth() ; } } ) ;

		JScrollPane scrollFont = new JScrollPane(m_FontList) ;
		JScrollPane scrollStyle = new JScrollPane(m_StyleList) ;

		JLabel sampleLabel = new JLabel("Sample") ;
		JTextArea topPad = new JTextArea("") ;
		m_SampleField = new JTextArea("  Sample AaBbYyZz  ", 1, 1) ;
		JTextArea bottomPad = new JTextArea("") ;
		JPanel fieldPanel = new JPanel() ;
		fieldPanel.setLayout(new GridLayout(0,1,0,0)) ;
		fieldPanel.add(topPad) ;
		fieldPanel.add(m_SampleField) ;
		fieldPanel.add(bottomPad) ;
		
		JPanel samplePanel = new JPanel() ;
		//samplePanel.setLayout(new BorderLayout()) ;		
		samplePanel.add(sampleLabel, BorderLayout.NORTH) ;
		samplePanel.add(fieldPanel, BorderLayout.SOUTH) ;

		// The list of font names
		JLabel fontLabel = new JLabel("Font") ;
		JPanel fontPanel = new JPanel() ;
		fontPanel.setLayout(new BorderLayout()) ;
		fontPanel.add(fontLabel, BorderLayout.NORTH) ;
		fontPanel.add(scrollFont, BorderLayout.CENTER) ;

		// The list of styles
		JLabel styleLabel = new JLabel("Style") ;
		JPanel stylePanel = new JPanel() ;
		stylePanel.setLayout(new BorderLayout()) ;
		stylePanel.add(styleLabel, BorderLayout.NORTH) ;
		stylePanel.add(scrollStyle, BorderLayout.CENTER) ;

		// The size
		JLabel sizeLabel = new JLabel("Size") ;
		JPanel sizePanel = new JPanel() ;
		sizePanel.setLayout(new BorderLayout()) ;
		sizePanel.add(sizeLabel, BorderLayout.NORTH) ;
		sizePanel.add(m_SizeField, BorderLayout.CENTER) ;
  
		JPanel topPanel = new JPanel() ;
		topPanel.add(fontPanel) ;
		topPanel.add(stylePanel) ;
		topPanel.add(sizePanel) ;
		
		JPanel checkedTopPanel = new JPanel() ;
		checkedTopPanel.setLayout(new BorderLayout()) ;
		checkedTopPanel.add(topPanel, BorderLayout.CENTER) ;
		checkedTopPanel.add(this.m_FixedWidthOnly, BorderLayout.SOUTH) ;
		
		JPanel middlePanel = new JPanel() ;
		middlePanel.setLayout(new BorderLayout()) ;
		middlePanel.add(checkedTopPanel, BorderLayout.CENTER) ;
		middlePanel.add(samplePanel, BorderLayout.SOUTH) ;
		
		JPanel buttonPanel = new JPanel() ;

		JButton ok = new JButton("OK") ;
		JButton cancel = new JButton("Cancel") ;

		ok.addActionListener(new ActionListener()     { public void actionPerformed(ActionEvent e) { okPressed() ; } } ) ;
		cancel.addActionListener(new ActionListener() { public void actionPerformed(ActionEvent e) { cancelPressed() ; } } ) ;
				
		buttonPanel.setLayout(new GridLayout(1, 0, 10, 10)) ;
		buttonPanel.add(ok) ;
		buttonPanel.add(cancel) ;

		if (this.m_ParentDialog != null)
		{
			this.m_ParentDialog.getRootPane().setDefaultButton(ok) ;
		}
		
		setSampleFont() ;
		
		this.setLayout(new BorderLayout()) ;
		this.add(middlePanel, BorderLayout.CENTER) ;
		this.add(buttonPanel, BorderLayout.SOUTH) ;
		
		// We have to make the panel visible before we select the font name in the list
		// or we can't make sure the font name is visible.
		this.setVisible(true) ;
		//this.show(true) ;
		
		setSelectionFromFont(m_Font) ;
	}
	
	/********************************************************************************************
	* 
	* The user has clicked on the fixed width font check box (selecting or unselecting it).
	* 
	********************************************************************************************/
	private void clickedFixedWidth()
	{		
		if (m_FixedWidthOnly.isSelected())
		{
			this.InitFixedWidthInfo() ;
			m_FontList.setListData(this.m_FixedFontDisplays) ;
			setSelectionFromFont(m_Font) ;
		}
		else
		{
			m_FontList.setListData(this.m_AllFontDisplays) ;
			setSelectionFromFont(m_Font) ;
		}
	}
	
	/********************************************************************************************
	* 
	* Change the font used to display the sample text in the dialog.
	* 
	********************************************************************************************/
	private void setSampleFont()
	{
		this.m_SampleField.setFont(m_Font) ;
	}
	
	/********************************************************************************************
	* 
	* Select the appropriate values in the font name, style and size fields to reflect the font
	* passed in.
	* 
	********************************************************************************************/
	private void setSelectionFromFont(Font font)
	{
		m_SizeField.setText(Integer.toString(font.getSize())) ;
		
		int styleSelect = -1 ;
		int n = m_StyleList.getModel().getSize() ;
		for (int i = 0 ; i < n ; i++)
		{
			FontStyleDisplay style = (FontStyleDisplay)m_StyleList.getModel().getElementAt(i) ;
			if (style.m_Style == font.getStyle())
				styleSelect = i ;
		}
		
		int fontSelect = -1 ;
		n = m_FontList.getModel().getSize() ;

		for (int i = 0 ; i < n ; i++)
		{
			FontDisplay fd = (FontDisplay)m_FontList.getModel().getElementAt(i) ;
			if (fd.m_Font.getFontName().equalsIgnoreCase(font.getFontName()))
				fontSelect = i ;
		}
		
		m_StyleList.setSelectedIndex(styleSelect) ;
		m_FontList.setSelectedIndex(fontSelect) ;
		
		if (fontSelect != -1)
			m_FontList.ensureIndexIsVisible(fontSelect) ;
	}
	
	/********************************************************************************************
	* 
	* One of the lists or fields used to describe the font has changed.
	* 
	********************************************************************************************/
	private void selectionChanged()
	{
		FontDisplay fontDisplay = (FontDisplay)m_FontList.getSelectedValue() ;
		FontStyleDisplay styleDisplay = (FontStyleDisplay)m_StyleList.getSelectedValue() ;

		if (fontDisplay == null || styleDisplay == null)
			return ;

		Font font ;

		try
		{
			int size = Integer.parseInt(m_SizeField.getText()) ;

			// Create a font from the current selections.
			font = fontDisplay.m_Font.deriveFont(styleDisplay.m_Style, size) ;
		}
		catch (Exception e)
		{
			// The current entry in the size field is not an integer, so ignore it or
			// for some reason we couldn't create the font, so again ignore the inputs.
			return ;
		}
		
		// Change the font
		m_Font = font ;
		setSampleFont() ;
	}
	
	/********************************************************************************************
	* 
	* The user pressed OK.
	* 
	********************************************************************************************/
	private void okPressed()
	{
		EndDialog() ;
	}
	
	/********************************************************************************************
	* 
	* The user pressed Cancel.
	* 
	********************************************************************************************/
	private void cancelPressed()
	{
		m_Font = null ;
		EndDialog() ;
	}
	
	/********************************************************************************************
	* 
	* Close the dialog window.
	* 
	********************************************************************************************/
	private void EndDialog()
	{
		JDialog dialog = this.m_ParentDialog ;
		if (dialog == null)
			return ;
		
		// Close the dialog
		dialog.setVisible(false) ;
		dialog.dispose() ;
	}

	/**************************************************************************
	 * 
	 * Creates the panel inside a dialog box, shows the dialog (modally)
	 * and returns the results of the user's selection.
	 * 
	 * @param	frame		The frame which that will own this dialog
	 * @param	title		The title of the dialog window
	 * @param	initialFont	The font to select initially (can't be null)
	 * 
	 * @return Font		The font the user chose (or null if cancelled)
	 *************************************************************************/
	public static Font ShowDialog(Frame frame, String title, Font initialFont)
	{		
		if (initialFont == null)
			throw new IllegalArgumentException("Must pass in an initial font") ;
	
		// Create a new, modal dialog
		boolean modal = true ;
		JDialog dialog = new JDialog(frame, title, modal) ;
		
		// Set the size of the window and center it
		int width = 400 ;
		int height = 400 ;
		dialog.setSize(width, height) ;
		dialog.setLocation(frame.getWidth() / 2 - width / 2, frame.getHeight() / 2 - height / 2) ;
		
		dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE) ;
		
		// Build the content for the dialog
		ChooseFontDialog panel = new ChooseFontDialog() ;
		panel.m_Font		 = initialFont ;
		panel.m_ParentDialog = dialog ;
		
		// Make the view the contents of the dialog box.
		dialog.getContentPane().add(panel) ;
		
		// Show everything -- the dialog becomes modal at this point and
		// the flow of control stops until the window is closed.
		dialog.setVisible(true) ;

		// Return the user's choice
		return panel.m_Font ;
	}
}
