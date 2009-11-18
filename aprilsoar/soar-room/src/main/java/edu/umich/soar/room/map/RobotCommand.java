package edu.umich.soar.room.map;

import edu.umich.soar.robot.DifferentialDriveCommand;
import edu.umich.soar.room.core.Names;

public class RobotCommand {
	public static final RobotCommand NULL = new Builder(DifferentialDriveCommand.newEStopCommand()).build();

	public static class Builder {
		private final DifferentialDriveCommand ddc;
		private boolean stopSim;
		
		public Builder(DifferentialDriveCommand ddc) {
			this.ddc = ddc;
		}
		
		public Builder stopSim() {
			this.stopSim = true;
			return this;
		}
		
		public RobotCommand build() {
			return new RobotCommand(this);
		}
	}
	
	private final DifferentialDriveCommand ddc;
	private final boolean stopSim;
	
	private RobotCommand(Builder builder) {
		this.ddc = builder.ddc;
		this.stopSim = builder.stopSim;
	}
	
	public DifferentialDriveCommand getDdc() {
		return ddc;
	}

	public boolean isStopSim() {
		return stopSim;
	}

	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder();
		if (ddc != null) {
			sb.append(ddc);
		}
		Commands.memberAppend(sb, isStopSim(), Names.kStopSimID);
		return sb.toString();
	}
}
