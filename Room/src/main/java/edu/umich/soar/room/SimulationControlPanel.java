package edu.umich.soar.room;

import java.awt.Dimension;
import java.awt.GridBagLayout;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import java.awt.GridBagConstraints;
import java.util.prefs.Preferences;

import javax.swing.BorderFactory;
import javax.swing.ButtonGroup;
import javax.swing.JSlider;
import javax.swing.JLabel;
import javax.swing.JButton;
import javax.swing.JTextField;
import javax.swing.JToggleButton;
import javax.swing.SwingConstants;

import edu.umich.soar.room.GoAction.GoProvider;
import edu.umich.soar.room.core.Simulation;

public class SimulationControlPanel extends JPanel implements GoProvider {

	private static final long serialVersionUID = 1L;
	private JToggleButton jToggleBreadcrumbsButton = null;
	private JButton jClearBreadcrumbsButton = null;
	private JRadioButton jSyncRadioButton = null;
	private JRadioButton jAsyncRadioButton = null;
	private JPanel jRunPanel = null;
	private JRadioButton jForeverRadioButton = null;
	private JRadioButton jTicksRadioButton = null;
	private JTextField jQuantityTextField = null;
	private JButton jRunButton = null;
	private JButton jStopButton = null;
	private JButton jResetButton = null;
	private JPanel jTypePanel = null;
	private JPanel jBreadcrumbsPanel = null;
	private JPanel jDelayPanel = null;
	private JSlider jDelaySlider = null;
	private JLabel jMsecPerTickLabel = null;
	private JPanel jInfoPanel = null;
	private JLabel jCountsLabel = null;

	private ButtonGroup typeButtonGroup = new ButtonGroup();  
	private ButtonGroup runButtonGroup = new ButtonGroup(); 
	
	private final String KEY_RUN_QTY = "runQty";
	private final String KEY_RUN_FOREVER = "runForever"; 
	private final Simulation sim;
	private final Preferences pref;
	private final ActionManager am;
	
	/**
	 * This is the default constructor
	 */
	public SimulationControlPanel(Adaptable app, Preferences pref) {
		super();
		this.sim = Adaptables.adapt(app, Simulation.class);
        this.am = Adaptables.adapt(app, ActionManager.class);
		this.pref = pref;
		
		initialize();

        int runQty = pref.getInt(KEY_RUN_QTY, 1);
    	boolean runForever = pref.getBoolean(KEY_RUN_FOREVER, true);

		getJAsyncRadioButton().setSelected(this.sim.getCogArch().isAsync());
		getJSyncRadioButton().setSelected(!this.sim.getCogArch().isAsync());
		
		getJQuantityTextField().setText(Integer.toString(runQty));
		getJForeverRadioButton().setSelected(runForever);
		getJTicksRadioButton().setSelected(!runForever);
		
		getJDelaySlider().setValue(100);

		foreverChanged();
		delayChanged();
		updateCounts();
	}

	/**
	 * This method initializes this
	 * 
	 * @return void
	 */
	private void initialize() {
		GridBagConstraints gridBagConstraints51 = new GridBagConstraints();
		gridBagConstraints51.fill = GridBagConstraints.BOTH;
		gridBagConstraints51.gridy = 0;
		gridBagConstraints51.weightx = 1;
		gridBagConstraints51.weighty = 1;
		
		GridBagConstraints gridBagConstraints2 = new GridBagConstraints();
		gridBagConstraints2.fill = GridBagConstraints.BOTH;
		gridBagConstraints2.gridy = 1;
		gridBagConstraints2.weightx = 1;
		gridBagConstraints2.weighty = 1;
		
		GridBagConstraints gridBagConstraints10 = new GridBagConstraints();
		gridBagConstraints10.fill = GridBagConstraints.BOTH;
		gridBagConstraints10.gridy = 2;
		gridBagConstraints10.weightx = 1;
		gridBagConstraints10.weighty = 1;

		GridBagConstraints gridBagConstraints52 = new GridBagConstraints();
		gridBagConstraints52.fill = GridBagConstraints.BOTH;
		gridBagConstraints52.gridy = 3;
		gridBagConstraints52.weightx = 1;
		gridBagConstraints52.weighty = 1;
		
		GridBagConstraints gridBagConstraints53 = new GridBagConstraints();
		gridBagConstraints53.fill = GridBagConstraints.BOTH;
		gridBagConstraints53.gridy = 4;
		gridBagConstraints53.weightx = 1;
		gridBagConstraints53.weighty = 1;
		
		this.setLayout(new GridBagLayout());
		this.add(getJIntegrationPanel(), gridBagConstraints51);
		this.add(getJRunPanel(), gridBagConstraints2);
		this.add(getJDelayPanel(), gridBagConstraints10);
		this.add(getJInfoPanel(), gridBagConstraints52);
		this.add(getJBreadcrumbsPanel(), gridBagConstraints53);
	}

