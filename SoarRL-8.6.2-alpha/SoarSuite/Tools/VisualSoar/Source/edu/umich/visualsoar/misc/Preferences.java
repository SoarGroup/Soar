package edu.umich.visualsoar.misc;

import java.io.*;
import java.awt.*;
import java.util.*;
import javax.swing.*;
import edu.umich.visualsoar.parser.*;

public class Preferences
{

    static Preferences      self = null;
    private File prefFile;
    private File templateFolder = null;
    private File openFolder = null;    
    private boolean autoTileEnabled = true;
    private boolean horizTile = true;
    private boolean highlightingEnabled = true;
    private boolean autoIndentingEnabled = true;
    private boolean autoSoarCompleteEnabled = true;
    private SyntaxColor[]   syntaxColors = null;
           
    // Shared Project variables
    private File sharedProjectFile = null;
    private boolean sharedProjectEnabled = false;
    private String userName = "User";

    public static String getVisualSoarFolder()
    {
    	// SoarLibrary/bin/../../Tools/VisualSoar
    	File folder = new File(System.getProperty("user.dir") + File.separator + ".." + File.separator + ".." + File.separator + "Tools" + File.separator + "VisualSoar") ;
    	return folder.toString() ;
    }
    
    private Preferences()
    {


        setDefaultPreferences();


        StreamTokenizer     stok;
        
        prefFile = new File(getVisualSoarFolder() 
                            + File.separator + "VSPreferences.txt");
        
        if (! prefFile.exists())
        {
            
            try
            {
                prefFile.createNewFile();
            } catch (IOException ioe)
            {
                ioe.printStackTrace();
                setDefaultPreferences();
                return;
            }
        }

        try
        {
            stok = new StreamTokenizer(new FileReader(prefFile));
        } catch (FileNotFoundException fnfe)
        {
            fnfe.printStackTrace();
            setDefaultPreferences();
            return;
        }
                
        if ((prefFile == null) || (prefFile.length() == 0))
        {

            setDefaultPreferences();
            return;
        }
        else { // read in prefs from file
                                        
            stok.slashSlashComments(true);
            stok.eolIsSignificant(true);
            stok.wordChars(':', ':');
            stok.wordChars('\\', '\\');
            stok.wordChars('/', '/');
            stok.wordChars('.', '.');
            stok.wordChars(' ', ' ');

            try
            {

                syntaxColors = getDefaultSyntaxColors();

                while (stok.nextToken() != stok.TT_EOF)
                {
                    if (stok.ttype == stok.TT_WORD)
                    {
                        if (stok.sval.equals("templateFolder"))
                        {
                            if (stok.nextToken() == stok.TT_WORD)
                            {

                                File temp = new File(stok.sval);
                                if (temp.exists())
                                {
                                    templateFolder = temp;
                                }
                            }
                        }
                        else if (stok.sval.equals("openFolder"))
                        {
                            if(stok.nextToken() == stok.TT_WORD)
                            {
                                File tempOpenFolder = new File(stok.sval);
                                if(tempOpenFolder.exists())
                                {
                                    openFolder = tempOpenFolder;
                                }
                            }
                        }
                        else if (stok.sval.equals("autoTileEnabled"))
                        {
                            if (stok.nextToken() == stok.TT_WORD)
                            {
                                if (stok.sval.equals("false"))
                                {
                                    autoTileEnabled = false;
                                }
                                // else, true by default
                            }
                        }
                        else if (stok.sval.equals("horizTile"))
                        {
                            if (stok.nextToken() == stok.TT_WORD)
                            {
                                if (stok.sval.equals("false"))
                                {
                                    horizTile = false;
                                }
                                // else, true by default
                            }
                        }
                        else if (stok.sval.equals("highlightingEnabled"))
                        {
                            if (stok.nextToken() == stok.TT_WORD)
                            {
                                if (stok.sval.equals("false"))
                                {
                                    highlightingEnabled = false;
                                }
                                // else, true by default
                            }
                        }
                        else if (stok.sval.equals("autoIndentingEnabled"))
                        {
                            if(stok.nextToken() == stok.TT_WORD)
                            {
                                if(stok.sval.equals("false"))
                                {
                                    autoIndentingEnabled = false;
                                }
                                // else, true by default
                            }
                        }
                        else if (stok.sval.equals("autoSoarCompleteEnabled"))
                        {
                            if(stok.nextToken() == stok.TT_WORD)
                            {
                                if(stok.sval.equals("false"))
                                {
                                    autoSoarCompleteEnabled = false;
                                }
                            }
                            // else, true by default
                        }
                        else if (stok.sval.equals("syntaxColor"))
                        {
                            if (stok.nextToken() == stok.TT_NUMBER)
                            {
                                int     type;
                                int     rgbVal;

                                type = (int)stok.nval;

                                if (stok.nextToken() == stok.TT_NUMBER)
                                {
                                    rgbVal = (int)stok.nval;
                                }
                                else
                                {
                                    break;
                                }

                                syntaxColors[type] =
                                    new SyntaxColor(rgbVal, syntaxColors[type]);
                            }
                        } // syntaxColor
                    } // if TT_WORD

                    while (stok.nextToken() != stok.TT_EOL); // go to next line
                } // iterate through file
                
                if (templateFolder == null)
                {
                    templateFolder = getDefaultTemplateFolder();
                }
                
                if (openFolder == null)
                {
                    openFolder = getDefaultOpenDirectory();
                }

                
            }
            catch (IOException ioe)
            {
                //ioe.printStackTrace();
                setDefaultPreferences();
                return;
            }
            //One thing that can cause the following exceptions is if the token
            //number constants have changed in the parser.  This usually occurs
            //when you change the soarparser.jj file. -:AMN: 07 Nov 03
            catch (NullPointerException npe)
            {
                setDefaultPreferences();
                return;
            }
            catch (ArrayIndexOutOfBoundsException abe)
            {
                setDefaultPreferences();
                return;
            }
            
                        
        } // read in prefs from file
        
    } // constructor


