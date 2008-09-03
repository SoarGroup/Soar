/********************************************************************************************
*
* Logger.java
* 
* Description:	Provides support for logging output from a window to a file.
* 
* Created on 	Feb 9, 2006
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package helpers;

import general.JavaElementXML;

import java.io.*;

import debugger.MainFrame;
import dialogs.LoggingDialog;
import doc.Document;

import manager.Pane;
import modules.AbstractView;

public class Logger
{
		// The file for handling output (perhaps we should use a PrintWriter?)
		protected FileWriter 	m_OutputFile ;
		
		// Are we currently logging?
		protected boolean	 	m_IsLogging = false ;
		
		// The name (and path) of the file to use
		protected String	 	m_LogFilename = "log.txt" ;
		
		// Path to folder to use for log file.  Store this separately because we can cache this between runs but the filenames
		// we want to generate to ensure they remain unique across multiple debugger frames and windows.
		protected String		m_LogFolder = "" ;
		
		// The view this logger is attached to
		protected AbstractView	m_View ;
		
		// When true, flush the file after each write so we can examine it during a run (otherwise it'll only update when the file closes)
		protected boolean		m_FlushEachWrite = true ;
		
		// When true, we should append to the log file.  This can be overridden by a direct call to startLogging().
		protected boolean		m_Append = false ;
		
		// Wait to call this until name has been assigned to the view
		public void setView(AbstractView view)
		{
			m_View = view ;
			
			// Create a filename that is unique to this window (view's name is only unique within this main frame so we need both parts).
			m_LogFilename = "log_" + view.getName() + "_" + view.getMainFrame().getName() + ".txt" ;
		}
		
		public boolean isLogging() 				{ return m_IsLogging ; }

		public String getFilePath()
		{
			if (m_LogFolder == null || m_LogFolder.length() == 0)
				return m_LogFilename ;
			
			File file = new File(m_LogFolder, m_LogFilename) ;
			return file.getPath() ;
		}
				
		public void setFilePath(String filename)
		{
			File fullPath = new File(filename) ;
			
			File folder = fullPath.getParentFile() ;
			
			m_LogFolder = (folder != null) ? folder.getPath() : "" ;
			m_LogFilename = fullPath.getName() ;
		}
		
		public boolean getAppend()				{ return m_Append ; }
		public void setAppend(boolean state) 	{ m_Append = state ; }
		
		public boolean getFlush()				{ return m_FlushEachWrite ; }
		public void setFlush(boolean state)		{ m_FlushEachWrite = state ; }
		
		public String startLogging(boolean append)
		{
			if (m_View == null)
				throw new IllegalStateException("Need to initialize view before starting to log") ;
			
			String file = getFilePath() ;
			try
			{
				m_OutputFile = new FileWriter(file, append) ;
				m_IsLogging = true ;
			}
			catch (Exception ex)
			{
				return "Error opening log file " + file + " " + ex ;
			}
			return null ;
		}
		
		/********************************************************************************************
		 * 
		 * Add this string to the log file (if we're currently logging).
		 * 
		 * @param text						The string to log
		 * @param replaceSoarNewLines		Some strings use Soar (\n) newlines.  If true we'll replace those with system newlines (e.g. \n\r on Windows)
		 * @param appendNewline				If true, append a system newline to the end of the text.
		********************************************************************************************/
		public void log(String text, boolean replaceSoarNewLines, boolean appendNewline)
		{
			if (!m_IsLogging)
				return ;
			
			try
			{
				if (replaceSoarNewLines)
					text = text.replaceAll(AbstractView.kLineSeparator, AbstractView.kSystemLineSeparator) ;

				m_OutputFile.write(text) ;
				
				if (appendNewline)
					m_OutputFile.write(AbstractView.kSystemLineSeparator) ;
				
				if (m_FlushEachWrite)
					m_OutputFile.flush() ;
			}
			catch (Exception ex)
			{
				System.out.println(ex) ;
			}
		}
		
		public void stopLogging()
		{
			try
			{
				if (m_OutputFile != null)
					m_OutputFile.close() ;
			}
			catch (Exception ex)
			{
				System.out.println(ex) ;
			}
			m_OutputFile = null ;
			m_IsLogging = false ;
		}
		
		public void showDialog()
		{
			LoggingDialog.showDialog(m_View.getMainFrame(), "Log this window", m_View) ;
		}
		
		public general.JavaElementXML convertToXML(String tagName)
		{
			JavaElementXML element = new JavaElementXML(tagName) ;
			
			element.addAttribute("Folder", m_LogFolder) ;
			element.addAttribute("Flush", Boolean.toString(m_FlushEachWrite)) ;
			element.addAttribute("Append", Boolean.toString(m_Append)) ;
			
			return element ;
		}
		
		public void loadFromXML(doc.Document doc, general.JavaElementXML element) throws Exception
		{
			// Check that we have this data -- this is just helpful when adding this feature as existing layout files won't have these fields.
			if (element.getAttribute("Folder") == null)
					return ;
			
			m_LogFolder = element.getAttributeThrows("Folder") ;
			m_FlushEachWrite = element.getAttributeBooleanThrows("Flush") ;
			m_Append = element.getAttributeBooleanThrows("Append") ;
		}

}
