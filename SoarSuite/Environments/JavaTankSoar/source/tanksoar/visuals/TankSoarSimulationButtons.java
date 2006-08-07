package tanksoar.visuals;

import org.eclipse.swt.widgets.*;
import simulation.visuals.*;
import tanksoar.*;

public class TankSoarSimulationButtons extends SimulationButtons {
	TankSoarSimulation m_Simulation;
	
	public TankSoarSimulationButtons(Composite parent, TankSoarSimulation simulation) {
		super(parent, simulation);
		m_Simulation = simulation;
		updateButtons();
	}
	
	void updateButtons() {
		boolean running = m_Simulation.isRunning();
		boolean done = m_Simulation.getTankSoarWorld().getVictoryCondition();
		boolean tanks = (m_Simulation.getTankSoarWorld().getTanks().length != 0);
		
        m_RunButton.setEnabled(!running && !done && tanks);
        m_StopButton.setEnabled(running);
        m_ResetButton.setEnabled(!running);
        m_StepButton.setEnabled(!running && !done && tanks);
//        m_RunsText.setText(Integer.toString(m_Simulation.getRuns()));
	}
}
