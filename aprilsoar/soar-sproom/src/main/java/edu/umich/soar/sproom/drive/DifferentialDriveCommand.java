package edu.umich.soar.sproom.drive;

public class DifferentialDriveCommand {
	private static final DifferentialDriveCommand ESTOP = new DifferentialDriveCommand();

	public static DifferentialDriveCommand newEStopCommand() {
		return ESTOP;
	}
	
	public static DifferentialDriveCommand newMotorCommand(double left, double right) {
		return new SplinterDriveCommandBuilder(CommandType.MOTOR).left(left).right(right).build();
	}
	
	public static DifferentialDriveCommand newVelocityCommand(double angularVelocity, double linearVelocity) {
		return new SplinterDriveCommandBuilder(CommandType.VEL).angularVelocity(angularVelocity).linearVelocity(linearVelocity).build();
	}
	
	public static DifferentialDriveCommand newAngularVelocityCommand(double angularVelocity) {
		return new SplinterDriveCommandBuilder(CommandType.ANGVEL).angularVelocity(angularVelocity).build();
	}
	
	public static DifferentialDriveCommand newLinearVelocityCommand(double linearVelocity) {
		return new SplinterDriveCommandBuilder(CommandType.LINVEL).linearVelocity(linearVelocity).build();
	}
	
	public static DifferentialDriveCommand newHeadingCommand(double heading) {
		return new SplinterDriveCommandBuilder(CommandType.HEADING).heading(heading).build();
	}
	
	public static DifferentialDriveCommand newHeadingLinearVelocityCommand(double heading, double linearVelocity) {
		return new SplinterDriveCommandBuilder(CommandType.HEADING_LINVEL).heading(heading).linearVelocity(linearVelocity).build();
	}
	
	public static DifferentialDriveCommand newMoveToCommand(double x, double y, double theta) {
		return new SplinterDriveCommandBuilder(CommandType.MOVE_TO).x(x).y(y).theta(theta).build();
	}
	
	private final static class SplinterDriveCommandBuilder {
		private final CommandType type;
		private double arg0 = 0;
		private double arg1 = 0;
		private double arg2 = 0;
		
		private SplinterDriveCommandBuilder(CommandType mode) {
			this.type = mode;
		}
		private SplinterDriveCommandBuilder left(double left) {
			if (type != CommandType.MOTOR) {
				throw new IllegalArgumentException();
			}
			this.arg0 = left;
			return this;
		}
		private SplinterDriveCommandBuilder right(double right) {
			if (type != CommandType.MOTOR) {
				throw new IllegalArgumentException();
			}
			this.arg1 = right;
			return this;
		}
		private SplinterDriveCommandBuilder angularVelocity(double av) {
			if (type != CommandType.ANGVEL && type != CommandType.LINVEL && type != CommandType.VEL) {
				throw new IllegalArgumentException();
			}
			this.arg0 = av;
			return this;
		}
		private SplinterDriveCommandBuilder linearVelocity(double lv) {
			if (type != CommandType.ANGVEL && type != CommandType.LINVEL && type != CommandType.VEL && type != CommandType.HEADING_LINVEL) {
				throw new IllegalArgumentException();
			}
			this.arg1 = lv;
			return this;
		}
		private SplinterDriveCommandBuilder heading(double heading) {
			if (type != CommandType.HEADING && type != CommandType.HEADING_LINVEL) {
				throw new IllegalArgumentException();
			}
			this.arg2 = heading;
			return this;
		}
		private SplinterDriveCommandBuilder x(double x) {
			if (type != CommandType.MOVE_TO) {
				throw new IllegalArgumentException();
			}
			this.arg0 = x;
			return this;
		}
		private SplinterDriveCommandBuilder y(double y) {
			if (type != CommandType.MOVE_TO) {
				throw new IllegalArgumentException();
			}
			this.arg1 = y;
			return this;
		}
		private SplinterDriveCommandBuilder theta(double theta) {
			if (type != CommandType.MOVE_TO) {
				throw new IllegalArgumentException();
			}
			this.arg2 = theta;
			return this;
		}
		private DifferentialDriveCommand build() {
			return new DifferentialDriveCommand(this);
		}
	}
	
	public enum CommandType {
		ESTOP(), 
		MOTOR(), 
		ANGVEL(), 
		LINVEL(), 
		VEL(), 
		HEADING(), 
		HEADING_LINVEL(), 
		MOVE_TO();
	}
	
	private final CommandType type;
	private final double arg0;	// left, angularVelocity, x
	private final double arg1;	// right, linearVelocity, y
	private final double arg2;	// theta
	
	private DifferentialDriveCommand() {
		type = CommandType.ESTOP;
		arg0 = 0;
		arg1 = 0;
		arg2 = 0;
	}
	
	private DifferentialDriveCommand(SplinterDriveCommandBuilder builder) {
		type = builder.type;
		arg0 = builder.arg0;
		arg1 = builder.arg1;
		arg2 = builder.arg2;
	}
	
	public CommandType getType() {
		return type;
	}
	
	public double getLeft() {
		if (type != CommandType.MOTOR) {
			throw new IllegalStateException();
		}
		return arg0;
	}
	
	public double getRight() {
		if (type != CommandType.MOTOR) {
			throw new IllegalStateException();
		}
		return arg1;
	}
	
	public double getAngularVelocity() {
		if (type != CommandType.ANGVEL && type != CommandType.LINVEL && type != CommandType.VEL) {
			throw new IllegalStateException();
		}
		return arg0;
	}
	
	public double getLinearVelocity() {
		if (type != CommandType.ANGVEL && type != CommandType.LINVEL && type != CommandType.VEL && type != CommandType.HEADING_LINVEL) {
			throw new IllegalStateException();
		}
		return arg1;
	}
	
	public double getHeading() {
		if (type != CommandType.HEADING && type != CommandType.HEADING_LINVEL) {
			throw new IllegalStateException();
		}
		return arg2;
	}
	
	double getX() {
		if (type != CommandType.MOVE_TO) {
			throw new IllegalStateException();
		}
		return arg0;
	}
	
	double getY() {
		if (type != CommandType.MOVE_TO) {
			throw new IllegalStateException();
		}
		return arg1;
	}
	
	double getTheta() {
		if (type != CommandType.MOVE_TO) {
			throw new IllegalStateException();
		}
		return arg2;
	}
	
	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append(type);
		sb.append("(");
		switch (type) {
		case ESTOP:
			break;
		case MOTOR:
			sb.append(getLeft());
			sb.append(",");
			sb.append(getRight());
			break;
		case VEL:
			sb.append(getAngularVelocity());
			sb.append(",");
			sb.append(getLinearVelocity());
			break;
		case ANGVEL:
			sb.append(getAngularVelocity());
			break;
		case LINVEL:
			sb.append(getLinearVelocity());
			break;
		case HEADING:
			sb.append(getHeading());
			break;
		case MOVE_TO:
			sb.append(getX());
			sb.append(",");
			sb.append(getY());
			sb.append(",");
			sb.append(getTheta());
			break;
		}
		sb.append(")");
		return sb.toString();
	}
}
