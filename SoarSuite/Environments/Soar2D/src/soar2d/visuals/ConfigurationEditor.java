package soar2d.visuals;

import java.io.*;
import java.util.*;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import soar2d.*;
import soar2d.Configuration.*;
import soar2d.player.*;

public class ConfigurationEditor extends Dialog {

	Configuration config;
	
	Tree tree;
	Composite rhs;
	Composite currentPage;
	Shell dialog;
	
	// general
	Button useSeed;
	Text seedText;
	Button tanksoarButton;
	Button eatersButton;
	Text mapText;
	Button remote;
	Button hide;
	Button gui;

	// logging
	Combo loggingLevelCombo;
	Button loggingFileButton;
	Text loggingNameText;
	Button loggingConsoleButton;
	Button loggingTimeButton;
	
	// agents
	Text agentsNameText;
	Text agentsProductionsText;
	Combo agentsColorCombo;
	Button agentsDebuggersButton;
	PlayerConfig playerConfig;
	int playerConfigIndex;
	Button agentsNameButton;
	Button agentsProductions;
	Button agentsColor;
	Button agentCoordinates;
	Text agentCoordinatesX;
	Text agentCoordinatesY;
	Button agentFacing;
	Combo agentFacingCombo;
	Button agentPoints;
	Text agentPointsText;
	Button agentHealth;
	Text agentHealthText;
	Button agentEnergy;
	Text agentEnergyText;
	Button agentMissiles;
	Text agentMissilesText;
	Button createAgentButton;
	Button productionsBrowse;
	Button removeAgentButton;
	
	// terminals
	Button terminalMaxUpdates;
	Text maxUpdates;
	Button terminalAgentCommand;
	Button terminalPointsRemaining;
	Button terminalWinningScore;
	Text winningScore;
	Button terminalFoodRemaining;
	Button terminalUnopenedBoxes;
	
	// clients
	ClientConfig clientConfig;
	int clientConfigIndex;
	Text clientName;
	Button clientCommandButton;
	Text clientCommand;
	Button clientTimeoutButton;
	Text clientTimeout;
	Button clientAfter;
	Button createClientButton;
	Button removeClientButton;

	public ConfigurationEditor(Shell parent) {
		super(parent);
		config = new Configuration(Soar2D.config);
	}

