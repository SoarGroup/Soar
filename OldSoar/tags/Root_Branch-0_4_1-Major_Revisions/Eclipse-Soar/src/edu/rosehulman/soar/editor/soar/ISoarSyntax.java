/**
 * Defines the syntax rules for the Soar language.
 * @file ISoarSyntax.java
 * @date December 11, 2003
 */
package edu.rosehulman.soar.editor.soar;

public interface ISoarSyntax
{
	public static final String[] RESERVED_WORDS =
		{
			"state",
			"sp",
			"-->",
		};

	public static final String[] SPECIAL_CHARACTERS =
		{
			"{",
			"}",
			"+",
			"-",
			"<",
			">",
			"=",
		};
}
