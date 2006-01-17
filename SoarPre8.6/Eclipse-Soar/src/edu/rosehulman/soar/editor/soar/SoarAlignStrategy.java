/**
 * Defines the auto-alignment strategy for the Soar editor.
 * @file SoarAlignStrategy.java
 * @date February 16, 2004
 */
package edu.rosehulman.soar.editor.soar;

import org.eclipse.jface.text.formatter.IFormattingStrategy;

public class SoarAlignStrategy implements IFormattingStrategy
{
	/**
	 * @see org.eclipse.jface.text.formatter.IFormattingStrategy#formatterStarts(String)
	 */
	public void formatterStarts(String initialIndentation)
	{
		System.out.println("intialIndentation:" + initialIndentation);
	}

	/**
	 * @see org.eclipse.jface.text.formatter.IFormattingStrategy#format(String,boolean,String,int[])
	 */
	public String format(
			String content, 
			boolean isLineStart, 
			String indentation,
			int[] positions)
	{
		System.out.println("content:" + content);
		System.out.println("isLineStart:" + isLineStart);
		System.out.println("indentation: '" + indentation + "'");
		System.out.println("positions:" + content);
		for (int i=0; i<positions.length; i++) {
			System.out.println( i + ":" + positions[i]);
		}

		return content;
	}

	/**
	 * @see org.eclipse.jface.text.formatter.IFormattingStrategy#formatterStops()
	 */
	public void formatterStops()
	{
		System.out.println("formatterStops()");
	}
}
