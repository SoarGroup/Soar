/**
 * Detects words in the Soar Editor Plug-in
 * @file SoarWordDetector.java
 * @date December 11, 2003
 */
package edu.rosehulman.soar.editor.util;

import org.eclipse.jface.text.rules.IWordDetector;

import edu.rosehulman.soar.editor.soar.ISoarSyntax;

/**
 * Determines whether a given character is valid as part of a Soar keyword
 * in the current context.
 */
public class SoarWordDetector implements IWordDetector, ISoarSyntax
{
	public static boolean inWord = false;
	
	/**
	 * @see org.eclipse.jface.text.rules.IWordDetector#isWordStart(char)
	 */
	public boolean isWordStart(char c)
	{
		for(int i = 0; i < RESERVED_WORDS.length; i++)
		{
			if( (((String) RESERVED_WORDS[i]).charAt(0) == c) ||
				 (((String) SPECIAL_CHARACTERS[i]).charAt(0) == c))
			{
				return true;
			}
		}

		return false;
	}

	/**
	 * @see org.eclipse.jface.text.rules.IWordDetector#isWordPart(char)
	 */
	public boolean isWordPart(char c)
	{
		for(int i = 0; i < RESERVED_WORDS.length; i++)
		{
			if( (((String) RESERVED_WORDS[i]).indexOf(c) != -1) ||
				 (((String)SPECIAL_CHARACTERS[i]).indexOf(c) != -1) )
			{
				return true;
			}
		}

		return false;
	}
}
