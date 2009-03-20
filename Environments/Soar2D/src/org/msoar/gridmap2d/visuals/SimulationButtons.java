package org.msoar.gridmap2d.visuals;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.msoar.gridmap2d.Gridmap2D;


public class SimulationButtons extends Composite {
	protected Button m_RunButton;
	protected Button m_StopButton;
	protected Button m_StepButton;
	protected Button m_ResetButton;

	public SimulationButtons(Composite parent, int numberOfPlayers) {
		super(parent, SWT.NONE);
		
		setLayout(new FillLayout());
				
		m_RunButton = new Button(this, SWT.PUSH);
		m_RunButton.setText("Run");
		m_RunButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				Gridmap2D.control.startSimulation(false, true);
			}
		});
		
		m_StopButton = new Button(this, SWT.PUSH);
		m_StopButton.setText("Stop");
		m_StopButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				Gridmap2D.wm.setVisualWorldFocus();
				if (Gridmap2D.wm.humanMove != null) {
					synchronized(Gridmap2D.wm.humanMove) {
						Gridmap2D.wm.humanMove.notify();
						Gridmap2D.wm.humanMove = null;
					}
				}
				Gridmap2D.control.stopSimulation();
			}
		});
		
		m_StepButton = new Button(this, SWT.PUSH);
		m_StepButton.setText("Step");
		m_StepButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				Gridmap2D.control.startSimulation(true, true);
			}
		});
		
		m_ResetButton = new Button(this, SWT.PUSH);
		m_ResetButton.setText("Reset");
		m_ResetButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				Gridmap2D.control.resetSimulation();
			}
		});
	
		updateButtons(numberOfPlayers);
	}
	
	void updateButtons(int numberOfPlayers) {
		boolean running = Gridmap2D.control.isRunning();
		boolean done = Gridmap2D.simulation.isDone();
		boolean eaters = numberOfPlayers > 0;
		
        m_RunButton.setEnabled(!running && !done && eaters);
        m_StopButton.setEnabled(running);
        m_ResetButton.setEnabled(!running);
        m_StepButton.setEnabled(!running && !done && eaters);
	}
}
