package edu.umich.soar.gridmap2d.players;


import edu.umich.soar.gridmap2d.Direction;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Simulation;
import edu.umich.soar.gridmap2d.map.TankSoarMap;

public class Tank extends Player {
	private TankCommander commander;
	
	private TankState state;

	public Tank(String playerId, int radarWidth, int radarHeight, 
			int initialMissiles, int initialEnergy, int initialHealth, 
			int defaultMissiles, int defaultEnergy, int defaultHealth) throws Exception {
		super(playerId);

		this.state = new TankState(getName(), radarWidth, radarHeight, 
				initialMissiles, initialEnergy, initialHealth, 
				defaultMissiles, defaultEnergy, defaultHealth);

		state.clearRadar(); // creates the radar structure
		reset();
	}
	
	public void setCommander(TankCommander commander) {
		this.commander = commander;
	}
	
	public CommandInfo getCommand() throws Exception {
		CommandInfo command;
		if (commander != null) {
			command = commander.nextCommand();
		} else {
			command = Gridmap2D.control.getHumanCommand(this);
		}
		
		return command;
	}
	
	@Override
	public void reset() throws Exception {
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

	public void update(int [] newLocation, TankSoarMap tankSoarMap) throws Exception {
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
	
	public void playersChanged(Player[] players) throws Exception {
		if (commander != null) {
			commander.playersChanged(players);
		}
	}

	public void commit(int[] location) throws Exception {
		if (commander != null) {
			commander.commit();
		}
	}

	public void shutdownCommander() throws Exception {
		if (commander != null) {
			commander.shutdown();
		}
	}
}
