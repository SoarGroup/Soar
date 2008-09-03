package eaters.visuals;

import org.eclipse.swt.widgets.*;
import eaters.*;
import simulation.visuals.*;

public class EatersSimulationButtons extends SimulationButtons {
	
	EatersSimulation m_Simulation;
	
	public EatersSimulationButtons(Composite parent, EatersSimulation simulation) {
		super(parent, simulation);
		m_Simulation = simulation;
		updateButtons();
	}
	
	void updateButtons() {
		boolean running = m_Simulation.isRunning();
		boolean done = (EatersCell.getFoodCount() == 0);
		boolean eaters = (m_Simulation.getEatersWorld().getEaters().length > 0);
		
        m_RunButton.setEnabled(!running && !done && eaters);
        m_StopButton.setEnabled(running);
        m_ResetButton.setEnabled(!running);
        m_StepButton.setEnabled(!running && !done && eaters);
//        m_RunsText.setText(Integer.toString(m_Simulation.getRuns()));
	}
}
