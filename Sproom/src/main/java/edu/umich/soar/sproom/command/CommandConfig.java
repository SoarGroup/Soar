package edu.umich.soar.sproom.command;

import java.util.Arrays;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.CopyOnWriteArrayList;

import edu.umich.soar.sproom.drive.Drive2;
import edu.umich.soar.sproom.drive.Drive3;

public enum CommandConfig {
	CONFIG;
	
	private String productions;
	
	// TODO defaults
	private double limitLinVelMax = 0.5;
	private double limitLinVelMin;
	private double limitAngVelMax = Math.PI;
	private double limitAngVelMin;
	
	private double geomLength;
	private double geomWidth;
	private double geomHeight;
	private double geomWheelbase;
	
	public enum LengthUnit { 
		METERS(1), FEET(0.3048), MILES(1609.344), YARDS(0.9144), INCH(0.0254);
		
		private final double meters;
		
		LengthUnit(double meters) {
			this.meters = meters;
		}
		
		double fromView(double view) {
			return view * meters;
		}
		
		double toView(double system) {
			return system / meters;
		}
	};
	private LengthUnit lengthUnits = LengthUnit.METERS;
	
	public enum SpeedUnit { 
		METERS_PER_SEC(1), FEET_PER_SEC(0.3048), MILES_PER_HOUR(0.44704);
		
		private final double metersPerSec;
		
		SpeedUnit(double metersPerSec) {
			this.metersPerSec = metersPerSec;
		}
		
		double fromView(double view) {
			return view * metersPerSec;
		}
		
		double toView(double system) {
			return system / metersPerSec;
		}
	};
	private SpeedUnit speedUnits = SpeedUnit.METERS_PER_SEC;
	
	public enum AngleUnit { RADIANS, DEGREES };
	private AngleUnit angleUnits = AngleUnit.DEGREES;

	public enum AngleResolution { INT, FLOAT };
	private AngleResolution angleResolution = AngleResolution.INT;

	private final double[] poseTranslation = new double[] { 0, 0, 0 };
	private final ConcurrentMap<String, double[]> pidGains = new ConcurrentHashMap<String, double[]>(); 
	
	private int rangeCount = 5; // LIDAR
	
	private double fieldOfView = Math.PI / 2.0;
	private long visibleNanoTime = 2 * 1000000000;
	private double manipulationDistance = 1.0;
	private double gamepadZeroThreshold = 0.4;

	private final List<CommandConfigListener> listeners = new CopyOnWriteArrayList<CommandConfigListener>();

	CommandConfig() {
		setGains(Drive3.HEADING_PID_NAME, new double[] { 1, 0, 0.125 });
		setGains(Drive2.ANGULAR_PID_NAME, new double[] { 0.0238, 0, 0.0025 });
		setGains(Drive2.LINEAR_PID_NAME, new double[] { 0.12,  0, 0.025 });
	}
	
	public double lengthFromView(double view) {
		return lengthUnits.fromView(view);
	}
	
	public double lengthToView(double system) {
		return lengthUnits.toView(system);
	}
	
	public double angleFromView(double view) {
		return angleUnits == AngleUnit.RADIANS ? view : Math.toRadians(view);
	}
	
	public double angleToView(double system) {
		return angleUnits == AngleUnit.RADIANS ? system : Math.toDegrees(system);
	}
	
	public double speedFromView(double view) {
		return speedUnits == SpeedUnit.METERS_PER_SEC ? view : Math.toRadians(view);
	}
	
	public double speedToView(double system) {
		return speedUnits == SpeedUnit.METERS_PER_SEC ? system : Math.toDegrees(system);
	}
	
	public void addListener(CommandConfigListener listener) {
		listeners.add(listener);
	}
	
	public boolean removeListener(CommandConfigListener listener) {
		return listeners.remove(listener);
	}
	
	private void fireConfigChanged() {
		for (CommandConfigListener listener : listeners) {
			listener.configChanged();
		}
	}
	