    public SyntaxColor[] getSyntaxColors()
    {
        return (SyntaxColor[])syntaxColors.clone();
    }
    
    public void setSyntaxColors(SyntaxColor[] inSyntaxColors)
    {
        System.arraycopy(inSyntaxColors,0,syntaxColors,0,inSyntaxColors.length);
    }
    
    public boolean isHighlightingEnabled()
    {
        return highlightingEnabled;
    }
    
    public void setHighlightingEnabled(boolean isEnabled)
    {
        highlightingEnabled = isEnabled;
    }
    
    public boolean isHorizontalTilingEnabled()
    {
        return horizTile;
    }
    
    public void setHorizontalTilingEnabled(boolean isEnabled)
    {
        horizTile = isEnabled;
    }
    
    public boolean isAutoTilingEnabled()
    {
        return autoTileEnabled;
    }
    
    public void setAutoTilingEnabled(boolean isEnabled)
    {
        autoTileEnabled = isEnabled;
    }
    
    public boolean isAutoIndentingEnabled()
    {
        return autoIndentingEnabled;
    }

    public boolean isAutoSoarCompleteEnabled()
    {
        return autoSoarCompleteEnabled;
    }
    
    public void setAutoIndentingEnabled(boolean isEnabled)
    {
        autoIndentingEnabled = isEnabled;
    }

    public void setAutoSoarCompleteEnabled(boolean isEnabled)
    {
        autoSoarCompleteEnabled = isEnabled;
    }
    
    public File getTemplateFolder()
    {
        return templateFolder;
    }
    
    public void setTemplateFolder(File inTemplateFolder)
    {
        templateFolder = inTemplateFolder;
    }
    
    public File getOpenFolder()
    {
        return openFolder;
    }
    
    public void setOpenFolder(File inOpenFolder)
    {
        openFolder = inOpenFolder;
    }

    public void setSharedProjectFile(File projectFile)
    {
        sharedProjectFile = projectFile;
    }
  