	/**
	 * This method initializes jSyncRadioButton	
	 * 	
	 * @return javax.swing.JRadioButton	
	 */
	private JRadioButton getJSyncRadioButton() {
		if (jSyncRadioButton == null) {
			jSyncRadioButton = new JRadioButton();
			jSyncRadioButton.setText("Synchronous");
			jSyncRadioButton.addChangeListener(new javax.swing.event.ChangeListener() {
				public void stateChanged(javax.swing.event.ChangeEvent e) {
					typeChanged();
				}
			});
			typeButtonGroup.add(jSyncRadioButton);
		}
		return jSyncRadioButton;
	}

	private JToggleButton getJToggleBreadcrumbsButton() {
		if (jToggleBreadcrumbsButton == null) {
			jToggleBreadcrumbsButton = new JToggleButton(this.am.getAction(ToggleBreadcrumbsAction.class));
		}
		return jToggleBreadcrumbsButton;
	}

	private JButton getJClearBreadcrumbsButton() {
		if (jClearBreadcrumbsButton == null) {
			jClearBreadcrumbsButton = new JButton(this.am.getAction(ClearBreadcrumbsAction.class));
		}
		return jClearBreadcrumbsButton;
	}

	/**
	 * This method initializes jAsyncRadioButton	
	 * 	
	 * @return javax.swing.JRadioButton	
	 */
	private JRadioButton getJAsyncRadioButton() {
		if (jAsyncRadioButton == null) {
			jAsyncRadioButton = new JRadioButton();
			jAsyncRadioButton.setText("Asynchronous");
			jAsyncRadioButton.addChangeListener(new javax.swing.event.ChangeListener() {
				public void stateChanged(javax.swing.event.ChangeEvent e) {
					typeChanged();
				}
			});
			typeButtonGroup.add(jAsyncRadioButton);
		}
		return jAsyncRadioButton;
	}
	
	private void typeChanged() {
		sim.getCogArch().setAsync(jAsyncRadioButton.isSelected());
	}

