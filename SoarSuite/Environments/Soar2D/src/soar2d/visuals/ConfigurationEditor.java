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
	Label seedLabel;
	Text seedText;
	Button tanksoarButton;
	Button eatersButton;
	Text mapText;
	Button remote;
	Button hide;

	// logging
	Combo loggingLevelCombo;
	Button loggingFileButton;
	Label loggingNameLabel;
	Text loggingNameText;
	Button loggingConsoleButton;
	Button loggingTimeButton;
	
	// agents
	Text agentsNameText;
	Text agentsProductionsText;
	Combo agentsColorCombo;
	Button agentsDebuggersButton;
	PlayerConfig playerConfig;
	
	

	public ConfigurationEditor(Shell parent) {
		super(parent);
		config = new Configuration(Soar2D.config);
		playerConfig = new PlayerConfig();
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
		
		Iterator<PlayerConfig> iter = config.players.iterator();
		while (iter.hasNext()) {
			TreeItem agent = new TreeItem(agents, SWT.NONE);
			agent.setText(iter.next().getName());
		}

		TreeItem terminals = new TreeItem(tree, SWT.NONE);
		terminals.setText(kTerminals);
		
		TreeItem clients = new TreeItem(tree, SWT.NONE);
		clients.setText(kClients);

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
			if (parentItem == null) {
				parentItem = selectedItem;
				selectedItem = null;
			}
			switch (tree.indexOf(parentItem)) {
			case 0:
				generalPage();
				break;
			case 1:
				loggingPage();
				break;
			case 2:
				agentsPage(selectedItem);
				break;
			case 3:
				terminalsPage();
				break;
			case 4:
				clientsPage(selectedItem);
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
		{
			Button gui = new Button(currentPage, SWT.CHECK);
			gui.setText("Use GUI");
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = SWT.BEGINNING;
				gd.horizontalSpan = 3;
				gui.setLayoutData(gd);
			}
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
			
		seedLabel = new Label(currentPage, SWT.NONE);
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
		hide.setSelection(config.noWorld);
		useSeed.setSelection(!config.random);
		seedLabel.setEnabled(useSeed.getSelection());
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
			
			loggingNameLabel = new Label(targetsGroup, SWT.NONE);
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
		loggingNameLabel.setEnabled(config.logToFile);
		loggingNameText.setEnabled(config.logToFile);
		loggingNameText.setText(config.logFile.getAbsolutePath());
		loggingConsoleButton.setSelection(config.logConsole);
	}
	
	public void agentsPage(TreeItem selectedItem) {
		
		// lists all intended agents
		// create new agent, setting its properties
		// for each agent: create shutdown command
		// global spawn debugger setting
		if (currentPage != null) {
			currentPage.dispose();
		}
		
		if (selectedItem == null) {
			
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
			
			{
				Group newAgentGroup = new Group(currentPage, SWT.NONE);
				newAgentGroup.setText("Create a new default agent");
				{
					GridData gd = new GridData();
					gd.grabExcessHorizontalSpace = true;
					gd.horizontalAlignment = SWT.FILL;
					gd.verticalAlignment = SWT.TOP;
					newAgentGroup.setLayoutData(gd);
					
					GridLayout gl = new GridLayout();
					gl.numColumns = 2;
					newAgentGroup.setLayout(gl);
				}
				
				Label nameLabel = new Label(newAgentGroup, SWT.NONE);
				nameLabel.setText("Name:");
				{
					GridData gd = new GridData();
					gd.horizontalSpan = 2;
					nameLabel.setLayoutData(gd);
				}
				
				agentsNameText = new Text(newAgentGroup, SWT.SINGLE | SWT.BORDER);
				agentsNameText.addFocusListener(new FocusAdapter() {
					public void focusLost(FocusEvent e) {
						playerConfig.setName(agentsNameText.getText());
						agentsUpdate();
					}
				});
				{
					GridData gd = new GridData();
					gd.horizontalAlignment = GridData.FILL;
					gd.grabExcessHorizontalSpace = true;
					gd.horizontalSpan = 2;
					agentsNameText.setLayoutData(gd);
				}
				
				Label productionsLabel = new Label(newAgentGroup, SWT.NONE);
				productionsLabel.setText("Productions:");
				{
					GridData gd = new GridData();
					gd.horizontalSpan = 2;
					productionsLabel.setLayoutData(gd);
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
						
						agentsUpdate();
					}
				});
				{
					GridData gd = new GridData();
					gd.horizontalAlignment = GridData.FILL;
					gd.grabExcessHorizontalSpace = true;
					agentsProductionsText.setLayoutData(gd);
				}
				
				Button productionsBrowse = new Button(newAgentGroup, SWT.PUSH);
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
						fd.setFilterPath(config.getBasePath());
						fd.setFileName(config.logFile.getName());
						fd.setFilterExtensions(new String[] {"*.*"});
						String productionsString = fd.open();
						File productionsFile = null;
						if (productionsString != null) {
							productionsFile = new File(productionsString);
						}
						playerConfig.setProductions(productionsFile);
						
						agentsUpdate();
					}
				});
				
				Label colorLabel = new Label(newAgentGroup, SWT.NONE);
				colorLabel.setText("Color:");
				{
					GridData gd = new GridData();
					gd.horizontalSpan = 2;
					colorLabel.setLayoutData(gd);
				}
				
				agentsColorCombo = new Combo(newAgentGroup, SWT.READ_ONLY);
				{
					GridData gd = new GridData();
					gd.horizontalSpan = 2;
					gd.horizontalAlignment = GridData.FILL;
					gd.grabExcessHorizontalSpace = true;
					agentsColorCombo.setLayoutData(gd);
				}
				agentsColorCombo.setItems(Soar2D.simulation.kColors);
				agentsColorCombo.setVisibleItemCount(Soar2D.simulation.kColors.length);
				agentsColorCombo.select(0);
				// TODO: only make unused colors available
				agentsColorCombo.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						playerConfig.setColor(Soar2D.simulation.kColors[agentsColorCombo.getSelectionIndex()]);
						
						agentsUpdate();
					}
				});
			}
			
			// spawn debuggers
			agentsDebuggersButton = new Button(currentPage, SWT.CHECK);
			agentsDebuggersButton.setText("Spawn debuggers on agent creation");
			agentsDebuggersButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					config.debuggers = !config.debuggers;
					agentsUpdate();
				}
			});
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = SWT.BEGINNING;
				agentsDebuggersButton.setLayoutData(gd);
			}
		} else {
			
		}
	
		agentsUpdate();

		rhs.layout(true);
		dialog.layout(true);
	}
	
	public void agentsUpdate() {
		
	}
	
	public void terminalsPage() {
		// select terminal states to use
		// adjust parameters
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
		
		
		rhs.layout(true);
		dialog.layout(true);
	}
	
	public void clientsPage(TreeItem selectedItem) {
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
			gl.numColumns = 3;
			currentPage.setLayout(gl);
			
			GridData gd = new GridData();
			gd.grabExcessHorizontalSpace = true;
			gd.grabExcessVerticalSpace = true;
			gd.horizontalAlignment = SWT.FILL;
			gd.verticalAlignment = SWT.FILL;
			currentPage.setLayoutData(gd);
		}
		
		
		rhs.layout(true);
		dialog.layout(true);
		
	}
}
