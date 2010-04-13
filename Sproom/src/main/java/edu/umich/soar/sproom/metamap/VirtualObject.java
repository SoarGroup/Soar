package edu.umich.soar.sproom.metamap;

import java.util.Arrays;

import jmat.LinAlg;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.command.CommandConfig;
import edu.umich.soar.sproom.command.Pose;

/**
 * A virtual object managed by MapMetadata.
 *
 * @author voigtjr@gmail.com
 */
public class VirtualObject {
	private static final Log logger = LogFactory.getLog(VirtualObject.class);
	
	public enum Type {
		BLOCK, BRICK, BALL, PLAYER, IED
	}

	public enum Color {
		RED, GREEN, BLUE
	}

	private static int count = 0;
	private final int id;
	private final Type type;
	private final Color color;
	private final double[] pos;
	private final double[] size;
	private final double theta;
	private boolean diffused = false;
	
	public VirtualObject(Type type, Color color, double[] pos, double[] size, double theta) {
		this.id = count++;
		this.type = type;
		this.color = color;
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
		if (color != null) {
			sb.append(color);
			sb.append("/");
		}
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

	public Color getColor() {
		return color;
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

	public boolean isInRange(Pose pose)
	{
		double distance = LinAlg.distance(getPos(), pose.getPose().pos);
		CommandConfig c = CommandConfig.CONFIG;
		double manipDist = c.getManipulationDistanceMax();
		manipDist += getSize()[0] / 2.0;
		if (distance > c.getManipulationDistanceMax()) {
			return false;
		}
		return true;
	}

	public void diffuse()
	{
		diffused = true;
	}
	
	public boolean isDiffused() {
		return diffused;
	}
}
