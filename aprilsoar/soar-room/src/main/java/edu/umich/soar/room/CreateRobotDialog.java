package edu.umich.soar.room;

import javax.swing.JPanel;
import java.awt.Frame;
import javax.swing.JDialog;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.Insets;
import java.io.File;
import java.util.prefs.Preferences;

import javax.swing.ComboBoxModel;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JTextField;
import javax.swing.JCheckBox;
import javax.swing.event.ListDataListener;
import javax.swing.filechooser.FileFilter;

import edu.umich.soar.room.config.PlayerConfig;
import edu.umich.soar.room.core.PlayerColor;
import edu.umich.soar.room.core.Simulation;

class CreateRobotDialog extends JDialog {

	private static final long serialVersionUID = 1L;
	private JPanel jContentPane = null;
	private JLabel jProductionsFieldLabel = null;
	private JLabel jProductionsLabel = null;
	private JButton jSoarButton = null;
	private JButton jHumanButton = null;
	private JLabel jColorFieldLabel = null;
	private JLabel jLocFieldLabel = null;
	private JComboBox jColorCombo = null;
	private JTextField jTextFieldX = null;
	private JTextField jTextFieldY = null;
	private JLabel jNameFieldLabel = null;
	private JTextField jNameTextField = null;
	private JCheckBox jRandomCheckBox = null;
	private JLabel jDebuggerFieldLabel = null;
	private JCheckBox jSpawnCheckBox = null;
	private JLabel jColFieldLabel = null;
	private JLabel jRowFieldLabel = null;
	private JPanel jDonePanel = null;
	private JButton jCreateButton = null;
	private JButton jCancelButton = null;

	private final Simulation sim;
	private final Preferences pref;
	private final String HUMAN = "<human>";  //  @jve:decl-index=0:
	
	private final String KEY_PRODUCTIONS = "productions";
	private final String KEY_RANDOM = "random";  //  @jve:decl-index=0:
	private final String KEY_SOURCE_DIR = "sourcedir";  //  @jve:decl-index=0:
	private final String KEY_X = "x";  //  @jve:decl-index=0:
	private final String KEY_Y = "y";  //  @jve:decl-index=0:
	private final String KEY_SPAWN = "spawn";  //  @jve:decl-index=0:
	
	/**
	 * @param owner
	 */
	public CreateRobotDialog(Frame owner, Simulation sim) {
		super(owner);
		this.sim = sim;
		this.pref = Application.PREFERENCES.node("createAgent");
		initialize();

		setProductions(pref.get(KEY_PRODUCTIONS, ""));
		jSpawnCheckBox.setSelected(pref.getBoolean(KEY_SPAWN, true));
		jTextFieldX.setText(pref.get(KEY_X, ""));
		jTextFieldY.setText(pref.get(KEY_Y, ""));
		jRandomCheckBox.setSelected(pref.getBoolean(KEY_RANDOM, true));
		randomStateChanged();
	}
	
	private void setProductions(String absolutePath) {
		if (absolutePath != null && !absolutePath.isEmpty()) {
			productions = new File(absolutePath);
	        jProductionsLabel.setText(productions.getName());
		} else {
			productions = null;
	        jProductionsLabel.setText(HUMAN);
		}
		jSpawnCheckBox.setEnabled(productions != null);
	}

	/**
	 * This method initializes this
	 * 
	 * @return void
	 */
	private void initialize() {
		this.setSize(400, 200);
		this.setTitle("Create Robot");
		this.setContentPane(getJContentPane());
	}

