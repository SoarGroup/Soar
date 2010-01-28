package edu.umich.soar.room;

import java.net.URL;

import javax.swing.ImageIcon;

/**
 * @author ray
 */
public class Images
{
	public static final ImageIcon TANKSOAR_DOWN = loadImage("/edu/umich/soar/gridmap2d/images/tanksoar/tank_down.gif");
	public static final ImageIcon TANKSOAR_UP = loadImage("/edu/umich/soar/gridmap2d/images/tanksoar/tank_up.gif");
	public static final ImageIcon TANKSOAR_RIGHT = loadImage("/edu/umich/soar/gridmap2d/images/tanksoar/tank_right.gif");
	public static final ImageIcon TANKSOAR_LEFT = loadImage("/edu/umich/soar/gridmap2d/images/tanksoar/tank_left.gif");
	public static final ImageIcon TANKSOAR_QUESTION = loadImage("/edu/umich/soar/gridmap2d/images/tanksoar/question.gif");
	public static final ImageIcon TANKSOAR_MINITANK = loadImage("/edu/umich/soar/gridmap2d/images/tanksoar/tank-mini.gif");
	public static final ImageIcon TANKSOAR_BLOCKED = loadImage("/edu/umich/soar/gridmap2d/images/tanksoar/blocked-diagram.gif");

    /**
     * @param string
     * @return
     */
    private static ImageIcon loadImage(String file)
    {
        URL url = Images.class.getResource(file);
        return new ImageIcon(url);
    }
}
