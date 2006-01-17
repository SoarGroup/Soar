/*
 * "The Java Developer's Guide to Eclipse"
 *   by Shavor, D'Anjou, Fairbrother, Kehn, Kellerman, McCarthy
 * 
 * (C) Copyright International Business Machines Corporation, 2003. 
 * All Rights Reserved.
 * 
 * Code or samples provided herein are provided without warranty of any kind.
 */
package edu.rosehulman.soar.editor.util;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.RGB;
import org.eclipse.swt.widgets.Display;

/**
 * Colors used in the Soar editor
 */
public class ColorProvider
{

	public static final RGB BACKGROUND = new RGB(255, 255, 255);

	public static final RGB COMMENT = new RGB(0, 127, 85);
	public static final RGB SOAR_DOC = new RGB(0, 127, 150);
	
	public static final RGB ATTRIBUTE = new RGB(255, 128, 0);
	public static final RGB VARIABLE = new RGB(0, 128, 255);
	public static final RGB DEFAULT = new RGB(0, 0, 0);
	public static final RGB NAME = new RGB(255, 128, 0);
	public static final RGB NUMBER = new RGB(200, 0, 100);
	public static final RGB KEYWORD = new RGB(255, 0, 0);
	public static final RGB SPECIAL_CHARACTER = new RGB(255, 0, 0);
	public static final RGB OPERATOR = new RGB(0, 127, 255);
	public static final RGB IDENTIFIER = new RGB(0, 64, 200);
	public static final RGB STRING = new RGB(0, 0, 255);

	protected Map fColorTable = new HashMap(10);

	/**
	 * Method disposes of the colors.
	 */
	public void dispose()
	{
		Iterator e = fColorTable.values().iterator();
		while(e.hasNext())
			((Color) e.next()).dispose();
	}
	
	/**
	 * A getter method that returns a color.
	 * @param rgb
	 * @return Color
	 */
	public Color getColor(RGB rgb)
	{
		Color color = (Color) fColorTable.get(rgb);
		if(color == null)
		{
			color = new Color(Display.getCurrent(), rgb);
			fColorTable.put(rgb, color);
		}
		return color;
	}
}
