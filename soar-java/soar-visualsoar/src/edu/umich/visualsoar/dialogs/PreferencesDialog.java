package edu.umich.visualsoar.dialogs;
 
import java.io.File;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import java.util.*;
import edu.umich.visualsoar.ruleeditor.SoarDocument;
import edu.umich.visualsoar.misc.*;
import edu.umich.visualsoar.MainFrame;
          
/**
 * Dialog that allows preferences to be edited
 * @author Jon Bauman
 * @author Brian Harleton
 */
public class PreferencesDialog extends JDialog {
         
	JTabbedPane			tabPane = new JTabbedPane(JTabbedPane.TOP);
	/**
	 * Panel which contians the syntax color editing buttons
	 */
	SyntaxColorsPanel 	colorPanel;
	
	/**
	 * Panel which contains the auto-tiling pref buttons
	 */
	TemplatePanel		templatePanel = new TemplatePanel();

	/**          
	 * Panel which contains the auto-tiling pref buttons
	 */
	AutoTilePanel		tilePanel = new AutoTilePanel();

	/**
	 * Panel which contains all the buttons
	 */
	DefaultButtonPanel 	buttonPanel = new DefaultButtonPanel();



	/**
	 * The Preferences singleton
	 */
	Preferences			prefs = Preferences.getInstance();

	/**
	 * The syntax colors
	 */
	SyntaxColor[]		colorTable = prefs.getSyntaxColors();
	
	boolean				approved = false;

	JCheckBox     autoIndentingCheckBox = new JCheckBox("Auto-Indenting",prefs.isAutoIndentingEnabled());
  JCheckBox     autoSoarCompleteCheckBox = new JCheckBox("Auto-Soar Complete", prefs.isAutoSoarCompleteEnabled());



	/**
	 * @param owner Frame which owns the dialog
	 */	  
	public PreferencesDialog(final Frame owner) {
		super(owner, "Preferences", true);


		Container 			contentPane = getContentPane();
		GridBagLayout 		gridbag = new GridBagLayout();
		GridBagConstraints 	c = new GridBagConstraints();
		JPanel				ruleEditorPanel = new JPanel();
		JPanel				generalPanel = new JPanel();
    JPanel        checkBoxPanel = new JPanel();

		colorPanel = new SyntaxColorsPanel(Preferences.getInstance().getSyntaxColors());

		setResizable(false);

		contentPane.setLayout(gridbag);

		// specifies component as last one on the row
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.HORIZONTAL;

    checkBoxPanel.setLayout(new BorderLayout());
    checkBoxPanel.setBorder(new CompoundBorder(
			  BorderFactory.createTitledBorder("Auto Formatting"),
			  BorderFactory.createEmptyBorder(10,10,10,10)));
    checkBoxPanel.add(autoIndentingCheckBox, BorderLayout.NORTH);
    checkBoxPanel.add(autoSoarCompleteCheckBox, BorderLayout.SOUTH);

    ruleEditorPanel.setLayout(new BorderLayout());
		ruleEditorPanel.add(colorPanel, BorderLayout.NORTH);
		ruleEditorPanel.add(checkBoxPanel, BorderLayout.SOUTH);

		generalPanel.setLayout(new BorderLayout());
		generalPanel.add(templatePanel, BorderLayout.NORTH);
		generalPanel.add(tilePanel, BorderLayout.SOUTH);


		tabPane.addTab("General", generalPanel);
		tabPane.addTab("Rule Editor", ruleEditorPanel);

		tabPane.setSelectedIndex(0);

		contentPane.add(tabPane, c);
		contentPane.add(buttonPanel, c);

		pack();
		getRootPane().setDefaultButton(buttonPanel.okButton);

		addWindowListener(new WindowAdapter() {
			public void windowOpened(WindowEvent we) {
				setLocationRelativeTo(owner);
			}
		});

		buttonPanel.cancelButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				dispose();
			}
		});

		buttonPanel.okButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				File newTemplateFolder = templatePanel.getTemplateFolder();
				approved = true;

				if (! newTemplateFolder.exists()) {

					int response = JOptionPane.showConfirmDialog(PreferencesDialog.this,
						"\"" + newTemplateFolder + "\""
							+ " is not a valid folder\nno templates will be available",
						"Invalid Path", JOptionPane.OK_CANCEL_OPTION);

					if (response == JOptionPane.CANCEL_OPTION) {
						templatePanel.requestFocus();
						return;
					}
				}
				prefs.setTemplateFolder(newTemplateFolder);
				MainFrame.getMainFrame().getTemplateManager().load(newTemplateFolder);
				prefs.setHighlightingEnabled(colorPanel.getEnableHighlighting());
				prefs.setAutoTilingEnabled(tilePanel.getAutoTile());
				prefs.setHorizontalTilingEnabled(tilePanel.getHorizontalTile());
				prefs.setAutoIndentingEnabled(autoIndentingCheckBox.isSelected());
        prefs.setAutoSoarCompleteEnabled(autoSoarCompleteCheckBox.isSelected());

        
				commitChanges();
				prefs.setSyntaxColors(colorTable);
				dispose();
			}
		});

	}     // end of constructor
	
	public void commitChanges() {
		TreeMap		colorsToChange = colorPanel.getChanges();
		Iterator	keys = colorsToChange.keySet().iterator();
		Integer		theKey;
		Color		theColor;
		int			temp;
		
		while (keys.hasNext()) {
			theKey = (Integer)keys.next();
			theColor = (Color)colorsToChange.get(theKey);
			
			temp = theKey.intValue();
			colorTable[temp] = new SyntaxColor(theColor, colorTable[temp]);
		}
	}

	public boolean wasApproved() {
		return approved;
	}
}
