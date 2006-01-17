/**
 * Detects whitespace in Soar files.
 * @file SoarWhiteSpaceDetector.java
 * @date December 11, 2003
 */
package edu.rosehulman.soar.editor.util;

import org.eclipse.jface.text.rules.IWhitespaceDetector;

public class SoarWhitespaceDetector implements IWhitespaceDetector
{
	/**
	 * Whitespace test method.
	 * @see org.eclipse.jface.text.rules.IWhiteSpaceDetector#isWhiteSpace(char)
	 */
	public boolean isWhitespace(char c)
	{
		return Character.isWhitespace(c);
	}
}
