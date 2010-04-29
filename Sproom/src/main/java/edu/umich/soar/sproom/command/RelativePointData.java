/**
 * 
 */
package edu.umich.soar.sproom.command;

public class RelativePointData
{
	public final double distance;
	public final double yaw;
	public final double relativeYaw;

	public RelativePointData(double distance, double yaw, double relativeYaw)
	{
		this.distance = distance;
		this.yaw = yaw;
		this.relativeYaw = relativeYaw;
	}
}