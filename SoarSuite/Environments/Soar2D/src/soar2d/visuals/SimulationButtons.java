package soar2d.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import soar2d.*;
import soar2d.world.Cell;

public class SimulationButtons extends Composite {
	protected Button m_RunButton;
	protected Button m_StopButton;
	protected Button m_StepButton;
	protected Button m_ResetButton;
	Controller control = Soar2D.control;

	public SimulationButtons(Composite parent) {
		super(parent, SWT.NONE);
		
		setLayout(new FillLayout());
				
		m_RunButton = new Button(this, SWT.PUSH);
		m_RunButton.setText("Run");
		m_RunButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
//				int input = 0;
//				try {
//					input = Integer.valueOf(m_RunsText.getText()).intValue();
//				} catch (NumberFormatException exception) {
//					if (m_RunsText.getText().equalsIgnoreCase("forever")) {
//						m_RunsText.setText("-1");
//						input = -1;
//					}
//					m_RunsText.setText("0");
//				}
//				if (input < 0) {
//					input = -1;
//				}
//				m_Simulation.setRuns(input);
				control.startSimulation(true);
			}
		});
		
		m_StopButton = new Button(this, SWT.PUSH);
		m_StopButton.setText("Stop");
		m_StopButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				control.stopSimulation();
			}
		});
		
		m_StepButton = new Button(this, SWT.PUSH);
		m_StepButton.setText("Step");
		m_StepButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				control.stepSimulation();
			}
		});
		
		m_ResetButton = new Button(this, SWT.PUSH);
		m_ResetButton.setText("Reset");
		m_ResetButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				control.resetSimulation();
			}
		});
		
//		Label runsLabel = new Label(this, SWT.NONE);
//		gd = new GridData();
//		gd.horizontalSpan = 2;
//		gd.horizontalAlignment = SWT.END;
//		runsLabel.setLayoutData(gd);
//		runsLabel.setText("Runs:");
//		
//		m_RunsText = new Text(this, SWT.BORDER);
//		gd = new GridData();
//		gd.horizontalSpan = 2;
//		gd.horizontalSpan = 2;
//		gd.widthHint = 67;
//		m_RunsText.setLayoutData(gd);
//		m_RunsText.setTextLimit(10);

		updateButtons();
	}
	
	void updateButtons() {
		boolean running = control.isRunning();
		boolean done = (Cell.getFoodCount() == 0);
		boolean eaters = control.hasEntities();
		
        m_RunButton.setEnabled(!running && !done && eaters);
        m_StopButton.setEnabled(running);
        m_ResetButton.setEnabled(!running);
        m_StepButton.setEnabled(!running && !done && eaters);
//        m_RunsText.setText(Integer.toString(m_Simulation.getRuns()));
	}
}
