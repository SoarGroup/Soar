package edu.umich.soar.sproom.command;

import java.util.Arrays;

enum CommandConfig {
	CONFIG;
	
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
}
