package edu.umich.soar.gridmap2d.players;


import edu.umich.soar.gridmap2d.Direction;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Simulation;
import edu.umich.soar.gridmap2d.map.TankSoarMap;

public class Tank extends Player {
	public static class Builder {
		// Required parameters
		final String id;
		
		// Optional parameters
		int missiles = Gridmap2D.config.tanksoarConfig().max_missiles;
		int energy = Gridmap2D.config.tanksoarConfig().max_energy;
		int health = Gridmap2D.config.tanksoarConfig().max_health;
		
		public Builder(String id) {
			this.id = id;
		}
		
		public Builder missiles(int missiles) {
			this.missiles = missiles;
			return this;
		}
		
		public Builder energy(int energy) {
			this.energy = energy;
			return this;
		}
		
		public Builder health(int health) {
			this.health = health;
			return this;
		}
		
		public Tank build() {
			return new Tank(this);
		}
	}
	
	private TankCommander commander;
	private TankState state;

	private Tank(Builder builder) {
		super(builder.id);

		this.state = new TankState(getName(), builder);

		state.clearRadar(); // creates the radar structure
		reset();
	}
	
	public void setCommander(TankCommander commander) {
		this.commander = commander;
	}
	
	public CommandInfo getCommand() {
		CommandInfo command;
		if (commander != null) {
			command = commander.nextCommand();
		} else {
			command = Gridmap2D.control.getHumanCommand(this);
		}
		
		return command;
	}
	
	@Override
	public void reset() {
		super.reset();

		if (state != null) {
			state.reset();
		}

		if (commander != null) {
			commander.reset();
		}
	}
	
	public TankState getState() {
		return state;
	}

	public void update(int [] newLocation, TankSoarMap tankSoarMap) {
		super.update(newLocation);
		
		if (state.getRadarSwitch()) {
			state.setObservedPower(tankSoarMap.getRadar(state.getRadar(), newLocation, getFacing(), state.getRadarPower()));
		} else {
			state.clearRadar();
			state.setObservedPower(0);
		}
		
		state.setBlocked(tankSoarMap.getBlocked(newLocation));
		
		if (commander != null) {
			commander.update(tankSoarMap);
		}
}
	
	public void fragged() {
		state.fragged();
		setFacing(Direction.values()[Simulation.random.nextInt(4) + 1]);
		if (commander != null) {
			commander.fragged();
		}
	}
	
	public void playersChanged(Player[] players) {
		if (commander != null) {
			commander.playersChanged(players);
		}
	}

	public void commit(int[] location) {
		if (commander != null) {
			commander.commit();
		}
	}

	public void shutdownCommander() {
		if (commander != null) {
			commander.shutdown();
		}
	}
}
