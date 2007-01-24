package soar2d.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

public class ConfigurationEditor extends Dialog {

	Tree tree;
	Composite rhs;
	Composite currentPage;
	Shell dialog;
	
	public ConfigurationEditor(Shell parent) {
		super(parent);
	}

	public void open() {
		Shell parent = getParent();
		dialog = new Shell(parent, SWT.DIALOG_TRIM | SWT.APPLICATION_MODAL);
		dialog.setText("Configuration Editor");

		GridLayout gl = new GridLayout();
		gl.numColumns = 2;
		dialog.setLayout(gl);

		tree = new Tree(dialog, SWT.BORDER);
		{
			GridData gd = new GridData();
			gd.heightHint = 400;
			gd.widthHint = 100;
			tree.setLayoutData(gd);
		}
		tree.addListener(SWT.Selection, new Listener () {
			public void handleEvent (Event event) {
				TreeItem item = (TreeItem) event.item;
				if (item.getText().equals("General")) {
					generalPage();
				} else if (item.getText().equals("Logging")) {
					loggingPage();
				}
			}
		});

		TreeItem general = new TreeItem(tree, SWT.NONE);
		general.setText("General");
		
		TreeItem logging = new TreeItem(tree, SWT.NONE);
		logging.setText("Logging");
		
		TreeItem agents = new TreeItem(tree, SWT.NONE);
		agents.setText("Agents");
		// lists all intended agents
		// create new agent, setting its properties
		// for each agent: create shutdown command
		// global spawn debugger setting

		TreeItem clients = new TreeItem(tree, SWT.NONE);
		clients.setText("Clients");
		// lists all intended clients
		// create new clients, setting its properties

		TreeItem terminals = new TreeItem(tree, SWT.NONE);
		terminals.setText("Terminals");
		// select terminal states to use
		// adjust parameters
		
		rhs = new Composite(dialog, SWT.NONE);
		rhs.setLayout(new FillLayout());
		{
			GridData gd = new GridData();
			gd.widthHint = 400;
			rhs.setLayoutData(gd);
		}

		Composite saveAsCancel = new Composite(dialog, SWT.NONE);
		saveAsCancel.setLayout(new FillLayout());
		{
			GridData gd = new GridData();
			gd.horizontalSpan = 2;
			gd.horizontalAlignment = SWT.END;
			saveAsCancel.setLayoutData(gd);
		}
		
		Button saveAs = new Button(saveAsCancel, SWT.PUSH);
		saveAs.setText("Save as...");
		saveAs.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				dialog.dispose();
			}
		});
		
		Button cancel = new Button(saveAsCancel, SWT.PUSH);
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
	
	public void generalPage() {
		// eaters or tanksoar
		// map
		// use graphical display or not
		// display world or not
		// connect to remote kernel
		// random or deterministic
		// missile reset threshold (tanksoar)
		System.out.println("General!");
		
		if (currentPage != null) {
			currentPage.dispose();
		}
		currentPage = new Composite(rhs, SWT.NONE);
		currentPage.setLayout(new FillLayout());
		
		Label currentLabel = new Label(currentPage, SWT.NONE);
		currentLabel.setText("General!");
		
		rhs.layout(true);
		dialog.layout(true);
	}

	public void loggingPage() {
		// log level
		// log to file and/or console
		// log filename
		System.out.println("Logging!");
	
		if (currentPage != null) {
			currentPage.dispose();
		}
		currentPage = new Composite(rhs, SWT.NONE);
		currentPage.setLayout(new FillLayout());
		
		Label currentLabel = new Label(currentPage, SWT.NONE);
		currentLabel.setText("Logging!");
		
		rhs.layout(true);
		dialog.layout(true);
	}
}
