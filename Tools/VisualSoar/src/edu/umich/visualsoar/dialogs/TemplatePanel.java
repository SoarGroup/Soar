package edu.umich.visualsoar.dialogs;

import java.awt.*;
import java.io.File;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.border.*;
import edu.umich.visualsoar.misc.Preferences;
          
/**
 * Panel that contains path panel for the template folder
 * @author Jon Bauman
 */
class TemplatePanel extends JPanel {
	
	File 			currFolder = Preferences.getInstance().getTemplateFolder();
	JFileChooser	chooser = new JFileChooser(currFolder);
	JTextField		pathField = new JTextField(currFolder.toString(), 20);
	JLabel			mustReopen = new JLabel(
					"Rule editors must be reopened to realize changes");
	JButton 		browse = new JButton("Browse...");

	public TemplatePanel() {

		setLayout(new GridLayout(3, 1, 0, 5));

		setBorder(new CompoundBorder(
			BorderFactory.createTitledBorder("Template Folder"),
			BorderFactory.createEmptyBorder(10,10,10,10)));

		add(pathField);
		add(browse);
		add(mustReopen);

		browse.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
				int state = chooser.showDialog(null, "Select Template Folder");
				File file = chooser.getSelectedFile();
				
				if (file != null && state == JFileChooser.APPROVE_OPTION) {
					pathField.setText(currFolder.toString());
				}
			}
		});
			
	}
	
	public File getTemplateFolder() {
		currFolder = new File(pathField.getText());
		return currFolder;
	}
	
	public void requestFocus() {
		pathField.selectAll();
		pathField.requestFocus(); 
	}

}