	public void open() {

		Shell parent = getParent();
		dialog = new Shell(parent, SWT.DIALOG_TRIM | SWT.APPLICATION_MODAL);
		dialog.setText("Configuration Editor");
		{ 
			GridLayout gl = new GridLayout();
			gl.numColumns = 2;
			dialog.setLayout(gl);
		}

		tree = new Tree(dialog, SWT.BORDER);
		{
			GridData gd = new GridData();
			gd.heightHint = 400;
			gd.widthHint = 100;
			tree.setLayoutData(gd);
		}
		tree.addListener(SWT.Selection, new Listener () {
			public void handleEvent (Event e) {
				updateCurrentPage();
			}
		});
		
		final String kGeneral = "General";
		final String kLogging = "Logging";
		final String kAgents = "Agents";
		final String kTerminals = "Terminals";
		final String kClients = "Clients";

		TreeItem general = new TreeItem(tree, SWT.NONE);
		general.setText(kGeneral);
		
		TreeItem logging = new TreeItem(tree, SWT.NONE);
		logging.setText(kLogging);
		
		TreeItem agents = new TreeItem(tree, SWT.NONE);
		agents.setText(kAgents);
		
		Iterator<PlayerConfig> playerIter = config.players.iterator();
		while (playerIter.hasNext()) {
			TreeItem agent = new TreeItem(agents, SWT.NONE);
			String name = playerIter.next().getName();
			if (name == null) {
				name = "<unnamed>";
			}
			agent.setText(name);
		}

		TreeItem terminals = new TreeItem(tree, SWT.NONE);
		terminals.setText(kTerminals);
		
		TreeItem clients = new TreeItem(tree, SWT.NONE);
		clients.setText(kClients);

		Iterator<ClientConfig> clientIter = config.clients.iterator();
		while (clientIter.hasNext()) {
			TreeItem client = new TreeItem(clients, SWT.NONE);
			String name = clientIter.next().name;
			if (name == null) {
				//TODO: warn
				continue;
			}
			client.setText(name);
		}

		rhs = new Composite(dialog, SWT.NONE);
		{
			GridLayout gl = new GridLayout();
			gl.marginHeight = 0;
			gl.marginWidth = 0;
			rhs.setLayout(gl);

			GridData gd = new GridData();
			gd.widthHint = 400;
			gd.horizontalAlignment = SWT.FILL;
			gd.verticalAlignment = SWT.FILL;
			rhs.setLayoutData(gd);
		}

		Composite bottomButtons = new Composite(dialog, SWT.NONE);
		bottomButtons.setLayout(new FillLayout());
		{
			GridData gd = new GridData();
			gd.horizontalSpan = 2;
			gd.horizontalAlignment = SWT.END;
			bottomButtons.setLayoutData(gd);
		}
		
		Button resetDefaults = new Button(bottomButtons, SWT.PUSH);
		resetDefaults.setText("Reset defaults");
		resetDefaults.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config = new Configuration();
				config.setType(Soar2D.config.getType());
				updateCurrentPage();
			}
		});
		
		Button saveAs = new Button(bottomButtons, SWT.PUSH);
		saveAs.setText("Save as...");
		saveAs.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				dialog.dispose();
			}
		});
		
		Button cancel = new Button(bottomButtons, SWT.PUSH);
		cancel.setText("Cancel");
		cancel.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				dialog.dispose();
			}
		});
		
		dialog.setSize(dialog.computeSize(SWT.DEFAULT, SWT.DEFAULT, false));
		
		dialog.open();
		Display display = parent.getDisplay();
		while (!dialog.isDisposed()) {
			if (!display.readAndDispatch()) {
				display.sleep();
			}
		}
	}
	
	public void updateCurrentPage() {
		if (tree.getSelectionCount() > 0) {
			TreeItem selectedItem = tree.getSelection()[0];
			TreeItem parentItem = selectedItem.getParentItem();
			int index = -1;
			if (parentItem == null) {
				parentItem = selectedItem;
				selectedItem = null;
			} else {
				index = parentItem.indexOf(selectedItem);
			}
			switch (tree.indexOf(parentItem)) {
			case 0:
				generalPage();
				break;
			case 1:
				loggingPage();
				break;
			case 2:
				agentsPage(selectedItem, index);
				break;
			case 3:
				terminalsPage();
				break;
			case 4:
				clientsPage(selectedItem, index);
				break;
			}
		}		
	}
	
	public void generalPage() {
		if (currentPage != null) {
			currentPage.dispose();
		}
		currentPage = new Composite(rhs, SWT.NONE);
		{
			GridLayout gl = new GridLayout();
			gl.marginHeight = 0;
			gl.marginWidth = 0;
			gl.numColumns = 3;
			currentPage.setLayout(gl);
			
			GridData gd = new GridData();
			gd.grabExcessHorizontalSpace = true;
			gd.grabExcessVerticalSpace = true;
			gd.horizontalAlignment = SWT.FILL;
			gd.verticalAlignment = SWT.FILL;
			currentPage.setLayoutData(gd);
		}
		
		// eaters or tanksoar
		{
			Group simGroup = new Group(currentPage, SWT.NONE);
			simGroup.setText("Simulation");
			{
				GridData gd = new GridData();
				gd.horizontalSpan = 3;
				gd.grabExcessHorizontalSpace = true;
				gd.horizontalAlignment = SWT.FILL;
				gd.verticalAlignment = SWT.TOP;
				simGroup.setLayoutData(gd);
				
				GridLayout gl = new GridLayout();
				simGroup.setLayout(gl);
			}
			
			tanksoarButton = new Button(simGroup, SWT.RADIO);
			tanksoarButton.setText("TankSoar");
			tanksoarButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					config.setType(SimType.kTankSoar);
					generalUpdate();
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = SWT.BEGINNING;
				tanksoarButton.setLayoutData(gd);
			}

			eatersButton = new Button(simGroup, SWT.RADIO);
			eatersButton.setText("Eaters");
			eatersButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					config.setType(SimType.kEaters);
					generalUpdate();
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = SWT.BEGINNING;
				eatersButton.setLayoutData(gd);
			}
		}
		
		// map
		{
			Label mapLabel = new Label(currentPage, SWT.NONE);
			mapLabel.setText("Map:");
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.BEGINNING;
				gd.horizontalSpan = 3;
				mapLabel.setLayoutData(gd);
			}
			
			mapText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
			mapText.addFocusListener(new FocusAdapter() {
				public void focusLost(FocusEvent e) {
					String mapFileString = mapText.getText();
					if (mapFileString != null) {
						config.map = new File(mapFileString);
					}
					generalUpdate();
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.FILL;
				gd.horizontalSpan = 2;
				gd.grabExcessHorizontalSpace = true;
				mapText.setLayoutData(gd);
			}
			
			Button mapBrowse = new Button(currentPage, SWT.PUSH);
			mapBrowse.setText("Browse...");
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.END;
				mapBrowse.setLayoutData(gd);
			}
			mapBrowse.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					FileDialog fd = new FileDialog(dialog, SWT.OPEN);
					fd.setText("Open");
					fd.setFilterPath(config.map.getPath());
					fd.setFileName(config.map.getName());
					fd.setFilterExtensions(new String[] {"*.*"});
					String mapFileString = fd.open();
					if (mapFileString != null) {
						config.map = new File(mapFileString);
					}
					generalUpdate();
				}
			});
		}
		
		// graphical
		gui = new Button(currentPage, SWT.CHECK);
		gui.setText("Use GUI");
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = SWT.BEGINNING;
			gd.horizontalSpan = 3;
			gui.setLayoutData(gd);
		}

		// world display
		hide = new Button(currentPage, SWT.CHECK);
		hide.setText("Hide world");
		hide.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.noWorld = !config.noWorld;
				generalUpdate();
			}
		});
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = SWT.BEGINNING;
			gd.horizontalSpan = 3;
			hide.setLayoutData(gd);
		}

		// random seed
		useSeed = new Button(currentPage, SWT.CHECK);
		useSeed.setText("Use random seed");
		useSeed.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.random = !config.random;
				generalUpdate();
			}
		});
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = SWT.BEGINNING;
			gd.horizontalSpan = 3;
			useSeed.setLayoutData(gd);
		}
			
		Label seedLabel = new Label(currentPage, SWT.NONE);
		seedLabel.setText("Random seed:");
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = GridData.BEGINNING;
			gd.horizontalIndent = 20;
			seedLabel.setLayoutData(gd);
		}

		seedText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		seedText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int newSeed;
				try {
					newSeed = Integer.parseInt(seedText.getText());
				} catch (NumberFormatException exception) {
					newSeed = Soar2D.config.randomSeed;
				}
				config.randomSeed = newSeed;
				generalUpdate();
			}
		});
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = GridData.BEGINNING;
			gd.horizontalSpan = 2;
			gd.widthHint = 100;
			seedText.setLayoutData(gd);
		}
		
		// remote kernel
		remote = new Button(currentPage, SWT.CHECK);
		remote.setText("Remote kernel");
		remote.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.remote = !config.remote;
				generalUpdate();
			}
		});
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = SWT.BEGINNING;
			gd.horizontalSpan = 3;
			remote.setLayoutData(gd);
		}

		generalUpdate();
		
		rhs.layout(true);
		dialog.layout(true);
	}
	
	public void generalUpdate() {
		switch (config.getType()) {
		case kEaters:
			tanksoarButton.setSelection(false);
			eatersButton.setSelection(true);
			break;
		case kTankSoar:
			tanksoarButton.setSelection(true);
			eatersButton.setSelection(false);
			break;
		}
		mapText.setText(config.map.getAbsolutePath());
		gui.setSelection(config.graphical);
		hide.setSelection(config.noWorld);
		useSeed.setSelection(!config.random);
		seedText.setEnabled(useSeed.getSelection());
		seedText.setText(Integer.toString(config.randomSeed));
		remote.setSelection(config.remote);
	}

	public void loggingPage() {
		// log level
		// log to file and/or console
		// log filename
		// log time or not
		if (currentPage != null) {
			currentPage.dispose();
		}
		currentPage = new Composite(rhs, SWT.NONE);
		{
			GridLayout gl = new GridLayout();
			gl.marginHeight = 0;
			gl.marginWidth = 0;
			gl.numColumns = 1;
			currentPage.setLayout(gl);
			
			GridData gd = new GridData();
			gd.grabExcessHorizontalSpace = true;
			gd.grabExcessVerticalSpace = true;
			gd.horizontalAlignment = SWT.FILL;
			gd.verticalAlignment = SWT.FILL;
			currentPage.setLayoutData(gd);
		}
		
		// targets
		{
			Group targetsGroup = new Group(currentPage, SWT.NONE);
			targetsGroup.setText("Log targets");
			{
				GridData gd = new GridData();
				gd.grabExcessHorizontalSpace = true;
				gd.horizontalAlignment = SWT.FILL;
				gd.verticalAlignment = SWT.TOP;
				targetsGroup.setLayoutData(gd);
				
				GridLayout gl = new GridLayout();
				gl.numColumns = 2;
				targetsGroup.setLayout(gl);
			}
			
			loggingConsoleButton = new Button(targetsGroup, SWT.CHECK);
			loggingConsoleButton.setText("Console");
			loggingConsoleButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					config.logConsole = !config.logConsole;
					loggingUpdate();
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = SWT.BEGINNING;
				gd.horizontalSpan = 2;
				loggingConsoleButton.setLayoutData(gd);
			}

			loggingFileButton = new Button(targetsGroup, SWT.CHECK);
			loggingFileButton.setText("File");
			loggingFileButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					config.logToFile = !config.logToFile;
					loggingUpdate();
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = SWT.BEGINNING;
				gd.horizontalSpan = 2;
				loggingFileButton.setLayoutData(gd);
			}
			
			Label loggingNameLabel = new Label(targetsGroup, SWT.NONE);
			loggingNameLabel.setText("Log file:");
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.BEGINNING;
				gd.horizontalIndent = 20;
				gd.horizontalSpan = 2;
				loggingNameLabel.setLayoutData(gd);
			}

			loggingNameText = new Text(targetsGroup, SWT.SINGLE | SWT.BORDER);
			loggingNameText.addFocusListener(new FocusAdapter() {
				public void focusLost(FocusEvent e) {
					if (loggingNameText.getText() != null) {
						config.logFile = new File(loggingNameText.getText());
					}
					loggingUpdate();
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.FILL;
				gd.grabExcessHorizontalSpace = true;
				gd.horizontalIndent = 20;
				loggingNameText.setLayoutData(gd);
			}
			
			Button loggingBrowse = new Button(targetsGroup, SWT.PUSH);
			loggingBrowse.setText("Browse...");
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.END;
				loggingBrowse.setLayoutData(gd);
			}
			loggingBrowse.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					FileDialog fd = new FileDialog(dialog, SWT.OPEN);
					fd.setText("Choose");
					fd.setFilterPath(config.getBasePath());
					fd.setFileName(config.logFile.getName());
					fd.setFilterExtensions(new String[] {"*.*"});
					String logFileString = fd.open();
					if (logFileString != null) {
						config.logFile = new File(logFileString);
					}
					loggingUpdate();
				}
			});
		}
		
		// log time
		loggingTimeButton = new Button(currentPage, SWT.CHECK);
		loggingTimeButton.setText("Print a time stamp with each message");
		loggingTimeButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.logTime = !config.logTime;
				loggingUpdate();
			}
		});
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = SWT.BEGINNING;
			loggingTimeButton.setLayoutData(gd);
		}

		loggingUpdate();

		rhs.layout(true);
		dialog.layout(true);
	}
	
	public void loggingUpdate() {
		loggingFileButton.setSelection(config.logToFile);
		loggingNameText.setEnabled(config.logToFile);
		loggingNameText.setText(config.logFile.getAbsolutePath());
		loggingConsoleButton.setSelection(config.logConsole);
	}
	
	public void agentsPage(final TreeItem selectedItem, int selectedIndex) {
		
		// lists all intended agents
		// create new agent, setting its properties
		// for each agent: create shutdown command
		// global spawn debugger setting
		if (currentPage != null) {
			currentPage.dispose();
		}
		
		currentPage = new Composite(rhs, SWT.NONE);
		{
			GridLayout gl = new GridLayout();
			gl.marginHeight = 0;
			gl.marginWidth = 0;
			gl.numColumns = 1;
			currentPage.setLayout(gl);
			
			GridData gd = new GridData();
			gd.grabExcessHorizontalSpace = true;
			gd.grabExcessVerticalSpace = true;
			gd.horizontalAlignment = SWT.FILL;
			gd.verticalAlignment = SWT.FILL;
			currentPage.setLayoutData(gd);
		}
		
		if (selectedItem == null) {
			playerConfig = new PlayerConfig();
			playerConfigIndex = -1;
			
			// spawn debuggers
			agentsDebuggersButton = new Button(currentPage, SWT.CHECK);
			agentsDebuggersButton.setText("Spawn debuggers on agent creation");
			agentsDebuggersButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					config.debuggers = !config.debuggers;
					agentsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = SWT.BEGINNING;
				agentsDebuggersButton.setLayoutData(gd);
			}
		} else {
			playerConfigIndex = selectedIndex;
			playerConfig = config.players.get(playerConfigIndex);
		}
		
		{
			Group newAgentGroup = new Group(currentPage, SWT.NONE);
			if (selectedItem == null) {
				newAgentGroup.setText("Create a new default agent");
			} else {
				newAgentGroup.setText("Agent settings");
			}
			{
				GridData gd = new GridData();
				gd.grabExcessHorizontalSpace = true;
				gd.horizontalAlignment = SWT.FILL;
				gd.verticalAlignment = SWT.TOP;
				newAgentGroup.setLayoutData(gd);
				
				GridLayout gl = new GridLayout();
				gl.numColumns = 5;
				newAgentGroup.setLayout(gl);
			}
			
			agentsNameButton = new Button(newAgentGroup, SWT.CHECK);
			agentsNameButton.setText("Specify name (default is color):");
			agentsNameButton.setSelection(playerConfig.hasName());
			agentsNameButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					if (!agentsNameButton.getSelection()) {
						playerConfig.setName(null);
						if (selectedItem != null) {
							String name = "<unnamed>";
							selectedItem.setText(name);
						}
					}
					agentsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalSpan = 5;
				agentsNameButton.setLayoutData(gd);
			}
			
			agentsNameText = new Text(newAgentGroup, SWT.SINGLE | SWT.BORDER);
			if (playerConfig.hasName()) {
				agentsNameText.setText(playerConfig.getName());
			}
			agentsNameText.addFocusListener(new FocusAdapter() {
				public void focusLost(FocusEvent e) {
					playerConfig.setName(agentsNameText.getText());
					if (selectedItem != null) {
						String name = playerConfig.getName();
						if (name == null || name.length() < 1) {
							name = "<unnamed>";
						}
						selectedItem.setText(name);
					}
					agentsUpdate(selectedItem);
				}
			});
			agentsNameText.addKeyListener(new KeyAdapter() {
				public void keyPressed(KeyEvent e) {
					if (Character.isWhitespace(e.character)) {
						e.doit = false;
					}
				}
				public void keyReleased(KeyEvent e) {
					agentsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.FILL;
				gd.horizontalIndent = 20;
				gd.grabExcessHorizontalSpace = true;
				gd.horizontalSpan = 5;
				agentsNameText.setLayoutData(gd);
			}
			
			agentsProductions = new Button(newAgentGroup, SWT.CHECK);
			agentsProductions.setText("Specify productions (leave unspecified for human control):");
			agentsProductions.setSelection(playerConfig.hasProductions());
			agentsProductions.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					if (!agentsProductions.getSelection()) {
						playerConfig.setProductions(null);
					}
					agentsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalSpan = 5;
				agentsProductions.setLayoutData(gd);
			}
			
			agentsProductionsText = new Text(newAgentGroup, SWT.SINGLE | SWT.BORDER);
			agentsProductionsText.addFocusListener(new FocusAdapter() {
				public void focusLost(FocusEvent e) {
					File productionsFile = null;
					if (agentsProductionsText.getText() != null) {
						if (agentsProductionsText.getText().length() > 0) {
							productionsFile = new File(agentsProductionsText.getText());
						}
					}
					playerConfig.setProductions(productionsFile);
					
					agentsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.FILL;
				gd.grabExcessHorizontalSpace = true;
				gd.horizontalIndent = 20;
				gd.horizontalSpan = 4;
				agentsProductionsText.setLayoutData(gd);
			}
			
			productionsBrowse = new Button(newAgentGroup, SWT.PUSH);
			productionsBrowse.setText("Browse...");
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.END;
				productionsBrowse.setLayoutData(gd);
			}
			productionsBrowse.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					FileDialog fd = new FileDialog(dialog, SWT.OPEN);
					fd.setText("Choose");
					fd.setFilterPath(config.getAgentPath());
					fd.setFileName(config.logFile.getName());
					fd.setFilterExtensions(new String[] {"*.soar", "*.*"});
					String productionsString = fd.open();
					File productionsFile = null;
					if (productionsString != null) {
						productionsFile = new File(productionsString);
					}
					playerConfig.setProductions(productionsFile);
					
					agentsUpdate(selectedItem);
				}
			});
			
			agentsColor = new Button(newAgentGroup, SWT.CHECK);
			agentsColor.setText("Specify color (default is random):");
			agentsColor.setSelection(playerConfig.hasColor());
			agentsColor.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					if (!agentsColor.getSelection()) {
						playerConfig.setColor(null);
					}
					agentsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalSpan = 5;
				agentsColor.setLayoutData(gd);
			}
			
			agentsColorCombo = new Combo(newAgentGroup, SWT.READ_ONLY);
			{
				GridData gd = new GridData();
				gd.horizontalSpan = 5;
				gd.horizontalIndent = 20;
				gd.horizontalAlignment = GridData.FILL;
				gd.grabExcessHorizontalSpace = true;
				agentsColorCombo.setLayoutData(gd);
			}
			agentsColorCombo.setItems(Soar2D.simulation.kColors);
			agentsColorCombo.setVisibleItemCount(Soar2D.simulation.kColors.length);
			// TODO: only make unused colors available
			if (playerConfig.hasColor()) {
				int index;
				for (index = 0; index < Soar2D.simulation.kColors.length; ++index) {
					if (Soar2D.simulation.kColors[index].equals(playerConfig.getColor())) {
						break;
					}
				}
				assert index < Soar2D.simulation.kColors.length;
				agentsColorCombo.select(index);
			} else {
				agentsColorCombo.select(0);
			}
			agentsColorCombo.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					playerConfig.setColor(Soar2D.simulation.kColors[agentsColorCombo.getSelectionIndex()]);
					
					agentsUpdate(selectedItem);
				}
			});
			
			agentCoordinates = new Button(newAgentGroup, SWT.CHECK);
			agentCoordinates.setText("Specify coordinates (default is random):");
			agentCoordinates.setSelection(playerConfig.hasInitialLocation());
			agentCoordinates.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					if (!agentCoordinates.getSelection()) {
						playerConfig.setInitialLocation(null);
					}
					agentsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalSpan = 5;
				agentCoordinates.setLayoutData(gd);
			}
			
			Label xLabel = new Label(newAgentGroup, SWT.NONE);
			xLabel.setText("X:");
			{
				GridData gd = new GridData();
				gd.horizontalIndent = 20;
				xLabel.setLayoutData(gd);
			}
			
			agentCoordinatesX = new Text(newAgentGroup, SWT.SINGLE | SWT.BORDER);
			agentCoordinatesX.addFocusListener(new FocusAdapter() {
				public void focusLost(FocusEvent e) {
					java.awt.Point il;
					if (playerConfig.hasInitialLocation()) {
						il = playerConfig.getInitialLocation();
					} else {
						il = new java.awt.Point(-1, -1);
					}
					try {
						il.x = Integer.parseInt(agentCoordinatesX.getText());
					} catch (NumberFormatException exception) {
						il.x = -1;
					}
					playerConfig.setInitialLocation(il);
					
					agentsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				gd.widthHint = 20;
				agentCoordinatesX.setLayoutData(gd);
			}
			
			Label yLabel = new Label(newAgentGroup, SWT.NONE);
			yLabel.setText("Y:");
			{
				GridData gd = new GridData();
				yLabel.setLayoutData(gd);
			}

			agentCoordinatesY = new Text(newAgentGroup, SWT.SINGLE | SWT.BORDER);
			agentCoordinatesY.addFocusListener(new FocusAdapter() {
				public void focusLost(FocusEvent e) {
					java.awt.Point il;
					if (playerConfig.hasInitialLocation()) {
						il = playerConfig.getInitialLocation();
					} else {
						il = new java.awt.Point(-1, -1);
					}
					try {
						il.y = Integer.parseInt(agentCoordinatesY.getText());
					} catch (NumberFormatException exception) {
						il.y = -1;
					}
					playerConfig.setInitialLocation(il);

					agentsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
					gd.widthHint = 20;
				gd.horizontalSpan = 2;
				agentCoordinatesY.setLayoutData(gd);
			}

			Composite nittyGritty = new Composite(newAgentGroup, SWT.NONE);
			{
				GridLayout gl = new GridLayout();
				gl.marginHeight = 0;
				gl.marginWidth = 0;
				gl.numColumns = 2;
				nittyGritty.setLayout(gl);
				
				GridData gd = new GridData();
				gd.grabExcessHorizontalSpace = true;
				gd.grabExcessVerticalSpace = true;
				gd.horizontalSpan = 5;
				gd.horizontalAlignment = SWT.FILL;
				gd.verticalAlignment = SWT.FILL;
				nittyGritty.setLayoutData(gd);
			}
			
			agentFacing = new Button(nittyGritty, SWT.CHECK);
			agentFacing.setText("Specify initial facing (default is random):");
			agentFacing.setSelection(playerConfig.hasFacing());
			agentFacing.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					if (!agentFacing.getSelection()) {
						playerConfig.setFacing(0);
					}
					agentsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				agentFacing.setLayoutData(gd);
			}
			
			agentFacingCombo = new Combo(nittyGritty, SWT.READ_ONLY);
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.FILL;
				gd.grabExcessHorizontalSpace = true;
				agentFacingCombo.setLayoutData(gd);
			}
			agentFacingCombo.setItems(new String [] {
					Direction.stringOf[Direction.kNorthInt], 
					Direction.stringOf[Direction.kEastInt], 
					Direction.stringOf[Direction.kSouthInt], 
					Direction.stringOf[Direction.kWestInt]});
			agentFacingCombo.setVisibleItemCount(4);
			agentFacingCombo.select(0);
			agentFacingCombo.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					playerConfig.setFacing(Direction.getInt(agentFacingCombo.getText()));
					agentsUpdate(selectedItem);
				}
			});
			
			agentPoints = new Button(nittyGritty, SWT.CHECK);
			agentPoints.setText("Specify initial points:");
			agentPoints.setSelection(playerConfig.hasPoints());
			agentPoints.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					if (!agentPoints.getSelection()) {
						playerConfig.setPoints(false);
					}
					agentsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				agentPoints.setLayoutData(gd);
			}

			agentPointsText = new Text(nittyGritty, SWT.SINGLE | SWT.BORDER);
			agentPointsText.addFocusListener(new FocusAdapter() {
				public void focusLost(FocusEvent e) {
					try {
						int newPoints = Integer.parseInt(agentPointsText.getText());
						playerConfig.setPoints(newPoints);
						playerConfig.setPoints(true);
					} catch (NumberFormatException exception) {}
					
					agentsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.FILL;
				gd.grabExcessHorizontalSpace = true;
				agentPointsText.setLayoutData(gd);
			}
			
			agentHealth = new Button(nittyGritty, SWT.CHECK);
			agentHealth.setText("Specify initial health (TankSoar only):");
			agentHealth.setSelection(playerConfig.hasHealth());
			agentHealth.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					if (!agentHealth.getSelection()) {
						playerConfig.setHealth(-1);
					}
					agentsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				agentHealth.setLayoutData(gd);
			}
			
			agentHealthText = new Text(nittyGritty, SWT.SINGLE | SWT.BORDER);
			agentHealthText.addFocusListener(new FocusAdapter() {
				public void focusLost(FocusEvent e) {
					try {
						int newHealth = Integer.parseInt(agentHealthText.getText());
						playerConfig.setHealth(newHealth);
					} catch (NumberFormatException exception) {}
					
					agentsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.FILL;
				gd.grabExcessHorizontalSpace = true;
				agentHealthText.setLayoutData(gd);
			}
			
			agentEnergy = new Button(nittyGritty, SWT.CHECK);
			agentEnergy.setText("Specify initial energy (TankSoar only):");
			agentEnergy.setSelection(playerConfig.hasEnergy());
			agentEnergy.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					if (!agentEnergy.getSelection()) {
						playerConfig.setEnergy(-1);
					}
					agentsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				agentEnergy.setLayoutData(gd);
			}
			
			agentEnergyText = new Text(nittyGritty, SWT.SINGLE | SWT.BORDER);
			agentEnergyText.addFocusListener(new FocusAdapter() {
				public void focusLost(FocusEvent e) {
					try {
						int newEnergy = Integer.parseInt(agentEnergyText.getText());
						playerConfig.setEnergy(newEnergy);
					} catch (NumberFormatException exception) {}
					
					agentsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.FILL;
				gd.grabExcessHorizontalSpace = true;
				agentEnergyText.setLayoutData(gd);
			}
			
			agentMissiles = new Button(nittyGritty, SWT.CHECK);
			agentMissiles.setText("Specify initial missiles (TankSoar only):");
			agentMissiles.setSelection(playerConfig.hasMissiles());
			agentMissiles.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					if (!agentMissiles.getSelection()) {
						playerConfig.setMissiles(-1);
					}
					agentsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				agentMissiles.setLayoutData(gd);
			}
			
			agentMissilesText = new Text(nittyGritty, SWT.SINGLE | SWT.BORDER);
			agentMissilesText.addFocusListener(new FocusAdapter() {
				public void focusLost(FocusEvent e) {
					try {
						int newMissiles = Integer.parseInt(agentMissilesText.getText());
						playerConfig.setMissiles(newMissiles);
					} catch (NumberFormatException exception) {}
					
					agentsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.FILL;
				gd.grabExcessHorizontalSpace = true;
				agentMissilesText.setLayoutData(gd);
			}
			
			if (selectedItem == null) {
				createAgentButton = new Button(newAgentGroup, SWT.PUSH);
				createAgentButton.setText("Create new agent");
				createAgentButton.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						config.players.add(new PlayerConfig(playerConfig));
						TreeItem agents = tree.getItem(2);
						TreeItem newAgent = new TreeItem(agents, SWT.NONE);
						String name;
						if (playerConfig.hasName()) {
							name = playerConfig.getName();
						} else {
							name = "<unnamed>";
						}
						newAgent.setText(name);
						tree.redraw();
						agentsUpdate(selectedItem);
					}
				});
			{
					GridData gd = new GridData();
					gd.horizontalAlignment = GridData.END;
					gd.horizontalSpan = 5;
					createAgentButton.setLayoutData(gd);
				}
			} else {
				createAgentButton = null;
			}
		}
		
		if (selectedItem != null) {
			removeAgentButton = new Button(currentPage, SWT.PUSH);
			removeAgentButton.setText("Remove this agent");
			removeAgentButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					config.players.remove(playerConfigIndex);
					playerConfigIndex = -1;
					playerConfig = null;
					selectedItem.dispose();
					tree.redraw();
					TreeItem newSelection = tree.getItem(2);
					tree.setSelection(newSelection);
					agentsPage(null, -1);
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.END;
				gd.horizontalSpan = 5;
				removeAgentButton.setLayoutData(gd);
			}
		}

		agentsUpdate(selectedItem);

		rhs.layout(true);
		dialog.layout(true);
	}
	
	public void agentsUpdate(TreeItem selectedItem) {
		
		boolean createReady = true;
		boolean allDisabled = false;
		
		if ((selectedItem == null) 
				&& (config.players.size() >= Soar2D.simulation.kColors.length)) {
			allDisabled = true;
			agentsNameButton.setEnabled(false);
			agentsProductions.setEnabled(false);
			agentsColor.setEnabled(false);
			agentCoordinates.setEnabled(false);
			agentFacing.setEnabled(false);
			agentPoints.setEnabled(false);
			agentHealth.setEnabled(false);
			agentEnergy.setEnabled(false);
			agentMissiles.setEnabled(false);
		} else {
			agentsNameButton.setEnabled(true);
			agentsProductions.setEnabled(true);
			agentsColor.setEnabled(true);
			agentCoordinates.setEnabled(true);
			agentFacing.setEnabled(true);
			agentPoints.setEnabled(true);
			agentHealth.setEnabled(true);
			agentEnergy.setEnabled(true);
			agentMissiles.setEnabled(true);
		}
		
		if (agentsNameButton.getSelection() && !allDisabled) {
			agentsNameText.setEnabled(true);
			if (agentsNameText.getText().length() < 1) {
				createReady = false;
			}
		} else {
			agentsNameText.setEnabled(false);
			agentsNameText.setText("");
		}
		
		if (agentsProductions.getSelection() && !allDisabled) {
			agentsProductionsText.setEnabled(true);
			productionsBrowse.setEnabled(true);
			if (playerConfig.getProductions() == null) {
				createReady = false;
				agentsProductionsText.setText("");
			} else {
				agentsProductionsText.setText(playerConfig.getProductions().getAbsolutePath());
			}
		} else {
			agentsProductionsText.setEnabled(false);
			productionsBrowse.setEnabled(false);
			agentsProductionsText.setText("");
		}

		agentsColorCombo.setEnabled(agentsColor.getSelection() && !allDisabled);

		if (agentCoordinates.getSelection() && !allDisabled) {
			agentCoordinatesX.setEnabled(true);
			agentCoordinatesY.setEnabled(true);
			
			java.awt.Point loc = playerConfig.getInitialLocation();
			if (loc == null) {
				agentCoordinatesX.setText("-1");
				agentCoordinatesY.setText("-1");
			} else {
				agentCoordinatesX.setText(Integer.toString(loc.x));
				agentCoordinatesY.setText(Integer.toString(loc.y));
			}
			
		} else {
			agentCoordinatesX.setEnabled(false);
			agentCoordinatesY.setEnabled(false);
			agentCoordinatesX.setText("");
			agentCoordinatesY.setText("");
		}

		agentFacingCombo.setEnabled(agentFacing.getSelection() && !allDisabled);

		if (agentPoints.getSelection() && !allDisabled) {
			agentPointsText.setEnabled(true);
			agentPointsText.setText(Integer.toString(playerConfig.getPoints()));
		} else {
			agentPointsText.setEnabled(false);
			agentPointsText.setText("");
		}

		if (agentHealth.getSelection() && !allDisabled) {
			agentHealthText.setEnabled(true);
			agentHealthText.setText(Integer.toString(playerConfig.getHealth()));
		} else {
			agentHealthText.setEnabled(false);
			agentHealthText.setText("");
		}

		if (agentEnergy.getSelection() && !allDisabled) {
			agentEnergyText.setEnabled(true);
			agentEnergyText.setText(Integer.toString(playerConfig.getEnergy()));
		} else {
			agentEnergyText.setEnabled(false);
			agentEnergyText.setText("");
		}

		if (agentMissiles.getSelection() && !allDisabled) {
			agentMissilesText.setEnabled(true);
			agentMissilesText.setText(Integer.toString(playerConfig.getMissiles()));
		} else {
			agentMissilesText.setEnabled(false);
			agentMissilesText.setText("");
		}
		
		if (createAgentButton != null) {	
			createAgentButton.setEnabled(createReady && !allDisabled);
		}
	}
	
	public void terminalsPage() {
		if (currentPage != null) {
			currentPage.dispose();
		}
		currentPage = new Composite(rhs, SWT.NONE);
		{
			GridLayout gl = new GridLayout();
			gl.marginHeight = 0;
			gl.marginWidth = 0;
			gl.numColumns = 1;
			currentPage.setLayout(gl);
			
			GridData gd = new GridData();
			gd.grabExcessHorizontalSpace = true;
			gd.grabExcessVerticalSpace = true;
			gd.horizontalAlignment = SWT.FILL;
			gd.verticalAlignment = SWT.FILL;
			currentPage.setLayoutData(gd);
		}
		
		terminalMaxUpdates = new Button(currentPage, SWT.CHECK);
		terminalMaxUpdates.setText("Stop on update count");
		terminalMaxUpdates.setSelection(config.terminalMaxUpdates > 0);
		terminalMaxUpdates.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				terminalsUpdate();
			}
		});
		{
			GridData gd = new GridData();
			terminalMaxUpdates.setLayoutData(gd);
		}
		
		// max updates requires a number of updates
		maxUpdates = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		maxUpdates.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				try {
					config.terminalMaxUpdates = Integer.parseInt(maxUpdates.getText());
				} catch (NumberFormatException exeption) {}
				terminalsUpdate();
			}
		});
		maxUpdates.addKeyListener(new KeyAdapter() {
			public void keyPressed(KeyEvent e) {
				if (!Character.isDigit(e.character)) {
					e.doit = false;
				}
			}
			public void keyReleased(KeyEvent e) {
				terminalsUpdate();
			}
		});
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = GridData.FILL;
			gd.horizontalIndent = 20;
			gd.grabExcessHorizontalSpace = true;
			maxUpdates.setLayoutData(gd);
		}

		terminalAgentCommand = new Button(currentPage, SWT.CHECK);
		terminalAgentCommand.setText("Stop on agent command");
		terminalAgentCommand.setSelection(config.terminalAgentCommand);
		terminalAgentCommand.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.terminalAgentCommand = terminalAgentCommand.getSelection();
			}
		});
		{
			GridData gd = new GridData();
			terminalAgentCommand.setLayoutData(gd);
		}

		terminalWinningScore = new Button(currentPage, SWT.CHECK);
		terminalWinningScore.setText("Stop when a score is achieved");
		terminalWinningScore.setSelection(config.terminalWinningScore > 0);
		terminalWinningScore.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				terminalsUpdate();
			}
		});
		{
			GridData gd = new GridData();
			terminalWinningScore.setLayoutData(gd);
		}

		// winning score requires a score
		winningScore = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		winningScore.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				try {
					config.terminalWinningScore = Integer.parseInt(winningScore.getText());
				} catch (NumberFormatException exeption) {}
				terminalsUpdate();
			}
		});
		winningScore.addKeyListener(new KeyAdapter() {
			public void keyPressed(KeyEvent e) {
				if (!Character.isDigit(e.character)) {
					e.doit = false;
				}
			}
			public void keyReleased(KeyEvent e) {
				terminalsUpdate();
			}
		});
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = GridData.FILL;
			gd.horizontalIndent = 20;
			gd.grabExcessHorizontalSpace = true;
			winningScore.setLayoutData(gd);
		}

		terminalPointsRemaining = new Button(currentPage, SWT.CHECK);
		terminalPointsRemaining.setText("Stop when there are no more points available (Eaters)");
		terminalPointsRemaining.setSelection(config.terminalPointsRemaining);
		terminalPointsRemaining.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.terminalPointsRemaining = terminalPointsRemaining.getSelection();
			}
		});
		{
			GridData gd = new GridData();
			terminalPointsRemaining.setLayoutData(gd);
		}

		terminalFoodRemaining = new Button(currentPage, SWT.CHECK);
		terminalFoodRemaining.setText("Stop when there is no food remaining (Eaters)");
		terminalFoodRemaining.setSelection(config.terminalFoodRemaining);
		terminalFoodRemaining.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.terminalFoodRemaining = terminalFoodRemaining.getSelection();
			}
		});
		{
			GridData gd = new GridData();
			terminalFoodRemaining.setLayoutData(gd);
		}

		terminalUnopenedBoxes = new Button(currentPage, SWT.CHECK);
		terminalUnopenedBoxes.setText("Stop when there are no unopened boxes (Eaters)");
		terminalUnopenedBoxes.setSelection(config.terminalUnopenedBoxes);
		terminalUnopenedBoxes.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.terminalUnopenedBoxes = terminalUnopenedBoxes.getSelection();
			}
		});
		{
			GridData gd = new GridData();
			terminalUnopenedBoxes.setLayoutData(gd);
		}

		terminalsUpdate();
		
		rhs.layout(true);
		dialog.layout(true);
	}
	
	public void terminalsUpdate() {
		if (terminalMaxUpdates.getSelection()) {
			maxUpdates.setEnabled(true);
			maxUpdates.setText(Integer.toString(config.terminalMaxUpdates));
		} else {
			maxUpdates.setEnabled(false);
			maxUpdates.setText("");
		}
		
		if (terminalWinningScore.getSelection()) {
			winningScore.setEnabled(true);
			winningScore.setText(Integer.toString(config.terminalWinningScore));
		} else {
			winningScore.setEnabled(false);
			winningScore.setText("");
		}
	}
	
	public void clientsPage(final TreeItem selectedItem, int selectedIndex) {
		// lists all intended clients
		// create new clients, setting its properties

		if (currentPage != null) {
			currentPage.dispose();
		}
		
		currentPage = new Composite(rhs, SWT.NONE);
		{
			GridLayout gl = new GridLayout();
			gl.marginHeight = 0;
			gl.marginWidth = 0;
			gl.numColumns = 1;
			currentPage.setLayout(gl);
			
			GridData gd = new GridData();
			gd.grabExcessHorizontalSpace = true;
			gd.grabExcessVerticalSpace = true;
			gd.horizontalAlignment = SWT.FILL;
			gd.verticalAlignment = SWT.FILL;
			currentPage.setLayoutData(gd);
		}
		
		if (selectedItem == null) {
			clientConfigIndex = -1;
			clientConfig = new ClientConfig();
			
		} else {
			clientConfigIndex = selectedIndex;
			clientConfig = config.clients.get(clientConfigIndex);
		}
		
		Label notice = new Label(currentPage, SWT.NONE);
		notice.setText("Note: debugger clients are handled in agent configuration.");
		{
			GridData gd = new GridData();
			notice.setLayoutData(gd);
		}
		
		{
			Group newClientGroup = new Group(currentPage, SWT.NONE);
			if (selectedItem == null) {
				newClientGroup.setText("Create a new client");
			} else {
				newClientGroup.setText("Client settings");
			}
			{
				GridData gd = new GridData();
				gd.grabExcessHorizontalSpace = true;
				gd.horizontalAlignment = SWT.FILL;
				gd.verticalAlignment = SWT.TOP;
				newClientGroup.setLayoutData(gd);
				
				GridLayout gl = new GridLayout();
				gl.numColumns = 1;
				newClientGroup.setLayout(gl);
			}
			
			Label clientNameLabel = new Label(newClientGroup, SWT.NONE);
			clientNameLabel.setText("Client name:");
			{
				GridData gd = new GridData();
				clientNameLabel.setLayoutData(gd);
			}
		
			clientName = new Text(newClientGroup, SWT.SINGLE | SWT.BORDER);
			clientName.setText(clientConfig.name);
			clientName.addFocusListener(new FocusAdapter() {
				public void focusLost(FocusEvent e) {
					clientConfig.name = clientName.getText();
					if (selectedItem != null) {
						String name = clientConfig.name;
						selectedItem.setText(name);
					}
					clientsUpdate(selectedItem);
				}
			});
			clientName.addKeyListener(new KeyAdapter() {
				public void keyPressed(KeyEvent e) {
					if (Character.isWhitespace(e.character)) {
						e.doit = false;
					}
				}
				public void keyReleased(KeyEvent e) {
					clientsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.FILL;
				gd.grabExcessHorizontalSpace = true;
				clientName.setLayoutData(gd);
			}
			
			clientCommandButton = new Button(newClientGroup, SWT.CHECK);
			clientCommandButton.setText("Soar2D launches client:");
			clientCommandButton.setSelection(clientConfig.command != null);
			clientCommandButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					if (!clientCommandButton.getSelection()) {
						clientConfig.command = null;
					}
					
					clientsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				clientCommandButton.setLayoutData(gd);
			}
			
			Label clientCommandLabel = new Label(newClientGroup, SWT.NONE);
			clientCommandLabel.setText("Command:");
			{
				GridData gd = new GridData();
				clientCommandLabel.setLayoutData(gd);
			}
		
			clientCommand = new Text(newClientGroup, SWT.SINGLE | SWT.BORDER);
			if (clientConfig.command != null) {
				clientCommand.setText(clientConfig.command);
			}
			clientCommand.addFocusListener(new FocusAdapter() {
				public void focusLost(FocusEvent e) {
					clientConfig.command = clientCommand.getText();
					if (clientConfig.command.length() < 1) {
						clientConfig.command = null;
					}
					
					clientsUpdate(selectedItem);
				}
			});
			clientCommand.addKeyListener(new KeyAdapter() {
				public void keyReleased(KeyEvent e) {
					clientsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.FILL;
				gd.horizontalIndent = 20;
				gd.grabExcessHorizontalSpace = true;
				clientCommand.setLayoutData(gd);
			}
			
			clientTimeoutButton = new Button(newClientGroup, SWT.CHECK);
			clientTimeoutButton.setText("Do not use default timeout:");
			clientTimeoutButton.setSelection(clientConfig.timeout != Soar2D.config.kDefaultTimeout);
			clientTimeoutButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					if (!clientCommandButton.getSelection()) {
						clientConfig.timeout = Soar2D.config.kDefaultTimeout;
					}
					clientsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				clientTimeoutButton.setLayoutData(gd);
			}
			
			Label clientTimeoutLabel = new Label(newClientGroup, SWT.NONE);
			clientTimeoutLabel.setText("Timeout (seconds):");
			{
				GridData gd = new GridData();
				clientTimeoutLabel.setLayoutData(gd);
			}
		
			clientTimeout = new Text(newClientGroup, SWT.SINGLE | SWT.BORDER);
			clientTimeout.addFocusListener(new FocusAdapter() {
				public void focusLost(FocusEvent e) {
					try {
						clientConfig.timeout = Integer.parseInt(clientTimeout.getText());
					} catch (NumberFormatException exception) {}
					
					clientsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.FILL;
				gd.horizontalIndent = 20;
				gd.grabExcessHorizontalSpace = true;
				clientTimeout.setLayoutData(gd);
			}
			
			clientAfter = new Button(newClientGroup, SWT.CHECK);
			clientAfter.setText("Spawn client after agent creation (unchecked means spawn before)");
			clientAfter.setSelection(clientConfig.after);
			clientAfter.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					clientConfig.after = clientAfter.getSelection();

					clientsUpdate(selectedItem);
				}
			});
			{
				GridData gd = new GridData();
				clientAfter.setLayoutData(gd);
			}
			
			if (selectedItem == null) {
				createClientButton = new Button(newClientGroup, SWT.PUSH);
				createClientButton.setText("Create new client");
				createClientButton.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						config.clients.add(new ClientConfig(clientConfig));
						TreeItem clients = tree.getItem(4);
						TreeItem newClient = new TreeItem(clients, SWT.NONE);
						String name = clientConfig.name;
						newClient.setText(name);
						tree.redraw();
						clientsUpdate(selectedItem);
					}
				});
				{
					GridData gd = new GridData();
					gd.horizontalAlignment = GridData.END;
					createClientButton.setLayoutData(gd);
				}
			} else {
				createClientButton = null;
			}
		}
		
		if (selectedItem != null) {
			removeClientButton = new Button(currentPage, SWT.PUSH);
			removeClientButton.setText("Remove this Client");
			removeClientButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					config.clients.remove(clientConfigIndex);
					clientConfigIndex = -1;
					clientConfig = null;
					selectedItem.dispose();
					tree.redraw();
					TreeItem newSelection = tree.getItem(4);
					tree.setSelection(newSelection);
					clientsPage(null, -1);
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.END;
				removeClientButton.setLayoutData(gd);
			}
		}

		clientsUpdate(selectedItem);

		rhs.layout(true);
		dialog.layout(true);
	}
	
	public void clientsUpdate(TreeItem selectedItem) {
		
		boolean createReady = true;
		
		if (clientName.getText().length() < 1) {
			createReady = false;
		}
		
		if (clientCommandButton.getSelection()) {
			clientCommand.setEnabled(true);
			if (clientCommand.getText().length() < 1) {
				createReady = false;
			}
		} else {
			clientCommand.setEnabled(false);
			clientCommand.setText("");
		}
		
		if (clientTimeoutButton.getSelection()) {
			clientTimeout.setEnabled(true);
			clientTimeout.setText(Integer.toString(clientConfig.timeout));
		} else {
			clientTimeout.setEnabled(false);
			clientTimeout.setText("");
		}
		
		if (createClientButton != null) {	
			createClientButton.setEnabled(createReady);
		}
		
	}
}
