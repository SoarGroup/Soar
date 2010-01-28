package edu.umich.soar.room;

import org.flexdock.docking.DockingConstants;

//import edu.umich.soar.gridmap2d.core.Simulation;

public class SimulationControlView extends AbstractAdaptableView implements Disposable, Refreshable {
	
	private static final long serialVersionUID = 5460202126006667600L;
	
	
	//private final Simulation sim;
	private final SimulationControlPanel panel;
	
    public SimulationControlView(Adaptable app) {
        super("simulationControlView", "Simulation Control");
        addAction(DockingConstants.PIN_ACTION);
        addAction(DockingConstants.CLOSE_ACTION);

        //this.sim = Adaptables.adapt(app, Simulation.class);

        this.panel = new SimulationControlPanel(app, getPreferences());
        setContentPane(this.panel);
    }

	@Override
	public void refresh() {
		this.panel.updateCounts();
		
		this.panel.repaint();
	}
	

	@Override
	public void dispose() {
		this.panel.dispose();
	}
}