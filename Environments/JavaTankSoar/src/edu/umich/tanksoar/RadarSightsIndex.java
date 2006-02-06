package edu.umich.tanksoar;

/**
 * @author stokesd
 *
 * The radar information that's presented to the input link is an array.
 * This class represents a single spot in that array.  Position and distance
 * can be though of as the location of any object that's in this radar square,
 * but relative to the tank's position.
 */
public class RadarSightsIndex
{
  RadarSightsIndex(int p, int d){ position = p; distance = d; }
  public int position;//position on radar.  Min val of 0 (left), max val of 2 (right)
  public int distance;//how long the radar sight box is
}