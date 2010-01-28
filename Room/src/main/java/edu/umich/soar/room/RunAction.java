package edu.umich.soar.room;

import java.awt.event.ActionEvent;

public class RunAction extends AbstractGridmap2DAction {

	private static final long serialVersionUID = -3347861376925708892L;

	public RunAction(ActionManager manager) {
		super(manager, "Run");
	}

	@Override
	public void update() {
		boolean running = getApplication().getSim().isRunning();
		boolean players = !getApplication().getSim().getWorld().getPlayers().isEmpty();
        setEnabled(!running && players);
	}

	@Override
	public void actionPerformed(ActionEvent e) {
		getApplication().doRunForever(1);
	}

}
