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
			"@"
		};
	
	public static final String[] SOAR_DOC = {
		"brief",
		"created",
		"desc",
		"devnote",
		"end-no-soardoc",
		"file",
		"group",
		"ingroup",
		"kernel",
		"mainpage",
		"modified",
		"operator",
		"production",
		"problem-space",
		"project",
		"ref",
		"start-no-soardoc",
		"todo",
		"type"
	};
}