	/**
	 * This method initializes jRunPanel	
	 * 	
	 * @return javax.swing.JPanel	
	 */
	private JPanel getJRunPanel() {
		if (jRunPanel == null) {
			jRunPanel = new JPanel();
			jRunPanel.setLayout(new GridBagLayout());
			jRunPanel.setBorder(BorderFactory.createTitledBorder("Run"));

			GridBagConstraints gridBagConstraints3 = new GridBagConstraints();
			gridBagConstraints3.gridx = 0;
			gridBagConstraints3.anchor = GridBagConstraints.WEST;
			gridBagConstraints3.gridy = 0;
			jRunPanel.add(getJForeverRadioButton(), gridBagConstraints3);
			
			GridBagConstraints gridBagConstraints5 = new GridBagConstraints();
			gridBagConstraints5.gridx = 0;
			gridBagConstraints5.anchor = GridBagConstraints.WEST;
			gridBagConstraints5.gridy = 1;
			jRunPanel.add(getJTicksRadioButton(), gridBagConstraints5);

			GridBagConstraints gridBagConstraints6 = new GridBagConstraints();
			gridBagConstraints6.fill = GridBagConstraints.BOTH;
			gridBagConstraints6.gridx = 1;
			gridBagConstraints6.gridy = 1;
			gridBagConstraints6.weightx = 1.0;
			//gridBagConstraints6.gridheight = 2;
			jRunPanel.add(getJQuantityTextField(), gridBagConstraints6);

			GridBagConstraints gridBagConstraints7 = new GridBagConstraints();
			gridBagConstraints7.gridx = 2;
			gridBagConstraints7.gridheight = 2;
			gridBagConstraints7.fill = GridBagConstraints.BOTH;
			gridBagConstraints7.gridy = 0;
			jRunPanel.add(getJGoButton(), gridBagConstraints7);

			GridBagConstraints gridBagConstraints8 = new GridBagConstraints();
			gridBagConstraints8.gridx = 3;
			gridBagConstraints8.fill = GridBagConstraints.HORIZONTAL;
			gridBagConstraints8.gridy = 0;
			jRunPanel.add(getJStopButton(), gridBagConstraints8);
			
			GridBagConstraints gridBagConstraints9 = new GridBagConstraints();
			gridBagConstraints9.gridx = 3;
			gridBagConstraints9.fill = GridBagConstraints.HORIZONTAL;
			gridBagConstraints9.gridy = 1;
			jRunPanel.add(getJResetButton(), gridBagConstraints9);
		}
		return jRunPanel;
	}

	/**
	 * This method initializes jForeverRadioButton	
	 * 	
	 * @return javax.swing.JRadioButton	
	 */
	private JRadioButton getJForeverRadioButton() {
		if (jForeverRadioButton == null) {
			jForeverRadioButton = new JRadioButton();
			jForeverRadioButton.setText("Forever");
			jForeverRadioButton.setSelected(true);
			runButtonGroup.add(jForeverRadioButton);
			jForeverRadioButton.addChangeListener(new javax.swing.event.ChangeListener() {
				public void stateChanged(javax.swing.event.ChangeEvent e) {
					foreverChanged();
				}
			});
		}
		return jForeverRadioButton;
	}
	
	private void foreverChanged() {
		jQuantityTextField.setEnabled(!jForeverRadioButton.isSelected());
	}

	/**
	 * This method initializes jTicksRadioButton	
	 * 	
	 * @return javax.swing.JRadioButton	
	 */
	private JRadioButton getJTicksRadioButton() {
		if (jTicksRadioButton == null) {
			jTicksRadioButton = new JRadioButton();
			jTicksRadioButton.setText("Ticks");
			runButtonGroup.add(jTicksRadioButton);
			jTicksRadioButton.addChangeListener(new javax.swing.event.ChangeListener() {
				public void stateChanged(javax.swing.event.ChangeEvent e) {
					jQuantityTextField.setEnabled(true);
				}
			});
		}
		return jTicksRadioButton;
	}

	/**
	 * This method initializes jQuantityTextField	
	 * 	
	 * @return javax.swing.JTextField	
	 */
	private JTextField getJQuantityTextField() {
		if (jQuantityTextField == null) {
			jQuantityTextField = new JTextField();
			jQuantityTextField.setText("1");
			jQuantityTextField.setColumns(4);
			jQuantityTextField.setMinimumSize(new Dimension(25, 0));
			jQuantityTextField.addKeyListener(new java.awt.event.KeyAdapter() {
				public void keyTyped(java.awt.event.KeyEvent e) {
					if (!Character.isDigit(e.getKeyChar())) {
						e.consume();
					}
				}
			});
		}
		return jQuantityTextField;
	}

