package soar2d.visuals;

import java.io.*;
import java.util.*;
import java.util.logging.*;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import soar2d.*;
import soar2d.configuration.Configuration;
import soar2d.configuration.Configuration.*;
import soar2d.player.*;

public class ConfigurationEditor extends Dialog {

	static Logger logger = Logger.getLogger("soar2d");

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
	Button bookButton;
	Button kitchenButton;
	Button taxiButton;
	Text mapText;
	Button remote;
	Button hide;
	Button nogui;
	Text async;
	Button silentagent;
	Text port;
	Text metadataText;
	
	// rules/eaters
	Text eatersVisionText;
	Text eatersWallPenaltyText;
	Text eatersJumpPenaltyText;
	Text eatersLowProbText;
	Text eatersHighProbText;
	
	// rules/tanksoar
	Text tanksoarDefaultMissilesText;
	Text tanksoarDefaultEnergyText;
	Text tanksoarDefaultHealthText;
	Text tanksoarCollisionPenaltyText;
	Text tanksoarMaxMissilePacksText;
	Text tanksoarMissilePackRespawnText;
	Text tanksoarShieldEnergyUsageText;
	Text tanksoarMissileHitAwardText;
	Text tanksoarMissileHitPenaltyText;
	Text tanksoarKillAwardText;
	Text tanksoarKillPenaltyText;
	Text tanksoarRadarWidthText;
	Text tanksoarRadarHeightText;
	Text tanksoarSmellDistanceText;
	Text tanksoarResetThresholdText;

	// rules/book
	Button bookColoredRoomsButton;
	Text bookSpeedText;
	Text bookCellSizeText;
	Text bookCycleTimeSliceText;
	Text bookVisionConeText;
	Text bookRotateSpeedText;
	Button bookBlocksBlockButton;
	Button bookContinuousButton;
	Button bookZeroIsEastButton;
	
	// rules/taxi
	Button taxiDisableFuelButton;
	Text taxiFuelStartingMinText;
	Text taxiFuelStartingMaxText;
	Text taxiFuelMaxText;

	// logging
	Combo loggingLevelCombo;
	Button loggingFileButton;
	Text loggingNameText;
	Button loggingConsoleButton;
	Button loggingTimeButton;
	Button loggingSoarPrintButton;
	
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
	Button manageShutdownCommandsButton;
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
	Button terminalMaxRuns;
	Text maxRuns;
	Button terminalPassengerDelivered;
	Button terminalFuelRemaining;
	Button terminalPassengerPickUp;
	
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

	private GridData spanFill(int span) {		
		GridData gd = new GridData(GridData.HORIZONTAL_ALIGN_BEGINNING | GridData.GRAB_HORIZONTAL | GridData.FILL_HORIZONTAL);
		gd.horizontalSpan = span;
		return gd;
	}
	
	public ConfigurationEditor(Shell parent) {
		super(parent);
		config = new Configuration(Soar2D.config);
	}