	/**
	 * This method initializes jContentPane
	 * 
	 * @return javax.swing.JPanel
	 */
	private JPanel getJContentPane() {
		if (jContentPane == null) {
			GridBagConstraints gridBagConstraints23 = new GridBagConstraints();
			gridBagConstraints23.gridx = 0;
			gridBagConstraints23.gridwidth = 8;
			gridBagConstraints23.anchor = GridBagConstraints.EAST;
			gridBagConstraints23.gridy = 5;
			GridBagConstraints gridBagConstraints22 = new GridBagConstraints();
			gridBagConstraints22.gridx = 4;
			gridBagConstraints22.ipadx = 0;
			gridBagConstraints22.gridy = 3;
			jRowFieldLabel = new JLabel();
			jRowFieldLabel.setText("row");
			jRowFieldLabel.setEnabled(false);
			GridBagConstraints gridBagConstraints21 = new GridBagConstraints();
			gridBagConstraints21.gridx = 2;
			gridBagConstraints21.ipadx = 0;
			gridBagConstraints21.gridy = 3;
			jColFieldLabel = new JLabel();
			jColFieldLabel.setText("col");
			jColFieldLabel.setEnabled(false);
			GridBagConstraints gridBagConstraints17 = new GridBagConstraints();
			gridBagConstraints17.gridx = 1;
			gridBagConstraints17.gridwidth = 5;
			gridBagConstraints17.anchor = GridBagConstraints.WEST;
			gridBagConstraints17.insets = new Insets(0, 0, 0, 0);
			gridBagConstraints17.ipadx = 0;
			gridBagConstraints17.gridy = 4;
			GridBagConstraints gridBagConstraints16 = new GridBagConstraints();
			gridBagConstraints16.gridx = 0;
			gridBagConstraints16.anchor = GridBagConstraints.WEST;
			gridBagConstraints16.ipadx = 0;
			gridBagConstraints16.insets = new Insets(0, 0, 0, 6);
			gridBagConstraints16.gridy = 4;
			jDebuggerFieldLabel = new JLabel();
			jDebuggerFieldLabel.setText("Debugger:");
			GridBagConstraints gridBagConstraints15 = new GridBagConstraints();
			gridBagConstraints15.gridx = 1;
			gridBagConstraints15.anchor = GridBagConstraints.WEST;
			gridBagConstraints15.insets = new Insets(0, 0, 0, 0);
			gridBagConstraints15.ipadx = 0;
			gridBagConstraints15.gridy = 3;
			GridBagConstraints gridBagConstraints14 = new GridBagConstraints();
			gridBagConstraints14.fill = GridBagConstraints.BOTH;
			gridBagConstraints14.gridy = 2;
			gridBagConstraints14.weightx = 1.0;
			gridBagConstraints14.gridwidth = 5;
			gridBagConstraints14.anchor = GridBagConstraints.WEST;
			gridBagConstraints14.insets = new Insets(0, 0, 0, 0);
			gridBagConstraints14.ipadx = 0;
			gridBagConstraints14.gridx = 1;
			GridBagConstraints gridBagConstraints13 = new GridBagConstraints();
			gridBagConstraints13.gridx = 0;
			gridBagConstraints13.anchor = GridBagConstraints.WEST;
			gridBagConstraints13.ipadx = 0;
			gridBagConstraints13.insets = new Insets(0, 0, 0, 6);
			gridBagConstraints13.gridy = 2;
			jNameFieldLabel = new JLabel();
			jNameFieldLabel.setText("Name:");
			GridBagConstraints gridBagConstraints12 = new GridBagConstraints();
			gridBagConstraints12.fill = GridBagConstraints.BOTH;
			gridBagConstraints12.gridy = 3;
			gridBagConstraints12.weightx = 1.0;
			gridBagConstraints12.insets = new Insets(0, 0, 0, 0);
			gridBagConstraints12.ipadx = 0;
			gridBagConstraints12.gridx = 5;
			GridBagConstraints gridBagConstraints11 = new GridBagConstraints();
			gridBagConstraints11.fill = GridBagConstraints.BOTH;
			gridBagConstraints11.gridy = 3;
			gridBagConstraints11.weightx = 1.0;
			gridBagConstraints11.ipadx = 0;
			gridBagConstraints11.gridx = 3;
			GridBagConstraints gridBagConstraints10 = new GridBagConstraints();
			gridBagConstraints10.fill = GridBagConstraints.BOTH;
			gridBagConstraints10.gridy = 1;
			gridBagConstraints10.weightx = 1.0;
			gridBagConstraints10.gridwidth = 5;
			gridBagConstraints10.anchor = GridBagConstraints.WEST;
			gridBagConstraints10.insets = new Insets(0, 0, 0, 0);
			gridBagConstraints10.ipadx = 0;
			gridBagConstraints10.gridx = 1;
			GridBagConstraints gridBagConstraints9 = new GridBagConstraints();
			gridBagConstraints9.gridx = 0;
			gridBagConstraints9.anchor = GridBagConstraints.WEST;
			gridBagConstraints9.ipadx = 0;
			gridBagConstraints9.insets = new Insets(0, 0, 0, 6);
			gridBagConstraints9.gridy = 3;
			jLocFieldLabel = new JLabel();
			jLocFieldLabel.setText("Location:");
			GridBagConstraints gridBagConstraints8 = new GridBagConstraints();
			gridBagConstraints8.gridx = 0;
			gridBagConstraints8.anchor = GridBagConstraints.WEST;
			gridBagConstraints8.ipadx = 0;
			gridBagConstraints8.insets = new Insets(0, 0, 0, 6);
			gridBagConstraints8.gridy = 1;
			jColorFieldLabel = new JLabel();
			jColorFieldLabel.setText("Color:");
			GridBagConstraints gridBagConstraints7 = new GridBagConstraints();
			gridBagConstraints7.gridx = 7;
			gridBagConstraints7.ipadx = 0;
			gridBagConstraints7.fill = GridBagConstraints.HORIZONTAL;
			gridBagConstraints7.gridy = 0;
			GridBagConstraints gridBagConstraints6 = new GridBagConstraints();
			gridBagConstraints6.gridx = 6;
			gridBagConstraints6.ipadx = 0;
			gridBagConstraints6.fill = GridBagConstraints.HORIZONTAL;
			gridBagConstraints6.insets = new Insets(0, 4, 0, 0);
			gridBagConstraints6.gridy = 0;
			GridBagConstraints gridBagConstraints5 = new GridBagConstraints();
			gridBagConstraints5.gridx = 1;
			gridBagConstraints5.gridwidth = 5;
			gridBagConstraints5.anchor = GridBagConstraints.WEST;
			gridBagConstraints5.fill = GridBagConstraints.HORIZONTAL;
			gridBagConstraints5.insets = new Insets(0, 0, 0, 0);
			gridBagConstraints5.ipadx = 0;
			gridBagConstraints5.gridy = 0;
			jProductionsLabel = new JLabel();
			jProductionsLabel.setText(HUMAN);
			GridBagConstraints gridBagConstraints4 = new GridBagConstraints();
			gridBagConstraints4.gridx = 0;
			gridBagConstraints4.anchor = GridBagConstraints.WEST;
			gridBagConstraints4.ipadx = 0;
			gridBagConstraints4.insets = new Insets(0, 0, 0, 6);
			gridBagConstraints4.gridy = 0;
			jProductionsFieldLabel = new JLabel();
			jProductionsFieldLabel.setText("Productions:");
			GridBagConstraints gridBagConstraints = new GridBagConstraints();
			gridBagConstraints.insets = new Insets(0, 0, 157, 0);
			gridBagConstraints.gridy = 0;
			gridBagConstraints.ipadx = 338;
			gridBagConstraints.gridwidth = 1;
			gridBagConstraints.gridx = 0;
			jContentPane = new JPanel();
			jContentPane.setLayout(new GridBagLayout());
			jContentPane.add(jProductionsFieldLabel, gridBagConstraints4);
			jContentPane.add(jProductionsLabel, gridBagConstraints5);
			jContentPane.add(getJSoarButton(), gridBagConstraints6);
			jContentPane.add(getJHumanButton(), gridBagConstraints7);
			jContentPane.add(jColorFieldLabel, gridBagConstraints8);
			jContentPane.add(jLocFieldLabel, gridBagConstraints9);
			jContentPane.add(getJColorCombo(), gridBagConstraints10);
			jContentPane.add(getJTextFieldX(), gridBagConstraints11);
			jContentPane.add(getJTextFieldY(), gridBagConstraints12);
			jContentPane.add(jNameFieldLabel, gridBagConstraints13);
			jContentPane.add(getJNameTextField(), gridBagConstraints14);
			jContentPane.add(getJRandomCheckBox(), gridBagConstraints15);
			jContentPane.add(jDebuggerFieldLabel, gridBagConstraints16);
			jContentPane.add(getJSpawnCheckBox(), gridBagConstraints17);
			jContentPane.add(jColFieldLabel, gridBagConstraints21);
			jContentPane.add(jRowFieldLabel, gridBagConstraints22);
			jContentPane.add(getJDonePanel(), gridBagConstraints23);
		}
		return jContentPane;
	}