	/**
	 * This method initializes jGoButton	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getJGoButton() {
		if (jRunButton == null) {
			jRunButton = new JButton();
			GoAction ga = am.getAction(GoAction.class);
			ga.setGoProvider(this);
			jRunButton.setAction(ga);
			//jRunButton.setText("Go");
		}
		return jRunButton;
	}

	/**
	 * This method initializes jStopButton	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getJStopButton() {
		if (jStopButton == null) {
			jStopButton = new JButton();
			jStopButton.setAction(am.getAction(StopAction.class));
			//jStopButton.setText("Stop");
		}
		return jStopButton;
	}

	/**
	 * This method initializes jResetButton	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getJResetButton() {
		if (jResetButton == null) {
			jResetButton = new JButton();
			jResetButton.setAction(am.getAction(ResetAction.class));
			//jResetButton.setText("Reset");
		}
		return jResetButton;
	}

	/**
	 * This method initializes jIntegrationPanel	
	 * 	
	 * @return javax.swing.JPanel	
	 */
	private JPanel getJIntegrationPanel() {
		if (jTypePanel == null) {
			GridBagConstraints gridBagConstraints1 = new GridBagConstraints();
			gridBagConstraints1.gridx = 0;
			gridBagConstraints1.gridy = 0;
			//gridBagConstraints1.weightx = 1;
			gridBagConstraints1.anchor = GridBagConstraints.WEST;
			GridBagConstraints gridBagConstraints = new GridBagConstraints();
			gridBagConstraints.gridx = 1;
			gridBagConstraints.gridy = -1;
			gridBagConstraints.weightx = 1;
			gridBagConstraints.anchor = GridBagConstraints.WEST;
			gridBagConstraints.fill = GridBagConstraints.HORIZONTAL;
			
			jTypePanel = new JPanel();
			jTypePanel.setLayout(new GridBagLayout());
			jTypePanel.setBorder(BorderFactory.createTitledBorder("Type"));
			jTypePanel.add(getJSyncRadioButton(), gridBagConstraints);
			jTypePanel.add(getJAsyncRadioButton(), gridBagConstraints1);
		}
		return jTypePanel;
	}

	private JPanel getJBreadcrumbsPanel() {
		if (jBreadcrumbsPanel == null) {
			
			GridBagConstraints gridBagConstraints1 = new GridBagConstraints();
			gridBagConstraints1.gridx = 0;
			gridBagConstraints1.gridy = 0;
			gridBagConstraints1.anchor = GridBagConstraints.WEST;
			
			GridBagConstraints gridBagConstraints = new GridBagConstraints();
			gridBagConstraints.gridx = 1;
			gridBagConstraints.gridy = -1;
			gridBagConstraints.weightx = 1;
			gridBagConstraints.anchor = GridBagConstraints.WEST;
			gridBagConstraints.fill = GridBagConstraints.HORIZONTAL;
			
			jBreadcrumbsPanel = new JPanel();
			jBreadcrumbsPanel.setLayout(new GridBagLayout());
			jBreadcrumbsPanel.setBorder(BorderFactory.createTitledBorder("Breadcrumbs"));
			jBreadcrumbsPanel.add(getJToggleBreadcrumbsButton(), gridBagConstraints);
			jBreadcrumbsPanel.add(getJClearBreadcrumbsButton(), gridBagConstraints1);
		}
		return jBreadcrumbsPanel;
	}

	private JPanel getJInfoPanel() {
		if (jInfoPanel == null) {
			GridBagConstraints gridBagConstraints1 = new GridBagConstraints();
			gridBagConstraints1.gridx = 0;
			gridBagConstraints1.gridy = 0;
			gridBagConstraints1.weightx = 1;
			gridBagConstraints1.weighty = 1;
			gridBagConstraints1.fill = GridBagConstraints.BOTH;
			gridBagConstraints1.anchor = GridBagConstraints.WEST;
			jInfoPanel = new JPanel();
			jInfoPanel.setLayout(new GridBagLayout());
			jInfoPanel.setBorder(BorderFactory.createTitledBorder("Info"));
			jCountsLabel = new JLabel();
			jCountsLabel.setHorizontalAlignment(SwingConstants.LEFT);
			jCountsLabel.setVerticalAlignment(SwingConstants.TOP);
			jInfoPanel.add(jCountsLabel, gridBagConstraints1);
		}
		return jInfoPanel;
	}