	public String getProductions() {
		return productions;
	}
	
	public void setProductions(String productions) {
		this.productions = productions;
		fireConfigChanged();
	}
	
	public double getLimitLinVelMax() {
		return limitLinVelMax;
	}

	public void setLimitLinVelMax(double limitLinVelMax) {
		this.limitLinVelMax = limitLinVelMax;
		fireConfigChanged();
	}

	public double getLimitLinVelMin() {
		return limitLinVelMin;
	}

	public void setLimitLinVelMin(double limitLinVelMin) {
		this.limitLinVelMin = limitLinVelMin;
		fireConfigChanged();
	}

	public double getLimitAngVelMax() {
		return limitAngVelMax;
	}

	public void setLimitAngVelMax(double limitAngVelMax) {
		this.limitAngVelMax = limitAngVelMax;
		fireConfigChanged();
	}

	public double getLimitAngVelMin() {
		return limitAngVelMin;
	}

	public void setLimitAngVelMin(double limitAngVelMin) {
		this.limitAngVelMin = limitAngVelMin;
		fireConfigChanged();
	}

	public double getGeomLength() {
		return geomLength;
	}

	public void setGeomLength(double geomLength) {
		this.geomLength = geomLength;
		fireConfigChanged();
	}

	public double getGeomWidth() {
		return geomWidth;
	}

	public void setGeomWidth(double geomWidth) {
		this.geomWidth = geomWidth;
		fireConfigChanged();
	}

	public double getGeomHeight() {
		return geomHeight;
	}

	public void setGeomHeight(double geomHeight) {
		this.geomHeight = geomHeight;
		fireConfigChanged();
	}

	public double getGeomWheelbase() {
		return geomWheelbase;
	}

	public void setGeomWheelbase(double geomWheelbase) {
		this.geomWheelbase = geomWheelbase;
		fireConfigChanged();
	}

	public LengthUnit getLengthUnits() {
		return lengthUnits;
	}

	public void setLengthUnits(LengthUnit lengthUnits) {
		this.lengthUnits = lengthUnits;
		fireConfigChanged();
	}

	public AngleUnit getAngleUnits() {
		return angleUnits;
	}

	public void setAngleUnits(AngleUnit angleUnits) {
		this.angleUnits = angleUnits;
		fireConfigChanged();
	}

	public SpeedUnit getSpeedUnits() {
		return speedUnits;
	}

	public void setSpeedUnits(SpeedUnit speedUnits) {
		this.speedUnits = speedUnits;
		fireConfigChanged();
	}

	public AngleResolution getAngleResolution() {
		return angleResolution;
	}

	public void setAngleResolution(AngleResolution angleResolution) {
		this.angleResolution = angleResolution;
		fireConfigChanged();
	}

	public void setPoseTranslation(double[] poseTranslation) {
		synchronized(this.poseTranslation) {
			System.arraycopy(poseTranslation, 0, this.poseTranslation, 0, this.poseTranslation.length);
		}
		fireConfigChanged();
	}

	public double[] getPoseTranslation() {
		synchronized(poseTranslation) {
			return Arrays.copyOf(poseTranslation, poseTranslation.length);
		}
	}
	
	public void setGains(String name, double[] pid) {
		if (pid == null) {
			pidGains.remove(name);
			fireConfigChanged();
			return;
		}
		pidGains.put(name, pid);
		fireConfigChanged();
	}
	
	public double[] getGains(String name) {
		return pidGains.get(name);
	}

	public int getRangeCount() {
		return rangeCount;
	}
	
	public double getFieldOfView() {
		return fieldOfView;
	}

	public long getVisibleNanoTime() {
		return visibleNanoTime;
	}

	public double getManipulationDistance() {
		return manipulationDistance;
	}

	public double getGamepadZeroThreshold() {
		return gamepadZeroThreshold;
	}

}