	/**
	 * This method initializes jSoarButton	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getJSoarButton() {
		if (jSoarButton == null) {
			jSoarButton = new JButton();
			jSoarButton.setText("Soar");
			jSoarButton.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e) {
					choose();
				}
			});
		}
		return jSoarButton;
	}

	/**
	 * This method initializes jHumanButton	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getJHumanButton() {
		if (jHumanButton == null) {
			jHumanButton = new JButton();
			jHumanButton.setText("Human");
			jHumanButton.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e) {
					setProductions("");
				}
			});
		}
		return jHumanButton;
	}

	/**
	 * This method initializes jColorCombo	
	 * 	
	 * @return javax.swing.JComboBox	
	 */
	private JComboBox getJColorCombo() {
		if (jColorCombo == null) {
			jColorCombo = new JComboBox();
			jColorCombo.setModel(new ComboBoxModel() {
				private int selection = 0;
				
				@Override
				public Object getElementAt(int index) {
					return PlayerColor.getUnusedColors().get(index);
				}
				@Override
				public Object getSelectedItem() {
					if (selection < 0) {
						return null;
					}
					return PlayerColor.getUnusedColors().get(selection);
				}
				@Override
				public int getSize() {
					return PlayerColor.getUnusedColors().size();
				}
				@Override
				public void setSelectedItem(Object anItem) {
					if (anItem == null) {
						selection = -1;
					}
					PlayerColor color = (PlayerColor)anItem;
					if (color == null) {
						selection = -1;
					}
					selection = PlayerColor.getUnusedColors().indexOf(color);
				}
				@Override
				public void addListDataListener(ListDataListener l) {
				}
				@Override
				public void removeListDataListener(ListDataListener l) {
				}
			});
		}
		return jColorCombo;
	}

