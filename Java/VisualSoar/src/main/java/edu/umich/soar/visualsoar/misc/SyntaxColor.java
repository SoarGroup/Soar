package edu.umich.soar.visualsoar.misc;

import java.awt.Color;

import edu.umich.soar.visualsoar.parser.SoarParserConstants;

public class SyntaxColor extends Color
{
	public static SyntaxColor[] getDefaultSyntaxColors() {
		SyntaxColor[] temp;

		int size = SoarParserConstants.RARROW;

		size = Math.max(size, SoarParserConstants.SP);
		size = Math.max(size, SoarParserConstants.GP);
		size = Math.max(size, SoarParserConstants.CARET);
		size = Math.max(size, SoarParserConstants.VARIABLE);
		size = Math.max(size, SoarParserConstants.SYMBOLIC_CONST);
		size = Math.max(size, SoarParserConstants.DEFAULT);

		temp = new SyntaxColor[size + 1];

		temp[SoarParserConstants.RARROW] = new SyntaxColor(Color.red, "\"-->\"");
		temp[SoarParserConstants.SP] = new SyntaxColor(Color.red, "\"sp\"");
		temp[SoarParserConstants.GP] = new SyntaxColor(Color.decode("-65332"), "\"gp\"");
		temp[SoarParserConstants.CARET] = new SyntaxColor(Color.orange.darker(), "Literal Attributes");
		temp[SoarParserConstants.VARIABLE] = new SyntaxColor(Color.green.darker(), "Variables");
		temp[SoarParserConstants.SYMBOLIC_CONST] = new SyntaxColor(Color.blue, "Symbolic Constants");
		temp[SoarParserConstants.DEFAULT] = new SyntaxColor(Color.black); // default

		return temp;
	}

    String      name = null;

    public SyntaxColor(Color c)
    {
        super(c.getRGB());
    }
    
    public SyntaxColor(int rgb, String str)
    {
        super(rgb);
        
        name = str;
    }
    
    public SyntaxColor(Color c, String str)
    {
        this(c.getRGB(), str);
    }
    
    public SyntaxColor(int rgb, SyntaxColor old)
    {
        this(rgb, old.getName());
    }
    
    public SyntaxColor(Color c, SyntaxColor old)
    {
        this(c.getRGB(), old.getName());
    }
    
    /**
     * Returns the string identifying the SyntaxColor
     * @return the name String, null if there is none
     */
    public String getName()
    {
        return name;
    }
    
    public boolean equals(String s)
    {
        return (name != null ? name.equals(s) : false);
    }
}
