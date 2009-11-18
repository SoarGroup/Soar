package edu.umich.soar.room;

import java.awt.event.ActionEvent;

public class StepAction extends AbstractGridmap2DAction {

	private static final long serialVersionUID = 1812359437366074252L;

	public StepAction(ActionManager manager) {
		super(manager, "Step");
	}

	@Override
	public void update() {
		boolean running = getApplication().getSim().isRunning();
		boolean players = !getApplication().getSim().getWorld().getPlayers().isEmpty();
        setEnabled(!running && players);
	}

	@Override
	public void actionPerformed(ActionEvent e) {
		getApplication().doRunTick(1, 1);
	}

}