    public File getSharedProjectFile()
    {
        return sharedProjectFile;
    }
  
    public void setSharedProjectEnabled(boolean isEnabled)
    {
        sharedProjectEnabled = isEnabled;
    }

    public boolean isSharedProjectEnabled()
    {
        return sharedProjectEnabled;
    }

    public String getUserName()
    {
        return userName;
    }

    public void setUserName(String name)
    {
        userName = name;
    }
    
    /**
     * Writes back the preferences to the file
     */
    public void write()
    {
        FileWriter  writer;
        String      endl = System.getProperty("line.separator");
        
        if (self == null) { // no prefs recorded
            return;
        }
        
        try
        {
            writer = new FileWriter(prefFile);
            
            if (templateFolder != null)
            {
                writer.write("templateFolder\t" + templateFolder.toString() + endl);
            }
            writer.write("horizTile\t" + horizTile + endl);
            writer.write("autoTileEnabled\t" + autoTileEnabled + endl);
            writer.write("autoIndentingEnabled\t" + autoIndentingEnabled + endl);
            writer.write("autoSoarCompleteEnabled\t" + autoSoarCompleteEnabled + endl);
            writer.write("highlightingEnabled\t" + highlightingEnabled + endl);
            writer.write("openFolder\t"+ openFolder.toString() + endl);
            
            for (int i = 0; i < syntaxColors.length; i++)
            {
                if (i == SoarParserConstants.DEFAULT)
                {
                    continue;
                }
            
                if (syntaxColors[i] != null)
                {
                    writer.write("syntaxColor\t" + i + "\t" + syntaxColors[i].getRGB()
                                 + "\t//\t" + syntaxColors[i].getName() + endl);
                }               
            }
            writer.close();
            
        } catch (IOException ioe)
        {
            ioe.printStackTrace();
        }
        
    } 
    
    static public Preferences getInstance()
    {
    
        if (self == null)
        {
            self = new Preferences();
        }

        return self;
    }
    
    /**
     * In case of IO failure during construction, a way to fall back on a
     * complete set of functional preferences
     */
    void setDefaultPreferences()
    {
        
        templateFolder = getDefaultTemplateFolder();
        autoTileEnabled = true;
        horizTile = true;
        highlightingEnabled = true;
        autoIndentingEnabled = true;
        autoSoarCompleteEnabled = true;
        syntaxColors = getDefaultSyntaxColors();
        openFolder = getDefaultOpenDirectory();
    }
    
    File getDefaultTemplateFolder()
    {
        File temp = new File(getVisualSoarFolder() 
                             + File.separator + "templates");
                              
        return temp;    
    }
    
    File getDefaultOpenDirectory()
    {
        File temp = new File(getVisualSoarFolder());
        return temp;
    }
    
    SyntaxColor[] getDefaultSyntaxColors()
    {
        SyntaxColor[]   temp;
        
        int             size = SoarParserConstants.RARROW;
        
        size = Math.max(size, SoarParserConstants.SP);
        size = Math.max(size, SoarParserConstants.CARET);
        size = Math.max(size, SoarParserConstants.VARIABLE);
        size = Math.max(size, SoarParserConstants.SYMBOLIC_CONST);
        size = Math.max(size, SoarParserConstants.DEFAULT);

        temp = new SyntaxColor[size + 1];
        
        temp[SoarParserConstants.RARROW] = 
            new SyntaxColor(Color.red, "\"-->\"");
        temp[SoarParserConstants.SP] = 
            new SyntaxColor(Color.red, "\"sp\"");
        temp[SoarParserConstants.CARET] = 
            new SyntaxColor(Color.orange.darker(), "Literal Attributes");
        temp[SoarParserConstants.VARIABLE] = 
            new SyntaxColor(Color.green.darker(), "Variables");
        temp[SoarParserConstants.SYMBOLIC_CONST] = 
            new SyntaxColor(Color.blue, "Symbolic Constants");
        temp[SoarParserConstants.DEFAULT] =
            new SyntaxColor(Color.black); // default
            
        return temp;
    }
}
