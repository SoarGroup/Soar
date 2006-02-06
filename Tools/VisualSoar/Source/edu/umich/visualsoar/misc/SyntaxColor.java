package edu.umich.visualsoar.misc;

import java.awt.Color;

public class SyntaxColor extends Color
{
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
