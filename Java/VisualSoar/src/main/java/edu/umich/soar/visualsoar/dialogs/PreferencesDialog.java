package edu.umich.soar.visualsoar.dialogs;
 
import java.io.File;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import java.util.*;

import edu.umich.soar.visualsoar.MainFrame;
import edu.umich.soar.visualsoar.misc.Prefs;
import edu.umich.soar.visualsoar.misc.SyntaxColor;
import edu.umich.soar.visualsoar.ruleeditor.SoarDocument;
          
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
	AutoTilePanel		tilePanel = new AutoTilePanel();

	/**
	 * Panel which contains all the buttons
	 */
	DefaultButtonPanel 	buttonPanel = new DefaultButtonPanel();

	/**
	 * The syntax colors
	 */
	SyntaxColor[]		colorTable = Prefs.getSyntaxColors();
	
	boolean				approved = false;

	JCheckBox     autoIndentingCheckBox = new JCheckBox("Auto-Indenting", Prefs.autoIndentingEnabled.getBoolean());
  JCheckBox     autoSoarCompleteCheckBox = new JCheckBox("Auto-Soar Complete", Prefs.autoSoarCompleteEnabled.getBoolean());



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

		colorPanel = new SyntaxColorsPanel(Prefs.getSyntaxColors());

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
				approved = true;

				Prefs.highlightingEnabled.setBoolean(colorPanel.getEnableHighlighting());
				Prefs.autoTileEnabled.setBoolean(tilePanel.getAutoTile());
				Prefs.horizTile.setBoolean(tilePanel.getHorizontalTile());
				Prefs.autoIndentingEnabled.setBoolean(autoIndentingCheckBox.isSelected());
				Prefs.autoIndentingEnabled.setBoolean(autoSoarCompleteCheckBox.isSelected());
        
				commitChanges();
				Prefs.setSyntaxColors(colorTable);
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