	final String kGeneral = "General";
	final int kGeneralIndex = 0;
	final String kRules = "Rules";
	final int kRulesIndex = 1;
	final String kLogging = "Logging";
	final int kLoggingIndex = 2;
	final String kAgents = "Agents";
	final int kAgentsIndex = 3;
	final String kTerminals = "Terminals";
	final int kTerminalsIndex = 4;
	final String kClients = "Clients";
	final int kClientsIndex = 5;

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
			gd.heightHint = 430;
			gd.widthHint = 100;
			tree.setLayoutData(gd);
		}
		tree.addListener(SWT.Selection, new Listener () {
			public void handleEvent (Event e) {
				updateCurrentPage();
			}
		});
		
		TreeItem general = new TreeItem(tree, SWT.NONE);
		general.setText(kGeneral);
		
		TreeItem rules = new TreeItem(tree, SWT.NONE);
		rules.setText(kRules);
		
		TreeItem logging = new TreeItem(tree, SWT.NONE);
		logging.setText(kLogging);
		
		TreeItem agents = new TreeItem(tree, SWT.NONE);
		agents.setText(kAgents);
		
		Iterator<PlayerConfig> playerIter = config.getPlayers().iterator();
		while (playerIter.hasNext()) {
			TreeItem agent = new TreeItem(agents, SWT.NONE);
			String name = playerIter.next().getName();
			if (name == null) {
				name = "<unnamed>";
			}
			agent.setText(name);
		}
		
		agents.setExpanded(true);

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

		clients.setExpanded(true);

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
				config.setDefaultTerminals();
				TreeItem clients = tree.getItem(kClientsIndex);
				clients.removeAll();
				TreeItem agents = tree.getItem(kAgentsIndex);
				agents.removeAll();
				tree.redraw();
				updateCurrentPage();
			}
		});
		
		Button saveAs = new Button(bottomButtons, SWT.PUSH);
		saveAs.setText("Save as...");
		saveAs.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				String output = config.generateXMLString();

				if (output == null) {
					Soar2D.control.severeError("Couldn't generate configuration.");
					return;
				}
				
				FileDialog fd = new FileDialog(dialog, SWT.SAVE);
				fd.setText("Save as...");
				fd.setFilterPath(Soar2D.simulation.getBasePath());
				fd.setFilterExtensions(new String[] {"*.xml", "*.*"});
				String settingsFileString = fd.open();
				if (settingsFileString != null) {
					if (!settingsFileString.matches(".*\\..+")) {
						settingsFileString += ".xml";
					}
					File configFile = new File(settingsFileString);
					if (configFile.exists() && !configFile.canWrite()) {
						Soar2D.control.severeError("Cannot write to file.");
						return;
					}
					try {
						FileWriter out = new FileWriter(configFile);
						out.write(output);
						out.close();
					} catch (IOException exception) {
						Soar2D.control.severeError("Error writing file: " + exception.getMessage());
						return;
					}
					dialog.dispose();
				}
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
			case kGeneralIndex:
				generalPage();
				break;
			case kRulesIndex:
				rulesPage();
				break;
			case kLoggingIndex:
				loggingPage();
				break;
			case kAgentsIndex:
				agentsPage(selectedItem, index);
				break;
			case kTerminalsIndex:
				terminalsPage();
				break;
			case kClientsIndex:
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
		
		// what sim
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
			tanksoarButton.setSelection(config.getType() == SimType.kTankSoar);
			tanksoarButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					config.setType(SimType.kTankSoar);
					config.setDefaultTerminals();
					generalUpdate();
				}
			});
			tanksoarButton.setLayoutData(spanFill(3));

			eatersButton = new Button(simGroup, SWT.RADIO);
			eatersButton.setText("Eaters");
			eatersButton.setSelection(config.getType() == SimType.kEaters);
			eatersButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					config.setType(SimType.kEaters);
					config.setDefaultTerminals();
					generalUpdate();
				}
			});
			eatersButton.setLayoutData(spanFill(3));
			
			bookButton = new Button(simGroup, SWT.RADIO);
			bookButton.setText("Book");
			bookButton.setSelection(config.getType() == SimType.kBook);
			bookButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					config.setType(SimType.kBook);
					config.setDefaultTerminals();
					generalUpdate();
				}
			});
			bookButton.setLayoutData(spanFill(3));
			
			kitchenButton = new Button(simGroup, SWT.RADIO);
			kitchenButton.setText("Kitchen");
			kitchenButton.setSelection(config.getType() == SimType.kKitchen);
			kitchenButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					config.setType(SimType.kKitchen);
					config.setDefaultTerminals();
					generalUpdate();
				}
			});
			kitchenButton.setLayoutData(spanFill(3));
			
			taxiButton = new Button(simGroup, SWT.RADIO);
			taxiButton.setText("Taxi");
			taxiButton.setSelection(config.getType() == SimType.kTaxi);
			taxiButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					config.setType(SimType.kTaxi);
					config.setDefaultTerminals();
					generalUpdate();
				}
			});
			taxiButton.setLayoutData(spanFill(3));
		}
		
		// map
		{
			Label mapLabel = new Label(currentPage, SWT.NONE);
			mapLabel.setText("Map:");
			mapLabel.setLayoutData(spanFill(3));
			
			mapText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
			mapText.addFocusListener(new FocusAdapter() {
				public void focusLost(FocusEvent e) {
					String mapFileString = mapText.getText();
					if (mapFileString != null) {
						config.setMap(new File(mapFileString));
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
					fd.setFilterPath(config.getMap().getPath());
					fd.setFileName(config.getMap().getName());
					fd.setFilterExtensions(new String[] {"*.*"});
					String mapFileString = fd.open();
					if (mapFileString != null) {
						config.setMap(new File(mapFileString));
					}
					generalUpdate();
				}
			});
		}
		
		// graphical
		nogui = new Button(currentPage, SWT.CHECK);
		nogui.setText("Do not use GUI");
		nogui.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.setNoGUI(!config.getNoGUI());
				generalUpdate();
			}
		});
		nogui.setLayoutData(spanFill(3));

		// 
		Label asyncLabel = new Label(currentPage, SWT.NONE);
		asyncLabel.setText("Asynchronous time slice, milliseconds (0: run synchronously):");
		asyncLabel.setLayoutData(spanFill(3));
		
		async = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		async.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int newSlice;
				try {
					newSlice = Integer.parseInt(async.getText());
				} catch (NumberFormatException exception) {
					newSlice = Soar2D.config.getASyncDelay();
				}
				config.setASyncDelay(newSlice);
				generalUpdate();
			}
		});
		async.setLayoutData(spanFill(3));
		
		// 
		Label portLabel = new Label(currentPage, SWT.NONE);
		portLabel.setText("Port to listen on:");
		portLabel.setLayoutData(spanFill(3));
		
		port = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		port.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int newPort;
				try {
					newPort = Integer.parseInt(port.getText());
				} catch (NumberFormatException exception) {
					newPort = Soar2D.config.getPort();
				}
				config.setPort(newPort);
				generalUpdate();
			}
		});
		port.setLayoutData(spanFill(3));

		// world display
		hide = new Button(currentPage, SWT.CHECK);
		hide.setText("Hide world");
		hide.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.setHide(!config.getHide());
				generalUpdate();
			}
		});
		hide.setLayoutData(spanFill(3));

		// random seed
		useSeed = new Button(currentPage, SWT.CHECK);
		useSeed.setText("Use random seed");
		useSeed.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				if (config.hasRandomSeed()) {
					config.unsetRandomSeed();
				} else {
					config.setRandomSeed(0);
				}
				generalUpdate();
			}
		});
		useSeed.setLayoutData(spanFill(3));
			
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
					newSeed = Soar2D.config.getRandomSeed();
				}
				config.setRandomSeed(newSeed);
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
				config.setRemote(!config.getRemote());
				generalUpdate();
			}
		});
		remote.setLayoutData(spanFill(3));

		silentagent = new Button(currentPage, SWT.CHECK);
		silentagent.setText("Execute 'watch 0' after source");
		silentagent.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.setSilentAgents(!config.getSilentAgents());
				generalUpdate();
			}
		});
		silentagent.setLayoutData(spanFill(3));

		// metadata
		{
			Label metadataLabel = new Label(currentPage, SWT.NONE);
			metadataLabel.setText("Metadata:");
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.BEGINNING;
				gd.horizontalSpan = 3;
				metadataLabel.setLayoutData(gd);
			}
			
			metadataText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
			metadataText.addFocusListener(new FocusAdapter() {
				public void focusLost(FocusEvent e) {
					String metadataFileString = metadataText.getText();
					if (metadataFileString != null && metadataFileString.length() != 0) {
						config.setMetadata(new File(metadataFileString));
					} else {
						config.setMetadata(null);
					}
					generalUpdate();
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.FILL;
				gd.horizontalSpan = 2;
				gd.grabExcessHorizontalSpace = true;
				metadataText.setLayoutData(gd);
			}
			
			Button metadataBrowse = new Button(currentPage, SWT.PUSH);
			metadataBrowse.setText("Browse...");
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.END;
				metadataBrowse.setLayoutData(gd);
			}
			metadataBrowse.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					FileDialog fd = new FileDialog(dialog, SWT.OPEN);
					fd.setText("Open");
					if (config.getMetadata() != null) {
						fd.setFilterPath(config.getMetadata().getPath());
						fd.setFileName(config.getMetadata().getName());
					}
					fd.setFilterExtensions(new String[] {"*.*"});
					String metadataFileString = fd.open();
					if (metadataFileString != null) {
						config.setMetadata(new File(metadataFileString));
					}
					generalUpdate();
				}
			});
		}

		generalUpdate();
		
		rhs.layout(true);
		dialog.layout(true);
	}
	
	public void generalUpdate() {
		switch(config.getType()) {
		case kEaters:
			eatersButton.setSelection(true);
			tanksoarButton.setSelection(false);
			bookButton.setSelection(false);
			kitchenButton.setSelection(false);
			taxiButton.setSelection(false);
			break;
		case kTankSoar:
			eatersButton.setSelection(false);
			tanksoarButton.setSelection(true);
			bookButton.setSelection(false);
			kitchenButton.setSelection(false);
			taxiButton.setSelection(false);
			break;
		case kBook:
			eatersButton.setSelection(false);
			tanksoarButton.setSelection(false);
			bookButton.setSelection(true);
			kitchenButton.setSelection(false);
			taxiButton.setSelection(false);
			break;
		case kKitchen:
			eatersButton.setSelection(false);
			tanksoarButton.setSelection(false);
			bookButton.setSelection(false);
			kitchenButton.setSelection(true);
			taxiButton.setSelection(false);
			break;
		case kTaxi:
			eatersButton.setSelection(false);
			tanksoarButton.setSelection(false);
			bookButton.setSelection(false);
			kitchenButton.setSelection(false);
			taxiButton.setSelection(true);
			break;
		}
		mapText.setText(config.getMap().getAbsolutePath());
		nogui.setSelection(config.getNoGUI());
		async.setText(Integer.toString(config.getASyncDelay()));
		port.setText(Integer.toString(config.getPort()));
		hide.setSelection(config.getHide());
		useSeed.setSelection(config.hasRandomSeed());
		seedText.setEnabled(useSeed.getSelection());
		seedText.setText(Integer.toString(config.getRandomSeed()));
		remote.setSelection(config.getRemote());
		silentagent.setSelection(config.getSilentAgents());
		if (config.getMetadata() != null) {
			metadataText.setText(config.getMetadata().getAbsolutePath());
		} else {
			metadataText.setText("");
		}
	}

	public void rulesPage() {
		if (currentPage != null) {
			currentPage.dispose();
		}
		currentPage = new Composite(rhs, SWT.NONE);
		{
			GridLayout gl = new GridLayout();
			gl.marginHeight = 0;
			gl.marginWidth = 0;
			gl.numColumns = 2;
			currentPage.setLayout(gl);
			
			GridData gd = new GridData();
			gd.grabExcessHorizontalSpace = true;
			gd.grabExcessVerticalSpace = true;
			gd.horizontalAlignment = SWT.FILL;
			gd.verticalAlignment = SWT.FILL;
			currentPage.setLayoutData(gd);
		}
		
		switch(config.getType()) {
		case kEaters:
			rulesEatersPage();
			rulesEatersUpdate();
			break;
		case kTankSoar:
			rulesTankSoarPage();
			rulesTankSoarUpdate();
			break;
		case kBook:
			rulesBookPage();
			rulesBookUpdate();
			break;
		case kKitchen:
			rulesKitchenPage();
			rulesKitchenUpdate();
			break;
		case kTaxi:
			rulesTaxiPage();
			rulesTaxiUpdate();
			break;
		}

		rhs.layout(true);
		dialog.layout(true);
	}
	private void rulesEatersPage() {
//		Text eatersVisionText;
//		Text eatersWallPenaltyText;
//		Text eatersJumpPenaltyText;
//		Text eatersLowProbText;
//		Text eatersHighProbText;
		
		Label visionLabel = new Label(currentPage, SWT.NONE);
		visionLabel.setText("Eater vision (cells):");

		eatersVisionText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		eatersVisionText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int newVision;
				try {
					newVision = Integer.parseInt(eatersVisionText.getText());
				} catch (NumberFormatException exception) {
					newVision = Soar2D.config.eConfig.getEaterVision();
				}
				config.eConfig.setEaterVision(newVision);
				rulesEatersUpdate();
			}
		});
		eatersVisionText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label wallPenaltyLabel = new Label(currentPage, SWT.NONE);
		wallPenaltyLabel.setText("Wall collision penalty:");

		eatersWallPenaltyText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		eatersWallPenaltyText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int newPenalty;
				try {
					newPenalty = Integer.parseInt(eatersWallPenaltyText.getText());
				} catch (NumberFormatException exception) {
					newPenalty = Soar2D.config.eConfig.getWallPenalty();
				}
				config.eConfig.setWallPenalty(newPenalty);
				rulesEatersUpdate();
			}
		});
		eatersWallPenaltyText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label jumpPenaltyLabel = new Label(currentPage, SWT.NONE);
		jumpPenaltyLabel.setText("Jump action penalty:");

		eatersJumpPenaltyText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		eatersJumpPenaltyText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int newPenalty;
				try {
					newPenalty = Integer.parseInt(eatersJumpPenaltyText.getText());
				} catch (NumberFormatException exception) {
					newPenalty = Soar2D.config.eConfig.getJumpPenalty();
				}
				config.eConfig.setJumpPenalty(newPenalty);
				rulesEatersUpdate();
			}
		});
		eatersJumpPenaltyText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label lowProbLabel = new Label(currentPage, SWT.NONE);
		lowProbLabel.setText("Map generation low param:");

		eatersLowProbText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		eatersLowProbText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				double newProb;
				try {
					newProb = Double.parseDouble(eatersLowProbText.getText());
				} catch (NumberFormatException exception) {
					newProb = Soar2D.config.eConfig.getLowProbability();
				}
				config.eConfig.setLowProbability(newProb);
				rulesEatersUpdate();
			}
		});
		eatersLowProbText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label highProbLabel = new Label(currentPage, SWT.NONE);
		highProbLabel.setText("Map generation high param:");

		eatersHighProbText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		eatersHighProbText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				double newProb;
				try {
					newProb = Double.parseDouble(eatersHighProbText.getText());
				} catch (NumberFormatException exception) {
					newProb = Soar2D.config.eConfig.getHighProbability();
				}
				config.eConfig.setHighProbability(newProb);
				rulesEatersUpdate();
			}
		});
		eatersHighProbText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));
	}
	private void rulesEatersUpdate() {
		eatersVisionText.setText(Integer.toString(config.eConfig.getEaterVision()));
		eatersWallPenaltyText.setText(Integer.toString(config.eConfig.getWallPenalty()));
		eatersJumpPenaltyText.setText(Integer.toString(config.eConfig.getJumpPenalty()));
		eatersLowProbText.setText(Double.toString(config.eConfig.getLowProbability()));
		eatersHighProbText.setText(Double.toString(config.eConfig.getHighProbability()));
	}
	
	private void rulesTankSoarPage() {
//		Text tanksoarDefaultMissilesText;
//		Text tanksoarDefaultEnergyText;
//		Text tanksoarDefaultHealthText;
//		Text tanksoarCollisionPenaltyText;
//		Text tanksoarMaxMissilePacksText;
//		Text tanksoarMissilePackRespawnText;
//		Text tanksoarShieldEnergyUsageText;
//		Text tanksoarMissileHitAwardText;
//		Text tanksoarMissileHitPenaltyText;
//		Text tanksoarKillAwardText;
//		Text tanksoarKillPenaltyText;
//		Text tanksoarRadarWidthText;
//		Text tanksoarRadarHeightText;
//		Text tanksoarSmellDistanceText;
//		Text tanksoarResetThresholdText;

		Label defaultMissilesLabel = new Label(currentPage, SWT.NONE);
		defaultMissilesLabel.setText("Starting missiles:");

		tanksoarDefaultMissilesText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		tanksoarDefaultMissilesText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(tanksoarDefaultMissilesText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.tConfig.getDefaultMissiles();
				}
				config.tConfig.setDefaultMissiles(text);
				rulesTankSoarUpdate();
			}
		});
		tanksoarDefaultMissilesText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label defaultEnergyLabel = new Label(currentPage, SWT.NONE);
		defaultEnergyLabel.setText("Starting energy:");

		tanksoarDefaultEnergyText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		tanksoarDefaultEnergyText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(tanksoarDefaultEnergyText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.tConfig.getDefaultEnergy();
				}
				config.tConfig.setDefaultEnergy(text);
				rulesTankSoarUpdate();
			}
		});
		tanksoarDefaultEnergyText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label defaultHealthLabel = new Label(currentPage, SWT.NONE);
		defaultHealthLabel.setText("Starting health:");

		tanksoarDefaultHealthText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		tanksoarDefaultHealthText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(tanksoarDefaultHealthText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.tConfig.getDefaultHealth();
				}
				config.tConfig.setDefaultHealth(text);
				rulesTankSoarUpdate();
			}
		});
		tanksoarDefaultHealthText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label collisionPenaltyLabel = new Label(currentPage, SWT.NONE);
		collisionPenaltyLabel.setText("Collision penalty:");

		tanksoarCollisionPenaltyText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		tanksoarCollisionPenaltyText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(tanksoarCollisionPenaltyText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.tConfig.getCollisionPenalty();
				}
				config.tConfig.setCollisionPenalty(text);
				rulesTankSoarUpdate();
			}
		});
		tanksoarCollisionPenaltyText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label maxMissilePacksLabel = new Label(currentPage, SWT.NONE);
		maxMissilePacksLabel.setText("Max missile packs:");

		tanksoarMaxMissilePacksText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		tanksoarMaxMissilePacksText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(tanksoarMaxMissilePacksText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.tConfig.getMaxMissilePacks();
				}
				config.tConfig.setMaxMissilePacks(text);
				rulesTankSoarUpdate();
			}
		});
		tanksoarMaxMissilePacksText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label missilePackRespawnLabel = new Label(currentPage, SWT.NONE);
		missilePackRespawnLabel.setText("Missile pack respawn chance:");

		tanksoarMissilePackRespawnText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		tanksoarMissilePackRespawnText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(tanksoarMissilePackRespawnText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.tConfig.getMissilePackRespawnChance();
				}
				config.tConfig.setMissilePackRespawnChance(text);
				rulesTankSoarUpdate();
			}
		});
		tanksoarMissilePackRespawnText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label shieldEnergyUsageLabel = new Label(currentPage, SWT.NONE);
		shieldEnergyUsageLabel.setText("Shield energy usage:");

		tanksoarShieldEnergyUsageText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		tanksoarShieldEnergyUsageText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(tanksoarShieldEnergyUsageText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.tConfig.getShieldEnergyUsage();
				}
				config.tConfig.setShieldEnergyUsage(text);
				rulesTankSoarUpdate();
			}
		});
		tanksoarShieldEnergyUsageText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label missileHitAwardLabel = new Label(currentPage, SWT.NONE);
		missileHitAwardLabel.setText("Missile hit award:");

		tanksoarMissileHitAwardText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		tanksoarMissileHitAwardText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(tanksoarMissileHitAwardText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.tConfig.getMissileHitAward();
				}
				config.tConfig.setMissileHitAward(text);
				rulesTankSoarUpdate();
			}
		});
		tanksoarMissileHitAwardText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label missileHitPenaltyLabel = new Label(currentPage, SWT.NONE);
		missileHitPenaltyLabel.setText("Missile hit penalty:");

		tanksoarMissileHitPenaltyText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		tanksoarMissileHitPenaltyText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(tanksoarMissileHitPenaltyText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.tConfig.getMissileHitPenalty();
				}
				config.tConfig.setMissileHitPenalty(text);
				rulesTankSoarUpdate();
			}
		});
		tanksoarMissileHitPenaltyText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label killAwardLabel = new Label(currentPage, SWT.NONE);
		killAwardLabel.setText("Frag award:");

		tanksoarKillAwardText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		tanksoarKillAwardText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(tanksoarKillAwardText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.tConfig.getKillAward();
				}
				config.tConfig.setKillAward(text);
				rulesTankSoarUpdate();
			}
		});
		tanksoarKillAwardText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label killPenaltyLabel = new Label(currentPage, SWT.NONE);
		killPenaltyLabel.setText("Frag penalty:");

		tanksoarKillPenaltyText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		tanksoarKillPenaltyText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(tanksoarKillPenaltyText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.tConfig.getKillPenalty();
				}
				config.tConfig.setKillPenalty(text);
				rulesTankSoarUpdate();
			}
		});
		tanksoarKillPenaltyText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label radarWidthLabel = new Label(currentPage, SWT.NONE);
		radarWidthLabel.setText("Radar width:");

		tanksoarRadarWidthText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		tanksoarRadarWidthText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(tanksoarRadarWidthText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.tConfig.getRadarWidth();
				}
				config.tConfig.setRadarWidth(text);
				rulesTankSoarUpdate();
			}
		});
		tanksoarRadarWidthText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label radarHeightLabel = new Label(currentPage, SWT.NONE);
		radarHeightLabel.setText("Radar height:");

		tanksoarRadarHeightText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		tanksoarRadarHeightText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(tanksoarRadarHeightText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.tConfig.getRadarHeight();
				}
				config.tConfig.setRadarHeight(text);
				rulesTankSoarUpdate();
			}
		});
		tanksoarRadarHeightText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label maxSmellDistanceLabel = new Label(currentPage, SWT.NONE);
		maxSmellDistanceLabel.setText("Max smell distance:");

		tanksoarSmellDistanceText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		tanksoarSmellDistanceText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(tanksoarSmellDistanceText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.tConfig.getMaxSmellDistance();
				}
				config.tConfig.setMaxSmellDistance(text);
				rulesTankSoarUpdate();
			}
		});
		tanksoarSmellDistanceText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label resetThresholdLabel = new Label(currentPage, SWT.NONE);
		resetThresholdLabel.setText("Reset threshold:");

		tanksoarResetThresholdText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		tanksoarResetThresholdText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(tanksoarResetThresholdText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.tConfig.getMissileResetThreshold();
				}
				config.tConfig.setMissileResetThreshold(text);
				rulesTankSoarUpdate();
			}
		});
		tanksoarResetThresholdText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

	}
	private void rulesTankSoarUpdate() {
		tanksoarDefaultMissilesText.setText(Integer.toString(config.tConfig.getDefaultMissiles()));
		tanksoarDefaultEnergyText.setText(Integer.toString(config.tConfig.getDefaultEnergy()));
		tanksoarDefaultHealthText.setText(Integer.toString(config.tConfig.getDefaultHealth()));
		tanksoarCollisionPenaltyText.setText(Integer.toString(config.tConfig.getCollisionPenalty()));
		tanksoarMaxMissilePacksText.setText(Integer.toString(config.tConfig.getMaxMissilePacks()));
		tanksoarMissilePackRespawnText.setText(Integer.toString(config.tConfig.getMissilePackRespawnChance()));
		tanksoarShieldEnergyUsageText.setText(Integer.toString(config.tConfig.getShieldEnergyUsage()));
		tanksoarMissileHitAwardText.setText(Integer.toString(config.tConfig.getMissileHitAward()));
		tanksoarMissileHitPenaltyText.setText(Integer.toString(config.tConfig.getMissileHitPenalty()));
		tanksoarKillAwardText.setText(Integer.toString(config.tConfig.getKillAward()));
		tanksoarKillPenaltyText.setText(Integer.toString(config.tConfig.getKillPenalty()));
		tanksoarRadarWidthText.setText(Integer.toString(config.tConfig.getRadarWidth()));
		tanksoarRadarHeightText.setText(Integer.toString(config.tConfig.getRadarHeight()));
		tanksoarSmellDistanceText.setText(Integer.toString(config.tConfig.getMaxSmellDistance()));
		tanksoarResetThresholdText.setText(Integer.toString(config.tConfig.getMissileResetThreshold()));
		
	}
	private void rulesBookPage() {
//		Button bookColoredRoomsButton;
//		Text bookSpeedText;
//		Text bookCellSizeText;
//		Text bookCycleTimeSliceText;
//		Text bookVisionConeText;
//		Text bookRotateSpeedText;
//		Button bookBlocksBlockButton;
//		Button bookContinuousButton;
		
		bookColoredRoomsButton = new Button(currentPage, SWT.CHECK);
		bookColoredRoomsButton.setText("Colored rooms");
		bookColoredRoomsButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.bConfig.setColoredRooms(!config.bConfig.getColoredRooms());
				rulesBookUpdate();
			}
		});
		bookColoredRoomsButton.setLayoutData(spanFill(2));

		Label speedLabel = new Label(currentPage, SWT.NONE);
		speedLabel.setText("Speed:");

		bookSpeedText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		bookSpeedText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(bookSpeedText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.bConfig.getSpeed();
				}
				config.bConfig.setSpeed(text);
				rulesBookUpdate();
			}
		});
		bookSpeedText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label cellSizeLabel = new Label(currentPage, SWT.NONE);
		cellSizeLabel.setText("Cell size:");

		bookCellSizeText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		bookCellSizeText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(bookCellSizeText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.bConfig.getBookCellSize();
				}
				config.bConfig.setBookCellSize(text);
				rulesBookUpdate();
			}
		});
		bookCellSizeText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label cycleTimeSliceLabel = new Label(currentPage, SWT.NONE);
		cycleTimeSliceLabel.setText("Time slice:");

		bookCycleTimeSliceText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		bookCycleTimeSliceText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(bookCycleTimeSliceText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.bConfig.getCycleTimeSlice();
				}
				config.bConfig.setCycleTimeSlice(text);
				rulesBookUpdate();
			}
		});
		bookCycleTimeSliceText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label visionConeLabel = new Label(currentPage, SWT.NONE);
		visionConeLabel.setText("Field of view:");

		bookVisionConeText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		bookVisionConeText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				double text;
				try {
					text = Double.parseDouble(bookVisionConeText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.bConfig.getVisionCone();
				}
				config.bConfig.setVisionCone(text);
				rulesBookUpdate();
			}
		});
		bookVisionConeText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label rotateSpeedLabel = new Label(currentPage, SWT.NONE);
		rotateSpeedLabel.setText("Rotation speed:");

		bookRotateSpeedText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		bookRotateSpeedText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				float text;
				try {
					text = Float.parseFloat(bookRotateSpeedText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.bConfig.getRotateSpeed();
				}
				config.bConfig.setRotateSpeed(text);
				rulesBookUpdate();
			}
		});
		bookRotateSpeedText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		bookBlocksBlockButton = new Button(currentPage, SWT.CHECK);
		bookBlocksBlockButton.setText("Blocks block");
		bookBlocksBlockButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.bConfig.setBlocksBlock(!config.bConfig.getBlocksBlock());
				rulesBookUpdate();
			}
		});
		bookBlocksBlockButton.setLayoutData(spanFill(2));

		bookContinuousButton = new Button(currentPage, SWT.CHECK);
		bookContinuousButton.setText("Continuous");
		bookContinuousButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.bConfig.setContinuous(!config.bConfig.getContinuous());
				rulesBookUpdate();
			}
		});
		bookContinuousButton.setLayoutData(spanFill(2));

		bookZeroIsEastButton = new Button(currentPage, SWT.CHECK);
		bookZeroIsEastButton.setText("Zero is east (as opposed to North)");
		bookZeroIsEastButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.bConfig.setZeroIsEast(!config.bConfig.getZeroIsEast());
				rulesBookUpdate();
			}
		});
		bookZeroIsEastButton.setLayoutData(spanFill(2));

	}
	private void rulesBookUpdate() {
		bookColoredRoomsButton.setSelection(config.bConfig.getColoredRooms());
		bookSpeedText.setText(Integer.toString(config.bConfig.getSpeed()));
		bookCellSizeText.setText(Integer.toString(config.bConfig.getBookCellSize()));
		bookCycleTimeSliceText.setText(Integer.toString(config.bConfig.getCycleTimeSlice()));
		bookVisionConeText.setText(Double.toString(config.bConfig.getVisionCone()));
		bookRotateSpeedText.setText(Float.toString(config.bConfig.getRotateSpeed()));
		bookBlocksBlockButton.setSelection(config.bConfig.getBlocksBlock());
		bookContinuousButton.setSelection(config.bConfig.getContinuous());
		bookZeroIsEastButton.setSelection(config.bConfig.getZeroIsEast());
	}
	private void rulesKitchenPage() {
		Label kitchenLabel = new Label(currentPage, SWT.NONE);
		kitchenLabel.setText("No rules settings for kitchen environment.");
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = GridData.BEGINNING;
			gd.horizontalIndent = 20;
			kitchenLabel.setLayoutData(gd);
		}
	}
	private void rulesTaxiPage() {
//		Button taxiDisableFuelButton;
//		Text taxiFuelStartingMinText;
//		Text taxiFuelStartingMaxText;
//		Text taxiFuelMaxText;
		
		taxiDisableFuelButton = new Button(currentPage, SWT.CHECK);
		taxiDisableFuelButton.setText("Disable fuel");
		taxiDisableFuelButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.xConfig.setDisableFuel(!config.xConfig.getDisableFuel());
				rulesBookUpdate();
			}
		});
		taxiDisableFuelButton.setLayoutData(spanFill(2));

		Label fuelStartingMinLabel = new Label(currentPage, SWT.NONE);
		fuelStartingMinLabel.setText("Fuel starting minimum:");

		taxiFuelStartingMinText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		taxiFuelStartingMinText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(taxiFuelStartingMinText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.xConfig.getFuelStartingMinimum();
				}
				config.xConfig.setFuelStartingMinimum(text);
				rulesTaxiUpdate();
			}
		});
		taxiFuelStartingMinText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label fuelStartingMaxLabel = new Label(currentPage, SWT.NONE);
		fuelStartingMaxLabel.setText("Fuel starting maximum:");

		taxiFuelStartingMaxText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		taxiFuelStartingMaxText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(taxiFuelStartingMaxText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.xConfig.getFuelStartingMaximum();
				}
				config.xConfig.setFuelStartingMaximum(text);
				rulesTaxiUpdate();
			}
		});
		taxiFuelStartingMaxText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

		Label fuelMaxLabel = new Label(currentPage, SWT.NONE);
		fuelMaxLabel.setText("Fuel maximum:");

		taxiFuelMaxText = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		taxiFuelMaxText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				int text;
				try {
					text = Integer.parseInt(taxiFuelMaxText.getText());
				} catch (NumberFormatException exception) {
					text = Soar2D.config.xConfig.getFuelMaximum();
				}
				config.xConfig.setFuelMaximum(text);
				rulesTaxiUpdate();
			}
		});
		taxiFuelMaxText.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.GRAB_HORIZONTAL));

	}
	
	private void rulesKitchenUpdate() {
		
	}
	private void rulesTaxiUpdate() {
		taxiDisableFuelButton.setSelection(config.xConfig.getDisableFuel());
		taxiFuelStartingMinText.setText(Integer.toString(config.xConfig.getFuelStartingMinimum()));
		taxiFuelStartingMaxText.setText(Integer.toString(config.xConfig.getFuelStartingMaximum()));
		taxiFuelMaxText.setText(Integer.toString(config.xConfig.getFuelMaximum()));
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
					config.setLogConsole(!config.getLogConsole());
					loggingUpdate();
				}
			});
			loggingConsoleButton.setLayoutData(spanFill(2));

			loggingFileButton = new Button(targetsGroup, SWT.CHECK);
			loggingFileButton.setText("File");
			loggingFileButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					File logFile = config.getLogFile();
					if (logFile == null) {
						config.setLogFile(Configuration.kDefaultLogFile);
					} else {
						config.setLogFile(null);
					}
					loggingUpdate();
				}
			});
			loggingFileButton.setLayoutData(spanFill(2));
			
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
						config.setLogFile(new File(loggingNameText.getText()));
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
					fd.setFilterPath(Soar2D.simulation.getBasePath());
					fd.setFileName(config.getLogFile().getName());
					fd.setFilterExtensions(new String[] {"*.*"});
					String logFileString = fd.open();
					if (logFileString != null) {
						config.setLogFile(new File(logFileString));
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
				config.setLogTime(!config.getLogTime());
				loggingUpdate();
			}
		});
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = SWT.BEGINNING;
			loggingTimeButton.setLayoutData(gd);
		}
		
		Label loggingLevelLabel = new Label(currentPage, SWT.NONE);
		loggingLevelLabel.setText("Log detail level:");
		
		loggingLevelCombo = new Combo(currentPage, SWT.READ_ONLY);
		{
			GridData gd = new GridData();
			gd.horizontalIndent = 20;
			gd.horizontalAlignment = GridData.FILL;
			gd.grabExcessHorizontalSpace = true;
			loggingLevelCombo.setLayoutData(gd);
		}
		loggingLevelCombo.setItems(Configuration.kLogLevels);
		loggingLevelCombo.setVisibleItemCount(6);
		Level logLevel = config.getLogLevel();
		if (logLevel == Level.SEVERE) {
			loggingLevelCombo.select(0);
		} else if (logLevel == Level.WARNING) {
			loggingLevelCombo.select(1);
		} else if (logLevel == Level.INFO) {
			loggingLevelCombo.select(2);
		} else if (logLevel == Level.FINE) {
			loggingLevelCombo.select(3);
		} else if (logLevel == Level.FINER) {
			loggingLevelCombo.select(4);
		} else if (logLevel == Level.FINEST) {
			loggingLevelCombo.select(5);
		} else {
			loggingLevelCombo.select(2);
		}
		loggingLevelCombo.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.setLogLevel(Configuration.kLogLevels[loggingLevelCombo.getSelectionIndex()]);
				loggingUpdate();
			}
		});

		// log soar print events
		loggingSoarPrintButton = new Button(currentPage, SWT.CHECK);
		loggingSoarPrintButton.setText("Log Soar print events to Soar2D log");
		loggingSoarPrintButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.setLogSoarPrint(!config.getLogSoarPrint());
				loggingUpdate();
			}
		});
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = SWT.BEGINNING;
			loggingSoarPrintButton.setLayoutData(gd);
		}
		
		loggingUpdate();

		rhs.layout(true);
		dialog.layout(true);
	}
	
	public void loggingUpdate() {
		if (config.getLogFile() != null) {
			loggingFileButton.setSelection(true);
			loggingNameText.setEnabled(true);
			loggingNameText.setText(config.getLogFile().getAbsolutePath());
		} else {
			loggingFileButton.setSelection(false);
			loggingNameText.setEnabled(false);
		}
		loggingConsoleButton.setSelection(config.getLogConsole());
		loggingTimeButton.setSelection(config.getLogTime());
		loggingSoarPrintButton.setSelection(config.getLogSoarPrint());
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
					config.setDebuggers(!config.getDebuggers());
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
			playerConfig = config.getPlayers().get(playerConfigIndex);
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
					int dir = Direction.getInt(agentFacingCombo.getText());
					if (dir == 0) {
						dir = 1;
					}
					playerConfig.setFacing(dir);
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
						config.getPlayers().add(new PlayerConfig(playerConfig));
						TreeItem agents = tree.getItem(kAgentsIndex);
						TreeItem newAgent = new TreeItem(agents, SWT.NONE);
						String name;
						if (playerConfig.hasName()) {
							name = playerConfig.getName();
						} else {
							name = "<unnamed>";
						}
						newAgent.setText(name);
						agents.setExpanded(true);
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
			manageShutdownCommandsButton = new Button(currentPage, SWT.PUSH);
			manageShutdownCommandsButton.setText("Manage shutdown commands");
			manageShutdownCommandsButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					ShutdownCommandManager m = new ShutdownCommandManager(dialog, playerConfig);
					m.open();
				}			
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.END;
				gd.horizontalSpan = 5;
				manageShutdownCommandsButton.setLayoutData(gd);
			}
			
			removeAgentButton = new Button(currentPage, SWT.PUSH);
			removeAgentButton.setText("Remove this agent");
			removeAgentButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					config.getPlayers().remove(playerConfigIndex);
					playerConfigIndex = -1;
					playerConfig = null;
					selectedItem.dispose();
					tree.redraw();
					TreeItem newSelection = tree.getItem(kAgentsIndex);
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
		
		if (selectedItem == null) {	
			agentsDebuggersButton.setSelection(config.getDebuggers());
		}
		
		boolean createReady = true;
		boolean allDisabled = false;
		
		if ((selectedItem == null) 
				&& (config.getPlayers().size() >= Soar2D.simulation.kColors.length)) {
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
		terminalMaxUpdates.setSelection(config.getTerminalMaxUpdates() > 0);
		terminalMaxUpdates.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				terminalsUpdate();
			}
		});
		
		// max updates requires a number of updates
		maxUpdates = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		maxUpdates.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				try {
					config.setTerminalMaxUpdates(Integer.parseInt(maxUpdates.getText()));
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
		terminalAgentCommand.setSelection(config.getTerminalAgentCommand());
		terminalAgentCommand.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.setTerminalAgentCommand(terminalAgentCommand.getSelection());
			}
		});

		terminalWinningScore = new Button(currentPage, SWT.CHECK);
		terminalWinningScore.setText("Stop when a score is achieved");
		terminalWinningScore.setSelection(config.getTerminalWinningScore() > 0);
		terminalWinningScore.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				terminalsUpdate();
			}
		});

		// winning score requires a score
		winningScore = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		winningScore.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				try {
					config.setTerminalWinningScore(Integer.parseInt(winningScore.getText()));
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
		terminalPointsRemaining.setSelection(config.getTerminalPointsRemaining());
		terminalPointsRemaining.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.setTerminalPointsRemaining(terminalPointsRemaining.getSelection());
			}
		});

		terminalFoodRemaining = new Button(currentPage, SWT.CHECK);
		terminalFoodRemaining.setText("Stop when there is no food remaining (Eaters)");
		terminalFoodRemaining.setSelection(config.getTerminalFoodRemaining());
		terminalFoodRemaining.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.setTerminalFoodRemaining(terminalFoodRemaining.getSelection());
			}
		});

		terminalUnopenedBoxes = new Button(currentPage, SWT.CHECK);
		terminalUnopenedBoxes.setText("Stop when there are no unopened boxes (Eaters)");
		terminalUnopenedBoxes.setSelection(config.getTerminalUnopenedBoxes());
		terminalUnopenedBoxes.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.setTerminalUnopenedBoxes(terminalUnopenedBoxes.getSelection());
			}
		});

		terminalMaxRuns = new Button(currentPage, SWT.CHECK);
		terminalMaxRuns.setText("Number of runs to complete before stopping");
		terminalMaxRuns.setSelection(config.getTerminalMaxRuns() > 0);
		terminalMaxRuns.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				terminalsUpdate();
			}
		});
		
		// max updates requires a number of updates
		maxRuns = new Text(currentPage, SWT.SINGLE | SWT.BORDER);
		maxRuns.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				try {
					config.setTerminalMaxRuns(Integer.parseInt(maxRuns.getText()));
				} catch (NumberFormatException exeption) {}
				terminalsUpdate();
			}
		});
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = GridData.FILL;
			gd.horizontalIndent = 20;
			gd.grabExcessHorizontalSpace = true;
			maxRuns.setLayoutData(gd);
		}

		terminalPassengerDelivered = new Button(currentPage, SWT.CHECK);
		terminalPassengerDelivered.setText("Stop when the passenger is delivered (Taxi)");
		terminalPassengerDelivered.setSelection(config.getTerminalPassengerDelivered());
		terminalPassengerDelivered.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.setTerminalPassengerDelivered(terminalPassengerDelivered.getSelection());
			}
		});

		terminalFuelRemaining = new Button(currentPage, SWT.CHECK);
		terminalFuelRemaining.setText("Stop when there is no fuel remaining (Taxi)");
		terminalFuelRemaining.setSelection(config.getTerminalFoodRemaining());
		terminalFuelRemaining.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.setTerminalFoodRemaining(terminalFuelRemaining.getSelection());
			}
		});

		terminalPassengerPickUp = new Button(currentPage, SWT.CHECK);
		terminalPassengerPickUp.setText("Stop when the passenger is picked up (Taxi)");
		terminalPassengerPickUp.setSelection(config.getTerminalPassengerPickUp());
		terminalPassengerPickUp.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				config.setTerminalPassengerPickUp(terminalPassengerPickUp.getSelection());
			}
		});

		terminalsUpdate();
		
		rhs.layout(true);
		dialog.layout(true);
	}
	
	public void terminalsUpdate() {
		if (terminalMaxUpdates.getSelection()) {
			maxUpdates.setEnabled(true);
			maxUpdates.setText(Integer.toString(config.getTerminalMaxUpdates()));
		} else {
			maxUpdates.setEnabled(false);
			maxUpdates.setText("");
		}
		
		if (terminalWinningScore.getSelection()) {
			winningScore.setEnabled(true);
			winningScore.setText(Integer.toString(config.getTerminalWinningScore()));
		} else {
			winningScore.setEnabled(false);
			winningScore.setText("");
		}
		
		if (terminalMaxRuns.getSelection()) {
			maxRuns.setEnabled(true);
			maxRuns.setText(Integer.toString(config.getTerminalMaxRuns()));
		} else {
			maxRuns.setEnabled(false);
			maxRuns.setText("");
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
			
			Label clientCommandLabel = new Label(newClientGroup, SWT.NONE);
			clientCommandLabel.setText("Command:");
		
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
			clientTimeoutButton.setSelection(clientConfig.timeout != ClientConfig.kDefaultTimeout);
			clientTimeoutButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					if (!clientCommandButton.getSelection()) {
						clientConfig.timeout = ClientConfig.kDefaultTimeout;
					}
					clientsUpdate(selectedItem);
				}
			});
			
			Label clientTimeoutLabel = new Label(newClientGroup, SWT.NONE);
			clientTimeoutLabel.setText("Timeout (seconds):");
		
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
			
			if (selectedItem == null) {
				createClientButton = new Button(newClientGroup, SWT.PUSH);
				createClientButton.setText("Create new client");
				createClientButton.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						config.clients.add(new ClientConfig(clientConfig));
						TreeItem clients = tree.getItem(kClientsIndex);
						TreeItem newClient = new TreeItem(clients, SWT.NONE);
						String name = clientConfig.name;
						newClient.setText(name);
						clients.setExpanded(true);
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
					TreeItem newSelection = tree.getItem(kClientsIndex);
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

class ShutdownCommandManager extends Dialog {
	Shell dialog;
	Composite rhs;
	Text commandText;
	Button add;
	Button moveUp;
	Button moveDown;
	Button remove;
	PlayerConfig player;
	Table table;
	
	public ShutdownCommandManager(Shell parent, PlayerConfig player) {
		super(parent);
		this.player = player;
	}
	
	public void open() {
		Shell parent = getParent();
		dialog = new Shell(parent, SWT.DIALOG_TRIM | SWT.APPLICATION_MODAL);
		dialog.setText("Shutdown Commands");
		{ 
			GridLayout gl = new GridLayout();
			gl.numColumns = 2;
			dialog.setLayout(gl);
		}
		
		table = new Table (dialog, SWT.BORDER | SWT.V_SCROLL | SWT.H_SCROLL);
		Iterator<String> iter = player.getShutdownCommands().iterator();
		while (iter.hasNext()) {
			String command = iter.next();
			TableItem item = new TableItem(table, SWT.NONE);
			item.setText(command);
		}
		table.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				update();
			}
		});
		{
			GridData gd = new GridData();
			gd.widthHint = 150;
			gd.heightHint = 150;
			table.setLayoutData(gd);
		}
		
		rhs = new Composite(dialog, SWT.NONE);
		{
			GridLayout gl = new GridLayout();
			gl.marginHeight = 0;
			gl.marginWidth = 0;
			gl.numColumns = 2;
			rhs.setLayout(gl);

			GridData gd = new GridData();
			gd.widthHint = 150;
			gd.horizontalAlignment = SWT.FILL;
			gd.verticalAlignment = SWT.FILL;
			rhs.setLayoutData(gd);
		}

		Label newCommand = new Label(rhs, SWT.NONE);
		newCommand.setText("New Command:");
		{
			GridData gd = new GridData();
			gd.horizontalSpan = 2;
			newCommand.setLayoutData(gd);
		}

		commandText = new Text(rhs, SWT.SINGLE | SWT.BORDER);
		commandText.addFocusListener(new FocusAdapter() {
			public void focusLost(FocusEvent e) {
				update();
			}
		});
		commandText.addKeyListener(new KeyAdapter() {
			public void keyReleased(KeyEvent e) {
				update();
			}
		});
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = GridData.FILL;
			gd.grabExcessHorizontalSpace = true;
			commandText.setLayoutData(gd);
		}

		add = new Button(rhs, SWT.PUSH);
		add.setText("Add");
		add.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				TableItem item = new TableItem(table, SWT.NONE);
				item.setText(commandText.getText());
				player.addShutdownCommand(commandText.getText());
				update();
			}
		});
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = SWT.END;
			add.setLayoutData(gd);
		}

		moveUp = new Button(rhs, SWT.PUSH);
		moveUp.setText("Move up");
		moveUp.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				int index = table.getSelectionIndex();
				ArrayList<String> commands = player.getShutdownCommands();
				String selected = commands.remove(index);
				index -= 1;
				commands.add(index, selected);
				
				table.removeAll();
				Iterator<String> iter = commands.iterator();
				while (iter.hasNext()) {
					String command = iter.next();
					TableItem item = new TableItem(table, SWT.NONE);
					item.setText(command);
				}
				table.setSelection(index);
				update();
			}
		});
		{
			GridData gd = new GridData();
			gd.horizontalSpan = 2;
			gd.horizontalAlignment = SWT.BEGINNING;
			moveUp.setLayoutData(gd);
		}

		remove = new Button(rhs, SWT.PUSH);
		remove.setText("Remove");
		remove.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				player.getShutdownCommands().remove(table.getSelectionIndex());
				table.remove(table.getSelectionIndex());
				update();
			}
		});
		{
			GridData gd = new GridData();
			gd.horizontalSpan = 2;
			gd.horizontalAlignment = SWT.BEGINNING;
			remove.setLayoutData(gd);
		}

		moveDown = new Button(rhs, SWT.PUSH);
		moveDown.setText("Move down");
		moveDown.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				int index = table.getSelectionIndex();
				ArrayList<String> commands = player.getShutdownCommands();
				String selected = commands.remove(index);
				index += 1;
				commands.add(index, selected);
				
				table.removeAll();
				Iterator<String> iter = commands.iterator();
				while (iter.hasNext()) {
					String command = iter.next();
					TableItem item = new TableItem(table, SWT.NONE);
					item.setText(command);
				}
				table.setSelection(index);
				update();
			}
		});
		{
			GridData gd = new GridData();
			gd.horizontalSpan = 2;
			gd.horizontalAlignment = SWT.BEGINNING;
			moveDown.setLayoutData(gd);
		}

		Button ok = new Button(dialog, SWT.PUSH);
		ok.setText("OK");
		ok.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				dialog.dispose();
			}
		});
		{
			GridData gd = new GridData();
			gd.horizontalSpan = 2;
			gd.horizontalAlignment = SWT.END;
			ok.setLayoutData(gd);
		}

		update();
		
		dialog.setSize(dialog.computeSize(SWT.DEFAULT, SWT.DEFAULT, false));
		dialog.open ();
	}
	
	private void update() {
		if (commandText.getText().length() > 0) {
			add.setEnabled(true);
		} else {
			add.setEnabled(false);
		}
		
		if (table.getSelectionIndex() < 0) {
			remove.setEnabled(false);
		} else {
			remove.setEnabled(true);
		}
		
		if (table.getItemCount() < 2) {
			moveUp.setEnabled(false);
			moveDown.setEnabled(false);
		} else {
			if (table.getSelectionIndex() < 0) {
				moveUp.setEnabled(false);
				moveDown.setEnabled(false);
			} else if (table.getSelectionIndex() == 0) {
				moveUp.setEnabled(false);
				moveDown.setEnabled(true);
			} else if (table.getSelectionIndex() == (table.getItemCount() - 1)) {
				moveUp.setEnabled(true);
				moveDown.setEnabled(false);
			} else {
				moveUp.setEnabled(true);
				moveDown.setEnabled(true);
			}
		}
	}
}