	/**
	 * This method initializes jTextFieldX	
	 * 	
	 * @return javax.swing.JTextField	
	 */
	private JTextField getJTextFieldX() {
		if (jTextFieldX == null) {
			jTextFieldX = new JTextField();
			jTextFieldX.setEnabled(false);
			jTextFieldX.addKeyListener(new java.awt.event.KeyAdapter() {
				public void keyTyped(java.awt.event.KeyEvent e) {
					if (!Character.isDigit(e.getKeyChar())) {
						e.consume();
					}
				}
			});
		}
		return jTextFieldX;
	}

	/**
	 * This method initializes jTextFieldY	
	 * 	
	 * @return javax.swing.JTextField	
	 */
	private JTextField getJTextFieldY() {
		if (jTextFieldY == null) {
			jTextFieldY = new JTextField();
			jTextFieldY.setEnabled(false);
			jTextFieldY.addKeyListener(new java.awt.event.KeyAdapter() {
				public void keyTyped(java.awt.event.KeyEvent e) {
					if (!Character.isDigit(e.getKeyChar())) {
						e.consume();
					}
				}
			});
		}
		return jTextFieldY;
	}

	/**
	 * This method initializes jNameTextField	
	 * 	
	 * @return javax.swing.JTextField	
	 */
	private JTextField getJNameTextField() {
		if (jNameTextField == null) {
			jNameTextField = new JTextField();
			PlayerColor color = (PlayerColor)jColorCombo.getSelectedItem();
			jNameTextField.setText(color.toString().toLowerCase());
		}
		return jNameTextField;
	}

	/**
	 * This method initializes jRandomCheckBox	
	 * 	
	 * @return javax.swing.JCheckBox	
	 */
	private JCheckBox getJRandomCheckBox() {
		if (jRandomCheckBox == null) {
			jRandomCheckBox = new JCheckBox();
			jRandomCheckBox.setText("Random");
			jRandomCheckBox.addChangeListener(new javax.swing.event.ChangeListener() {
				public void stateChanged(javax.swing.event.ChangeEvent e) 
				{
					randomStateChanged();
				}
			});
		}
		return jRandomCheckBox;
	}
	
