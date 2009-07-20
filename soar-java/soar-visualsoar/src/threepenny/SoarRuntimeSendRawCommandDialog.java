package threepenny;
 
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.border.*;

import edu.umich.visualsoar.MainFrame;

import sml.Agent;
import sml.Kernel;

// 3P
// This dialog is used to retrieve a "raw command" (text string) from
// the user which is sent to the runtime via the STI.
public class SoarRuntimeSendRawCommandDialog extends JDialog {

	/**
	 * panels which contain the find input field and the buttons
	 */
	RawCommandPanel 			rawCommandPanel;
	SendRawCommandButtonPanel 	buttonPanel;
	Kernel						m_Kernel ;
	Agent						m_Agent ;

	public SoarRuntimeSendRawCommandDialog(final Frame owner, Kernel kernel, Agent agent) {
		super(owner, "Send Raw Command", false);

		// Store the kernel and agent
		m_Kernel = kernel ;
		m_Agent	 = agent ;
		
		rawCommandPanel = new RawCommandPanel();
		buttonPanel = new SendRawCommandButtonPanel();
		setResizable(false);
		Container contentPane = getContentPane();
		GridBagLayout gridbag = new GridBagLayout();
		GridBagConstraints c = new GridBagConstraints();
		contentPane.setLayout(gridbag);
		
		// specifies component as last one on the row
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.HORIZONTAL;
		
		contentPane.add(rawCommandPanel, c);
		contentPane.add(buttonPanel, c);
		pack();
		getRootPane().setDefaultButton(buttonPanel.sendButton);
		
		addWindowListener(new WindowAdapter() {
			public void windowOpened(WindowEvent we) {
				setLocationRelativeTo(owner);
				rawCommandPanel.requestFocus();
			}
		});
		
		buttonPanel.cancelButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				dispose();
			}
		});
				
		buttonPanel.sendButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				// Execute the command and echo the results to the debugger (if its connected)
				if (m_Agent != null)
				{
					String result = m_Agent.ExecuteCommandLine(rawCommandPanel.getFieldText(), true) ;
					MainFrame.getMainFrame().reportResult(result) ;
				}
				
				dispose();
			}
		});
	}	

	// Panel that contains the text entry for the raw command
	class RawCommandPanel extends JPanel
	{
		JTextField 			rawCommandField = new JTextField(20);

		public RawCommandPanel()
		{
			
			setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
			add(rawCommandField);

			setBorder(new CompoundBorder(
				BorderFactory.createTitledBorder("Command"),
				BorderFactory.createEmptyBorder(10,10,10,10)));
				
			// So that enter can affirmatively dismiss the dialog	
			rawCommandField.getKeymap().removeKeyStrokeBinding(KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, 0));
		}	
		
		public void requestFocus() {
			rawCommandField.requestFocus(); 
		}
		
		// Returns the text in the field
		public String getFieldText()
		{
			return rawCommandField.getText();
		}
	}
	
	// Panel containing the "Send" and "Cancel" buttons
	class SendRawCommandButtonPanel extends JPanel
	{
		JButton 	cancelButton = new JButton("Cancel");
		JButton 	sendButton = new JButton("Send");

		/**
		 * The 'find in project' version
		 */
		public SendRawCommandButtonPanel()
		{
			cancelButton.setMnemonic('c');
			sendButton.setMnemonic('s');
		
			setLayout(new FlowLayout());
			add(sendButton);
			add(cancelButton);
		}
	}
}
