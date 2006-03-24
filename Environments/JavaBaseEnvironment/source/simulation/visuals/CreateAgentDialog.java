package simulation.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import simulation.*;

public class CreateAgentDialog extends Dialog {

	final int kNameCharacterLimit = 12;
	SimulationManager m_Simulation;
	String m_Productions;
	Label m_ProductionsLabel;
	Text m_Name;
	Combo m_Color;
	Button m_CreateEater;
	
	public CreateAgentDialog(Shell parent, SimulationManager simulation) {
		super(parent);
		m_Simulation = simulation;
	}
	
	public void open() {
		Shell parent = getParent();
		final Shell dialog = new Shell(parent, SWT.DIALOG_TRIM | SWT.APPLICATION_MODAL);
		dialog.setText("Create Agent");
		
		GridLayout gl = new GridLayout();
		gl.numColumns = 3;
		dialog.setLayout(gl);

		GridData gd;
		
		final Label label2 = new Label(dialog, SWT.NONE);
		gd = new GridData();
		gd.horizontalAlignment = GridData.BEGINNING;
		label2.setLayoutData(gd);
		label2.setText("Productions:");
		
		m_ProductionsLabel = new Label(dialog, SWT.NONE);
		gd = new GridData();
		gd.grabExcessHorizontalSpace = true;
		gd.widthHint = 150;
		m_ProductionsLabel.setLayoutData(gd);
		m_ProductionsLabel.setText("<choose productions>");

		final Button productionsBrowse = new Button(dialog, SWT.PUSH);
		gd = new GridData();
		gd.horizontalAlignment = GridData.BEGINNING;
		productionsBrowse.setLayoutData(gd);
		productionsBrowse.setText("...");
		productionsBrowse.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				FileDialog fd = new FileDialog(dialog, SWT.OPEN);
				fd.setText("Open");
				fd.setFilterPath(m_Simulation.getAgentPath());
				fd.setFilterExtensions(new String[] {"*.soar", "*.*"});
				// TODO: these next commented out lines are going to cause a bug to reappear
				VisualWorld.internalRepaint = true;
				String productions = fd.open();
				VisualWorld.internalRepaint = false;
				if (productions != null) {
					m_Productions = productions;
					m_ProductionsLabel.setText(m_Productions.substring(m_Productions.lastIndexOf(System.getProperty("file.separator")) + 1));
				}
				updateButtons();
			}
		});
		
		final Label label4 = new Label(dialog, SWT.NONE);
		gd = new GridData();
		gd.horizontalAlignment = GridData.BEGINNING;
		label4.setLayoutData(gd);
		label4.setText("Color:");
		
		m_Color = new Combo(dialog, SWT.NONE);
		gd = new GridData();
		gd.horizontalSpan = 2;
		m_Color.setLayoutData(gd);
		m_Color.setItems(WindowManager.kColors);
		// remove taken colors
		WorldEntity[] entities = m_Simulation.getWorldManager().getEntities();
		if (entities != null) {
			for (int i = 0; i < entities.length; ++i) {
				m_Color.remove(entities[i].getColor());
			}
		}
		m_Color.select(0);
		m_Color.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Name.setText(m_Color.getText());
			}
		});
		
		final Label label1 = new Label(dialog, SWT.NONE);
		gd = new GridData();
		gd.horizontalAlignment = GridData.BEGINNING;
		label1.setLayoutData(gd);
		label1.setText("Name:");
		
		m_Name = new Text(dialog, SWT.SINGLE | SWT.BORDER);
		gd = new GridData();
		gd.horizontalSpan = 2;
		gd.widthHint = 150;
		gd.grabExcessHorizontalSpace = true;
		m_Name.setLayoutData(gd);
		m_Name.setTextLimit(kNameCharacterLimit);
		m_Name.setText(m_Color.getText());
		m_Name.addKeyListener(new KeyAdapter() {
			public void keyReleased(KeyEvent e) {
				updateButtons();
				if (Character.isWhitespace(e.character)) {
					e.doit = false;
				}
			}
		});
		
		Composite okCancel = new Composite(dialog, SWT.NONE);
		gd = new GridData();
		gd.horizontalAlignment = GridData.END;
		gd.horizontalSpan = 3;
		okCancel.setLayoutData(gd);
		okCancel.setLayout(new FillLayout());
		m_CreateEater = new Button(okCancel, SWT.PUSH);
		m_CreateEater.setText("Create Agent");
		m_CreateEater.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_CreateEater.setEnabled(false);
				m_Simulation.createEntity(m_Name.getText(), m_Productions, m_Color.getText(), null, null);
				dialog.dispose();
			}
		});

		Button cancel = new Button(okCancel, SWT.PUSH);
		cancel.setText("Cancel");
		cancel.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				dialog.dispose();
			}
		});
		
		updateButtons();

		dialog.setSize(dialog.computeSize(SWT.DEFAULT, SWT.DEFAULT, false));
		
		dialog.open();
		Display display = parent.getDisplay();
		while (!dialog.isDisposed()) {
			if (!display.readAndDispatch()) {
				display.sleep();
			}
		}
	}
	
	void updateButtons() {
		boolean productions = (m_Productions != null);
		boolean name = false;
	
		String nameText = m_Name.getText();
		if (nameText != null) {
			if (nameText.length() > 0) {
				name = true;
				for (int i = 0; i < nameText.length(); ++i) {
					if (Character.isWhitespace(nameText.charAt(i))) {
						name = false;
					}
				}			
			}
		}
		
		m_CreateEater.setEnabled(productions && name);
	}
}