	private void randomStateChanged() {
		boolean enabled = !jRandomCheckBox.isSelected();
		jTextFieldX.setEnabled(enabled);
		jTextFieldY.setEnabled(enabled);
		jColFieldLabel.setEnabled(enabled);
		jRowFieldLabel.setEnabled(enabled);
	}

	/**
	 * This method initializes jSpawnCheckBox	
	 * 	
	 * @return javax.swing.JCheckBox	
	 */
	private JCheckBox getJSpawnCheckBox() {
		if (jSpawnCheckBox == null) {
			jSpawnCheckBox = new JCheckBox();
			jSpawnCheckBox.setText("Spawn on agent creation");
			jSpawnCheckBox.setEnabled(productions != null);
		}
		return jSpawnCheckBox;
	}

	/**
	 * This method initializes jDonePanel	
	 * 	
	 * @return javax.swing.JPanel	
	 */
	private JPanel getJDonePanel() {
		if (jDonePanel == null) {
			jDonePanel = new JPanel();
			jDonePanel.setLayout(new GridBagLayout());
			jDonePanel.add(getJCreateButton(), new GridBagConstraints());
			jDonePanel.add(getJCancelButton(), new GridBagConstraints());
		}
		return jDonePanel;
	}

	/**
	 * This method initializes jCreateButton	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getJCreateButton() {
		if (jCreateButton == null) {
			jCreateButton = new JButton();
			jCreateButton.setText("Create Agent");
			jCreateButton.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e) {
			        PlayerConfig playerConfig = new PlayerConfig();
			        if (productions != null) {
			        	playerConfig.productions = productions.getAbsolutePath();
			        }
			        if (!jRandomCheckBox.isSelected()) {
			        	try {
			        		playerConfig.pos = new int[] { Integer.parseInt(jTextFieldX.getText()), Integer.parseInt(jTextFieldY.getText()) };
			        	} catch (NumberFormatException err) {
			        		playerConfig.pos = null;
			        	} catch (NullPointerException err) {
			        		playerConfig.pos = null;
			        	}
			        }
			        if (!jNameTextField.getText().isEmpty()) {
			        	playerConfig.name = jNameTextField.getText();
			        }
			        sim.getCogArch().setDebug(jSpawnCheckBox.isSelected());
			        sim.createPlayer(playerConfig);

			        if (productions == null) {
			        	pref.put(KEY_PRODUCTIONS, "");
			        } else {
			        	pref.put(KEY_PRODUCTIONS, productions.getAbsolutePath());
			        	pref.put(KEY_SOURCE_DIR, productions.getParent());
			        }
			        pref.putBoolean(KEY_RANDOM, jRandomCheckBox.isSelected());
			        pref.put(KEY_X, jTextFieldX.getText());
			        pref.put(KEY_Y, jTextFieldY.getText());
			        pref.putBoolean(KEY_SPAWN, jSpawnCheckBox.isSelected());

			        dispose();
				}
			});
		}
		return jCreateButton;
	}

	/**
	 * This method initializes jCancelButton	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getJCancelButton() {
		if (jCancelButton == null) {
			jCancelButton = new JButton();
			jCancelButton.setText("Cancel");
			jCancelButton.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e) {
					dispose();
				}
			});
		}
		return jCancelButton;
	}
	
	
	private File productions;  //  @jve:decl-index=0:
	private void choose() {
		
        JFileChooser chooser = new JFileChooser(pref.get(KEY_SOURCE_DIR, System.getProperty("user.dir", ".")));
        chooser.setFileFilter(new FileFilter() {

            @Override
            public boolean accept(File f)
            {
            	if (f.isDirectory()) {
            		return true;
            	}
            	if (!f.isFile()) {
            		return false;
            	}
            	int extPos = f.getAbsolutePath().lastIndexOf(".");
            	if (extPos < 0) {
            		return false;
            	}
            	return f.getAbsolutePath().substring(extPos + 1).equalsIgnoreCase("soar");
            }

            @Override
            public String getDescription()
            {
                return "Soar Files (*.soar)";
            }});
        
        if(JFileChooser.CANCEL_OPTION == chooser.showOpenDialog(this))
        {
            return;
        }
        
        setProductions(chooser.getSelectedFile().getAbsolutePath());
	}
}
