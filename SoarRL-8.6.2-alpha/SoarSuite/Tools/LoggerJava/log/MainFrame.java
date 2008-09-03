/********************************************************************************************
*
* MainFrame.java
* 
* Description:	Example of how to build a simple logging application for SML.
* 				We'll do this one in Swing since we have lots of SWT examples elsewhere in SML.
* 
* 				The MainFrame class will be in charge of the UI.
* 				The Logger class will handle the SML stuff and actually show how to log.
* 
* 				A user can hopefully take this application and modify the Logger class to output
* 				the information they're interested in for their specific usage of Soar.
* 
* Created on 	Feb 8, 2006
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package log;

import java.awt.GridLayout;

import javax.swing.*;

public class MainFrame extends javax.swing.JFrame
{
	// This class handles the SML interaction
	Logger	m_Logger = new Logger() ;
	String	m_Filename = "log.txt" ;
	
	// The UI
	JButton jStartLogging ;
	JButton jStopLogging ;
	JButton jAppendLogging ;
	JButton jSetLogFilename ;
	
	public void initComponents() throws Exception
	{
		jStartLogging = new JButton() ;
		jStartLogging.setText("Start Logging") ;

		jStopLogging = new JButton() ;
		jStopLogging.setText("Stop Logging") ;

		jAppendLogging = new JButton() ;
		jAppendLogging.setText("Append Logging") ;

		jSetLogFilename = new JButton() ;
		jSetLogFilename.setText("Set Log Filename") ;

		jStartLogging.addActionListener(new java.awt.event.ActionListener() {
			public void actionPerformed(java.awt.event.ActionEvent e) {
				String errorMsg = m_Logger.startLogging(m_Filename, false) ;
				
				if (errorMsg != null)
					JOptionPane.showMessageDialog(MainFrame.this, errorMsg) ;
			
				enableButtons() ;
			}
		});

		jAppendLogging.addActionListener(new java.awt.event.ActionListener() {
			public void actionPerformed(java.awt.event.ActionEvent e) {
				String errorMsg = m_Logger.startLogging(m_Filename, true) ;
				
				if (errorMsg != null)
					JOptionPane.showMessageDialog(MainFrame.this, errorMsg) ;
			
				enableButtons() ;
			}
		});

		jStopLogging.addActionListener(new java.awt.event.ActionListener() {
			public void actionPerformed(java.awt.event.ActionEvent e) {
				m_Logger.stopLogging() ;			
				enableButtons() ;
			}
		});

		jSetLogFilename.addActionListener(new java.awt.event.ActionListener() {
			public void actionPerformed(java.awt.event.ActionEvent e) {
				String filename = JOptionPane.showInputDialog(MainFrame.this, "Log filename", m_Filename) ;
				if (filename != null && filename.length() > 0)
					m_Filename = filename ;
			}
		});

		JPanel main = new JPanel() ;
		main.setLayout(new GridLayout(2, 0, 0, 0)) ;
		main.setVisible(true);
		main.add(jStartLogging) ;
		main.add(jStopLogging) ;
		main.add(jAppendLogging) ;
		main.add(jSetLogFilename) ;

		enableButtons() ;
		
		this.setTitle("Sample SML Logger") ;
		setSize(new java.awt.Dimension(300, 150));
		getContentPane().add(main);
		
		addWindowListener(new java.awt.event.WindowAdapter() {
			public void windowOpened(java.awt.event.WindowEvent e) {
				thisWindowOpened();
			}
			public void windowClosing(java.awt.event.WindowEvent e) {
				thisWindowClosing();
			}
		});
	}

	public void enableButtons()
	{
		jStartLogging.setEnabled(!m_Logger.isLogging()) ;
		jStopLogging.setEnabled(m_Logger.isLogging()) ;
		jAppendLogging.setEnabled(!m_Logger.isLogging()) ;
		jSetLogFilename.setEnabled(!m_Logger.isLogging()) ;
	}
	
	public void thisWindowOpened() { }
	
	public void thisWindowClosing()
	{
		setVisible(false);
		dispose();
		m_Logger.stopLogging() ;
		System.exit(0) ;
	}
	
	/********************************************************************************************
	 * 
	 * 
	 * 
	 * @param args
	 ********************************************************************************************/
	public static void main(String[] args)
	{
		try
		{
			MainFrame frame = new MainFrame();
			frame.initComponents();
			frame.setVisible(true);
		}
		catch (Exception ex)
		{
			System.out.println(ex) ;
		}
	}
}
