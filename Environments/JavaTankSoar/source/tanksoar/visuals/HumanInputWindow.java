package tanksoar.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import tanksoar.*;
import utilities.Direction;

public class HumanInputWindow {
	
	MoveInfo m_Move = new MoveInfo();
	
	public HumanInputWindow(Shell parent, final TankSoarSimulation simulation) {
		m_Move.reset();
		
		final Shell dialog = new Shell(parent, SWT.DIALOG_TRIM | SWT.APPLICATION_MODAL);
		dialog.setLayout(new FillLayout());
		dialog.setText("Human Input");
		
		Composite main = new Composite(dialog, SWT.NONE);
		GridLayout gl = new GridLayout();
		gl.numColumns = 3;
		main.setLayout(gl);
		
		final Button rotl = new Button(main, SWT.CHECK);
		rotl.setText("rotl");
		rotl.setLayoutData(new GridData());
		rotl.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Move.rotate = rotl.getSelection();
				m_Move.rotateDirection = Tank.kLeftID;
			}
		});		
		
		final Button north = new Button(main, SWT.CHECK);
		north.setText("north");
		north.setLayoutData(new GridData());
		
		final Button rotr = new Button(main, SWT.CHECK);
		rotr.setText("rotr");
		rotr.setLayoutData(new GridData());
		rotr.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Move.rotate = rotr.getSelection();
				m_Move.rotateDirection = Tank.kRightID;
			}
		});		
	
		final Button west = new Button(main, SWT.CHECK);
		west.setText("west");
		west.setLayoutData(new GridData());
	
		final Button fire = new Button(main, SWT.CHECK);
		fire.setText("fire");
		fire.setLayoutData(new GridData());
		fire.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Move.fire = fire.getSelection();
			}
		});		

		final Button east = new Button(main, SWT.CHECK);
		east.setText("east");
		east.setLayoutData(new GridData());
		
		final Button shields = new Button(main, SWT.CHECK);
		shields.setText("shields");
		shields.setLayoutData(new GridData());
		shields.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Move.shields = true;
				m_Move.shieldsSetting = shields.getSelection();
			}
		});		
		
		final Button south = new Button(main, SWT.CHECK);
		south.setText("south");
		south.setLayoutData(new GridData());
		
		final Button radar = new Button(main, SWT.CHECK);
		radar.setText("radar");
		radar.setLayoutData(new GridData());
		radar.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Move.radar = true;
				m_Move.radarSwitch = radar.getSelection();
			}
		});		
		
		north.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				west.setSelection(false);
				east.setSelection(false);
				south.setSelection(false);
				m_Move.move = north.getSelection();
				m_Move.moveDirection = Direction.kNorthInt;
			}
		});		
		south.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				north.setSelection(false);
				west.setSelection(false);
				east.setSelection(false);
				m_Move.move = south.getSelection();
				m_Move.moveDirection = Direction.kSouthInt;
			}
		});		
		west.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				north.setSelection(false);
				east.setSelection(false);
				south.setSelection(false);
				m_Move.move = west.getSelection();
				m_Move.moveDirection = Direction.kWestInt;
			}
		});		
		east.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				north.setSelection(false);
				west.setSelection(false);
				south.setSelection(false);
				m_Move.move = east.getSelection();
				m_Move.moveDirection = Direction.kEastInt;
			}
		});		

		final Button stop = new Button(main, SWT.CHECK);
		stop.setText("Stop Soar after step");
		GridData gd = new GridData();
		gd.horizontalSpan = 2;
		stop.setLayoutData(gd);

		final Button ok = new Button(main, SWT.PUSH);
		ok.setText("OK");
		ok.setLayoutData(new GridData());
		ok.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				if (stop.getSelection()) {
					simulation.setRuns(0);
					simulation.stopSimulation();
				}
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
	
	public MoveInfo getMove() {
		return m_Move;
	}
}
