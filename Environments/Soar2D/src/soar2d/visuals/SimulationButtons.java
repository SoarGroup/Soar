package soar2d.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import soar2d.*;

public class SimulationButtons extends Composite {
	protected Button m_RunButton;
	protected Button m_StopButton;
	protected Button m_StepButton;
	protected Button m_ResetButton;

	public SimulationButtons(Composite parent) {
		super(parent, SWT.NONE);
		
		setLayout(new FillLayout());
				
		m_RunButton = new Button(this, SWT.PUSH);
		m_RunButton.setText("Run");
		m_RunButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				Soar2D.control.startSimulation(false, true);
			}
		});
		
		m_StopButton = new Button(this, SWT.PUSH);
		m_StopButton.setText("Stop");
		m_StopButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				Soar2D.wm.setVisualWorldFocus();
				if (Soar2D.wm.humanMove != null) {
					synchronized(Soar2D.wm.humanMove) {
						Soar2D.wm.humanMove.notify();
						Soar2D.wm.humanMove = null;
					}
				}
				Soar2D.control.stopSimulation();
			}
		});
		
		m_StepButton = new Button(this, SWT.PUSH);
		m_StepButton.setText("Step");
		m_StepButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				Soar2D.control.startSimulation(true, true);
			}
		});
		
		m_ResetButton = new Button(this, SWT.PUSH);
		m_ResetButton.setText("Reset");
		m_ResetButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				Soar2D.control.resetSimulation();
			}
		});
	
		updateButtons();
	}
	
	void updateButtons() {
		boolean running = Soar2D.control.isRunning();
		boolean done = Soar2D.simulation.isDone();
		boolean eaters = Soar2D.simulation.hasPlayers();
		
        m_RunButton.setEnabled(!running && !done && eaters);
        m_StopButton.setEnabled(running);
        m_ResetButton.setEnabled(!running);
        m_StepButton.setEnabled(!running && !done && eaters);
	}
}
