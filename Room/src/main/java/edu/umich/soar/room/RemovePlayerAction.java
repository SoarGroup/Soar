package edu.umich.soar.room;

import java.awt.event.ActionEvent;
import java.util.List;

import edu.umich.soar.room.map.Robot;

public class RemovePlayerAction extends AbstractGridmap2DAction {

	private static final long serialVersionUID = 1235579020814912299L;

	public RemovePlayerAction(ActionManager manager) {
		super(manager, "Remove Player");
	}

	@Override
	public void update() {
        setEnabled(!getApplication().getSim().isRunning());
	}

	@Override
	public void actionPerformed(ActionEvent e) {
		List<Robot> players = Adaptables.adaptCollection(getApplication().getSelectionManager().getSelection(), Robot.class);
		if (players.isEmpty()) {
			return;
		}
		for (Robot player : players) {
			getApplication().getSim().destroyPlayer(player);
		}
	}

}