	/**
	 * This method initializes jDelayPanel	
	 * 	
	 * @return javax.swing.JPanel	
	 */
	private JPanel getJDelayPanel() {
		if (jDelayPanel == null) {
			GridBagConstraints gridBagConstraints12 = new GridBagConstraints();
			gridBagConstraints12.gridx = 0;
			gridBagConstraints12.gridy = 1;
			jMsecPerTickLabel = new JLabel();
			GridBagConstraints gridBagConstraints11 = new GridBagConstraints();
			gridBagConstraints11.fill = GridBagConstraints.BOTH;
			gridBagConstraints11.gridy = 0;
			gridBagConstraints11.weightx = 1.0;
			gridBagConstraints11.gridx = 0;
			jDelayPanel = new JPanel();
			jDelayPanel.setLayout(new GridBagLayout());
			jDelayPanel.setBorder(BorderFactory.createTitledBorder("Simulation Speed"));

			jDelayPanel.add(getJDelaySlider(), gridBagConstraints11);
			jDelayPanel.add(jMsecPerTickLabel, gridBagConstraints12);
		}
		return jDelayPanel;
	}

	/**
	 * This method initializes jDelaySlider	
	 * 	
	 * @return javax.swing.JSlider	
	 */
	private JSlider getJDelaySlider() {
		if (jDelaySlider == null) {
			jDelaySlider = new JSlider();
			jDelaySlider.setValue(100);
			jDelaySlider.setExtent(25);
			jDelaySlider.setMaximum(SLIDER_MAX);
			jDelaySlider.setMinimum(SLIDER_MIN);
			jDelaySlider.addChangeListener(new javax.swing.event.ChangeListener() {
				public void stateChanged(javax.swing.event.ChangeEvent e) {
					delayChanged();
				}
			});
		}
		return jDelaySlider;
	}
	
	private final int SLIDER_MAX = 500;
	private final int SLIDER_MIN = 0;
	
	@Override
	public double getTimeScale() {
		if (jDelaySlider.getValue() == SLIDER_MAX) {
			return Double.MAX_VALUE;
		}
		return jDelaySlider.getValue() / 100.0;
	}
	
	private void delayChanged() {
		double timeScale = getTimeScale();
		if (timeScale == Double.MAX_VALUE) {
			jMsecPerTickLabel.setText("maximum");
		} else {
			jMsecPerTickLabel.setText(String.format("%1.1f", timeScale) + "x real");
		}
	}

	public void dispose() {
		try {
			pref.putInt(KEY_RUN_QTY, Integer.parseInt(jQuantityTextField.getText()));
		} catch (NumberFormatException e) {
			pref.putInt(KEY_RUN_QTY, 0);
		}
		
    	pref.putBoolean(KEY_RUN_FOREVER, runButtonGroup.isSelected(jForeverRadioButton.getModel()));
	}

	@Override
	public boolean isRunForever() {
		if (runButtonGroup.isSelected(jTicksRadioButton.getModel())) {
			return false;
		}
		return true;
	}

	@Override
	public int getQuantity() {
		return Integer.valueOf(jQuantityTextField.getText());
	}

	public void updateCounts() {
		StringBuilder countsText = new StringBuilder();
		countsText.append("<html> <b>World&nbsp;Count:</b>&nbsp;");
		countsText.append(sim.getWorldCount());
		countsText.append("&nbsp;&nbsp;&nbsp;");
		countsText.append(" <br /><b>Soar&nbsp;Update&nbsp;Count:</b>&nbsp;");
		countsText.append(sim.getCogArch().getUpdateCount());
		countsText.append("&nbsp;&nbsp;&nbsp;");
		countsText.append(" </html>");
		jCountsLabel.setText(countsText.toString());
	}
} 

