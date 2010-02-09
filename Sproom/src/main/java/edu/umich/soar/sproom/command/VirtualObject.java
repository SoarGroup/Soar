package edu.umich.soar.sproom.command;

import java.util.Arrays;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class VirtualObject {
	private static final Log logger = LogFactory.getLog(VirtualObject.class);
	
	public enum Type {
		BLOCK, BRICK, BALL, PLAYER
	}

	private static int count = 0;
	private final int id;
	private final Type type;
	private final double[] pos;
	private final double[] size;
	private final double theta;
	
	public VirtualObject(Type type, double[] pos, double[] size, double theta) {
		this.id = count++;
		this.type = type;
		this.pos = new double[3];
		setPos(pos);
		this.size = new double[3];
		System.arraycopy(size, 0, this.size, 0, size.length);
		this.theta = theta;
	}
	
	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder("VO");
		sb.append(id);
		sb.append("/");
		sb.append(type);
		sb.append("/");
		sb.append(Arrays.toString(pos));
		return sb.toString();
	}
	
	public double[] getPos() {
		double[] temp = new double[pos.length];
		System.arraycopy(pos, 0, temp, 0, pos.length);
		return temp;
	}

	public double[] getSize() {
		double[] temp = new double[size.length];
		System.arraycopy(size, 0, temp, 0, size.length);
		return temp;
	}
	
	public double getTheta() {
		return theta;
	}

	public Type getType() {
		return type;
	}

	public int getId() {
		return id;
	}

	public void setPos(double[] pos) {
		System.arraycopy(pos, 0, this.pos, 0, pos.length);
		if (logger.isTraceEnabled()) {
			logger.trace("Updated position: " + toString());
		}
	}
}
