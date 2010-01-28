package edu.umich.soar.room;

import java.awt.event.ActionEvent;

public class GoAction extends AbstractGridmap2DAction {

	private static final long serialVersionUID = -3347861376925708892L;

	private GoProvider gop;
	
	public GoAction(ActionManager manager) {
		super(manager, "Go");
	}
	
	public interface GoProvider {
		public boolean isRunForever();
		public int getQuantity();
		public double getTimeScale();
	}
	
	public void setGoProvider(GoProvider gop) {
		this.gop = gop;
	}

	@Override
	public void update() {
		boolean running = getApplication().getSim().isRunning();
		boolean players = !getApplication().getSim().getWorld().getPlayers().isEmpty();
        setEnabled(!running && players);
	}

	@Override
	public void actionPerformed(ActionEvent e) {
		if (gop == null) {
			getApplication().doRunForever(gop.getTimeScale());
		} else {
			if (gop.isRunForever()) {
				getApplication().doRunForever(gop.getTimeScale());
			} else {
				getApplication().doRunTick(gop.getQuantity(), gop.getTimeScale());
			}
		}
	}

}
