package edu.umich.soar.sproom.command;

import java.util.Arrays;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

enum CommandConfig {
	CONFIG;
	
	private String productions;
	
	private double limitLinVelMax;
	private double limitLinVelMin;
	private double limitAngVelMax;
	private double limitAngVelMin;
	
	private double geomLength;
	private double geomWidth;
	private double geomHeight;
	private double geomWheelbase;
	
	enum LengthUnit { METERS, FEET, MILES };
	private LengthUnit lengthUnits;
	
	enum AngleUnit { RADIANS, DEGREES };
	private AngleUnit angleUnits;

	enum AngleResolution { INT, FLOAT };
	private AngleUnit angleResolution;

	private final double[] poseTranslation = new double[] { 0, 0, 0 };
	private final ConcurrentMap<String, double[]> pidGains = new ConcurrentHashMap<String, double[]>(); 

	CommandConfig() {
		setGains(Drive3.HEADING_PID_NAME, new double[] { 1, 0, 0.125 });
		setGains(Drive2.ANGULAR_PID_NAME, new double[] { 0.0238, 0, 0.0025 });
		setGains(Drive2.LINEAR_PID_NAME, new double[] { 0.12,  0, 0.025 });
	}
	public String getProductions() {
		return productions;
	}
	
	public void setProductions(String productions) {
		this.productions = productions;
	}
	
	public double getLimitLinVelMax() {
		return limitLinVelMax;
	}

	public void setLimitLinVelMax(double limitLinVelMax) {
		this.limitLinVelMax = limitLinVelMax;
	}

	public double getLimitLinVelMin() {
		return limitLinVelMin;
	}

	public void setLimitLinVelMin(double limitLinVelMin) {
		this.limitLinVelMin = limitLinVelMin;
	}

	public double getLimitAngVelMax() {
		return limitAngVelMax;
	}

	public void setLimitAngVelMax(double limitAngVelMax) {
		this.limitAngVelMax = limitAngVelMax;
	}

	public double getLimitAngVelMin() {
		return limitAngVelMin;
	}

	public void setLimitAngVelMin(double limitAngVelMin) {
		this.limitAngVelMin = limitAngVelMin;
	}

	public double getGeomLength() {
		return geomLength;
	}

	public void setGeomLength(double geomLength) {
		this.geomLength = geomLength;
	}

	public double getGeomWidth() {
		return geomWidth;
	}

	public void setGeomWidth(double geomWidth) {
		this.geomWidth = geomWidth;
	}

	public double getGeomHeight() {
		return geomHeight;
	}

	public void setGeomHeight(double geomHeight) {
		this.geomHeight = geomHeight;
	}

	public double getGeomWheelbase() {
		return geomWheelbase;
	}

	public void setGeomWheelbase(double geomWheelbase) {
		this.geomWheelbase = geomWheelbase;
	}

	public LengthUnit getLengthUnits() {
		return lengthUnits;
	}

	public void setLengthUnits(LengthUnit lengthUnits) {
		this.lengthUnits = lengthUnits;
	}

	public AngleUnit getAngleUnits() {
		return angleUnits;
	}

	public void setAngleUnits(AngleUnit angleUnits) {
		this.angleUnits = angleUnits;
	}

	public AngleUnit getAngleResolution() {
		return angleResolution;
	}

	public void setAngleResolution(AngleUnit angleResolution) {
		this.angleResolution = angleResolution;
	}

	public void setPoseTranslation(double[] poseTranslation) {
		synchronized(this.poseTranslation) {
			System.arraycopy(poseTranslation, 0, this.poseTranslation, 0, this.poseTranslation.length);
		}
	}

	public double[] getPoseTranslation() {
		synchronized(poseTranslation) {
			return Arrays.copyOf(poseTranslation, poseTranslation.length);
		}
	}
	
	public void setGains(String name, double[] pid) {
		if (pid == null) {
			pidGains.remove(name);
			return;
		}
		pidGains.put(name, pid);
	}
	
	public double[] getGains(String name) {
		return pidGains.get(name);
	}
}
