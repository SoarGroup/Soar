package edu.umich.tanksoar;

/**
 * @author stokesd
 *
 * TankSoarRadarElement
 */
abstract class TankSoarRadarElement
{
  public RadarSightsIndex getRelativePosition()
  {
    return relativePosition;        
  }

  public String getColor(){ return color; }

  abstract public void cleanUp();

  protected RadarSightsIndex relativePosition = null;

  public String color = null;//not all children will have color, so this is "fat interface" style
}