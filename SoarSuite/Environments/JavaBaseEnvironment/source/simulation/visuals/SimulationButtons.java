package simulation.visuals;

import java.util.logging.*;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;
import simulation.*;
import utilities.*;

public class SimulationButtons extends Composite {
	protected Button m_RunButton;
	protected Button m_StopButton;
	protected Button m_StepButton;
	protected Button m_ResetButton;
//	protected Text m_RunsText;
	private static Logger logger = Logger.getLogger("simulation");
	
	private Simulation m_Simulation;
	
	public SimulationButtons(Composite parent, Simulation simulation) {
		super(parent, SWT.NONE);
		
		m_Simulation = simulation;

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
				m_Simulation.startSimulation(true);
			}
		});
		
		m_StopButton = new Button(this, SWT.PUSH);
		m_StopButton.setText("Stop");
		m_StopButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.setRuns(0);
				m_Simulation.stopSimulation();
			}
		});
		
		m_StepButton = new Button(this, SWT.PUSH);
		m_StepButton.setText("Step");
		m_StepButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.stepSimulation();
			}
		});
		
		m_ResetButton = new Button(this, SWT.PUSH);
		m_ResetButton.setText("Reset");
		m_ResetButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.resetSimulation(true);
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
	}
}
