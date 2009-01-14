package soar2d.visuals;

import java.util.ArrayList;
import java.io.*;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import soar2d.*;
import soar2d.config.Config;
import soar2d.config.Soar2DKeys;
import soar2d.player.*;

public class CreateAgentDialog extends Dialog {
	final int kNameCharacterLimit = 12;
	File m_Productions;
	Label m_ProductionsLabel;
	Text m_Name;
	Combo m_Color;
	Button m_CreateEntity;
	static File lastProductions = null;
	Button m_SpawnDebuggerButton;
	static int guiPlayer = 0;
	
	public CreateAgentDialog(Shell parent) {
		super(parent);
	}
	
	public void open() {
		Shell parent = getParent();
		final Shell dialog = new Shell(parent, SWT.DIALOG_TRIM | SWT.APPLICATION_MODAL);
		dialog.setText("Create Agent");
		
		GridLayout gl = new GridLayout();
		gl.numColumns = 4;
		dialog.setLayout(gl);

		final Label label2 = new Label(dialog, SWT.NONE);
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = GridData.BEGINNING;
			label2.setLayoutData(gd);
		}
		label2.setText("Productions:");
		
		m_ProductionsLabel = new Label(dialog, SWT.NONE);
		{
			GridData gd = new GridData();
			gd.grabExcessHorizontalSpace = true;
			gd.widthHint = 150;
			m_ProductionsLabel.setLayoutData(gd);
		}
		if (lastProductions == null) {
			m_ProductionsLabel.setText(Names.kHumanProductions);
		} else {
			m_Productions = lastProductions;
			m_ProductionsLabel.setText(lastProductions.getName());
		}

		final Button productionsBrowse = new Button(dialog, SWT.PUSH);
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = GridData.BEGINNING;
			productionsBrowse.setLayoutData(gd);
		}
		productionsBrowse.setText("Soar");
		productionsBrowse.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				FileDialog fd = new FileDialog(dialog, SWT.OPEN);
				fd.setText("Open");
				fd.setFilterPath(Soar2D.simulation.getAgentPath());
				if (lastProductions == null) {
				} else {
					fd.setFileName(lastProductions.getAbsolutePath());
				}
				fd.setFilterExtensions(new String[] {"*.soar", "*.*"});
				// TODO: these next commented out lines are going to cause a bug to reappear
				VisualWorld.internalRepaint = true;
				String productions = fd.open();
				VisualWorld.internalRepaint = false;
				if (productions != null) {
					File file = new File(productions);
					m_Productions = file;
					lastProductions = file;
					m_ProductionsLabel.setText(file.getName());
				}
				updateButtons();
			}
		});
		
		final Button human = new Button(dialog, SWT.PUSH);
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = GridData.BEGINNING;
			human.setLayoutData(gd);
		}
		human.setText("Human");
		human.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				lastProductions = null;
				m_Productions = null;
				m_ProductionsLabel.setText(Names.kHumanProductions);
				Soar2D.config.setBoolean(Soar2DKeys.general.soar.spawn_debuggers, true);
				updateButtons();
			}
		});
		
		final Label label4 = new Label(dialog, SWT.NONE);
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = GridData.BEGINNING;
			label4.setLayoutData(gd);
		}
		label4.setText("Color:");
		
		m_Color = new Combo(dialog, SWT.NONE);
		{
			GridData gd = new GridData();
			gd.horizontalSpan = 3;
			m_Color.setLayoutData(gd);
		}
		ArrayList<String> unusedColors = Soar2D.simulation.getUnusedColors();
		
		m_Color.setItems(unusedColors.toArray(new String[0]));
		if (unusedColors.size() > 0) {
			m_Color.select(0);
			m_Color.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					m_Name.setText(m_Color.getText());
				}
			});
		}
		
		final Label label1 = new Label(dialog, SWT.NONE);
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = GridData.BEGINNING;
			label1.setLayoutData(gd);
		}
		label1.setText("Name:");
		
		m_Name = new Text(dialog, SWT.SINGLE | SWT.BORDER);
		{
			GridData gd = new GridData();
			gd.horizontalSpan = 3;
			gd.widthHint = 150;
			gd.grabExcessHorizontalSpace = true;
			m_Name.setLayoutData(gd);
		}
		m_Name.setTextLimit(kNameCharacterLimit);
		m_Name.setText(m_Color.getText());
		m_Name.addKeyListener(new KeyAdapter() {
			public void keyPressed(KeyEvent e) {
				if (Character.isWhitespace(e.character)) {
					e.doit = false;
				}
			}
			public void keyReleased(KeyEvent e) {
				updateButtons();
			}
		});
		
		m_SpawnDebuggerButton = new Button(dialog, SWT.CHECK);
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = GridData.BEGINNING;
			gd.horizontalSpan = 4;
			m_SpawnDebuggerButton.setLayoutData(gd);
		}
		m_SpawnDebuggerButton.setText("Spawn debugger");
		m_SpawnDebuggerButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				Soar2D.config.setBoolean(Soar2DKeys.general.soar.spawn_debuggers, m_SpawnDebuggerButton.getSelection());
			}
		});		
		m_SpawnDebuggerButton.setSelection(Soar2D.config.getBoolean(Soar2DKeys.general.soar.spawn_debuggers, false));

		Composite okCancel = new Composite(dialog, SWT.NONE);
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = GridData.END;
			gd.horizontalSpan = 4;
			okCancel.setLayoutData(gd);
		}
		okCancel.setLayout(new FillLayout());
		m_CreateEntity = new Button(okCancel, SWT.PUSH);
		m_CreateEntity.setText("Create Agent");
		m_CreateEntity.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_CreateEntity.setEnabled(false);

				// create id
				String playerId = "gui" + Integer.toString(++guiPlayer);
				
				// create configuration harness
				Config playerConfig = Soar2D.config.getChild(Soar2DKeys.playerKey(playerId));
				
				playerConfig.setString(Soar2DKeys.players.name, m_Name.getText());
				if (m_Productions != null) {
					playerConfig.setString(Soar2DKeys.players.productions, m_Productions.getAbsolutePath());
				}
				playerConfig.setString(Soar2DKeys.players.color, m_Color.getText());
				Soar2D.simulation.createPlayer(playerId);
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
		boolean debuggerConnected = Soar2D.simulation.isClientConnected(Names.kDebuggerClient);
	
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
		
		m_SpawnDebuggerButton.setEnabled(!debuggerConnected && productions);
		m_CreateEntity.setEnabled(name);
	}
}
