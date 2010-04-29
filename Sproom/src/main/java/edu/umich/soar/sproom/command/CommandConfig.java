package edu.umich.soar.sproom.command;

import java.util.Arrays;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.CopyOnWriteArrayList;

import april.config.Config;

import edu.umich.soar.sproom.drive.Drive2;
import edu.umich.soar.sproom.drive.Drive3;

/**
 * Stores configuration parameters for the Command class.
 *
 * @author voigtjr@gmail.com
 */
public enum CommandConfig {
	CONFIG;
	
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

	public enum AngleUnit { RADIANS, DEGREES };

	public enum AngleResolution { INT, FLOAT };

	private String productions;
	private boolean gamepad = true;
	
	private double limitLinVelMax = 0.5;
	private double limitLinVelMin = -0.5;
	private double limitAngVelMax = Math.PI;
	private double limitAngVelMin = -Math.PI;
	
	private double geomLength = 0;		// TODO
	private double geomWidth = 0;		// TODO
	private double geomHeight = 0;		// TODO
	private double geomWheelbase = 0;	// TODO
	
	private LengthUnit lengthUnits = LengthUnit.METERS;
	private SpeedUnit speedUnits = SpeedUnit.METERS_PER_SEC;
	private AngleUnit angleUnits = AngleUnit.DEGREES;
	private AngleResolution angleResolution = AngleResolution.INT;

	private final double[] poseTranslation = new double[] { 0, 0, 0 };
	
	private final ConcurrentMap<String, double[]> pidGains = new ConcurrentHashMap<String, double[]>(); 
	private final double[] HEADING_GAINS_DEFAULT = new double[] { 1, 0, 0.125 };
	private final double[] ANGULAR_GAINS_DEFAULT = new double[] { 0.0238, 0, 0.0025 };
	private final double[] LINEAR_GAINS_DEFAULT = new double[] { 0.12,  0, 0.025 };

	private int rangeCount = 5; // LIDAR
	
	private double fieldOfView = Math.PI / 2.0;
	private final int VISIBLE_SECONDS_DEFAULT = 2;
	private long visibleNanoTime = VISIBLE_SECONDS_DEFAULT * 1000000000L;
	private double manipulationDistanceMin = 0.2;
	private double manipulationDistanceMax = 1.0;
	private double gamepadZeroThreshold = 0.4;
	private final double LIDAR_CACHE_SECONDS_DEFAULT = 1.0;
	private long lidarCacheTime = (long)(LIDAR_CACHE_SECONDS_DEFAULT * 1000000000L);
	private boolean spawnDebugger = false;
	private Integer randomSeed;

	private final List<CommandConfigListener> listeners = new CopyOnWriteArrayList<CommandConfigListener>();

	public void initialize(Config config)
	{
		config = config.getChild("command");
		
		productions = config.getString("productions", null);
		gamepad = config.getBoolean("gamepad", gamepad);
		limitLinVelMax = config.getDouble("limitLinVelMax", limitLinVelMax);
		limitLinVelMin = config.getDouble("limitLinVelMin", limitLinVelMin);
		limitAngVelMax = config.getDouble("limitAngVelMax", limitAngVelMax);
		limitAngVelMin = config.getDouble("limitAngVelMin", limitAngVelMin);
		
		geomLength = config.getDouble("geomLength", geomLength);
		geomWidth = config.getDouble("geomWidth", geomWidth);
		geomHeight = config.getDouble("geomHeight", geomHeight);
		geomWheelbase = config.getDouble("geomWheelbase", geomWheelbase);
		
		try {
			lengthUnits = LengthUnit.valueOf(config.getString("lengthUnits", lengthUnits.toString()));
		} catch (IllegalArgumentException e) {
		}
		try {
			speedUnits = SpeedUnit.valueOf(config.getString("speedUnits", speedUnits.toString()));
		} catch (IllegalArgumentException e) {
		}
		try {
			angleUnits = AngleUnit.valueOf(config.getString("angleUnits", angleUnits.toString()));
		} catch (IllegalArgumentException e) {
		}
		try {
			angleResolution = AngleResolution.valueOf(config.getString("angleResolution", angleResolution.toString()));
		} catch (IllegalArgumentException e) {
		}

		double[] pt = config.getDoubles("poseTranslation", poseTranslation);
		System.arraycopy(pt, 0, poseTranslation, 0, pt.length);

		setGains(Drive3.HEADING_PID_NAME, config.getDoubles(Drive3.HEADING_PID_NAME + "Gains", HEADING_GAINS_DEFAULT));
		setGains(Drive2.ANGULAR_PID_NAME, config.getDoubles(Drive2.ANGULAR_PID_NAME + "Gains", ANGULAR_GAINS_DEFAULT));
		setGains(Drive2.LINEAR_PID_NAME, config.getDoubles(Drive2.LINEAR_PID_NAME + "Gains", LINEAR_GAINS_DEFAULT));
		
		rangeCount = config.getInt("rangeCount", rangeCount);
		fieldOfView = config.getDouble("fieldOfView", fieldOfView);
		visibleNanoTime = config.getInt("visibleSeconds", VISIBLE_SECONDS_DEFAULT) * 1000000000L;
		manipulationDistanceMin = config.getDouble("manipulationDistanceMin", manipulationDistanceMin);
		manipulationDistanceMax = config.getDouble("manipulationDistanceMax", manipulationDistanceMax);
		gamepadZeroThreshold = config.getDouble("gamepadZeroThreshold", gamepadZeroThreshold);
		lidarCacheTime = (long)(config.getDouble("lidarCacheTimeSeconds", LIDAR_CACHE_SECONDS_DEFAULT) * 1000000000L);
		spawnDebugger = config.getBoolean("spawnDebugger", spawnDebugger);
		if (config.hasKey("randomSeed")) {
			randomSeed = Integer.valueOf(config.requireInt("randomSeed"));
		} else {
			randomSeed = null;
		}
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

	public double getManipulationDistanceMin() {
		return manipulationDistanceMin;
	}

	public double getManipulationDistanceMax() {
		return manipulationDistanceMax;
	}

	public double getGamepadZeroThreshold() {
		return gamepadZeroThreshold;
	}

	public long getLidarCacheTime()
	{
		return lidarCacheTime;
	}
	
	public boolean getSpawnDebugger() {
		return spawnDebugger;
	}

	public boolean hasRandomSeed() {
		return randomSeed != null;
	}
	
	public int getRandomSeed() {
		if (randomSeed == null) {
			throw new IllegalStateException("Random seed null");
		}
		return randomSeed.intValue();
	}
	
	public void setRandomSeed(int seed) {
		randomSeed = Integer.valueOf(seed);
	}
	
	public void clearRandomSeed() {
		randomSeed = null;
	}
	
	public boolean getGamepad() {
		return gamepad;
	}
	
}
