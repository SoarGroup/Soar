package edu.umich.visualsoar.util;


import java.util.*;
import java.awt.*;
import javax.swing.*;
import javax.swing.plaf.metal.*;


/**
 * Author: Nobuo Tamemasa
 * http://www.codeguru.com/java/articles/187.shtml
 * @version 1.0 01/12/99
 */
public class TextIcons extends MetalIconFactory.TreeLeafIcon {

  protected String label;
  private static Hashtable labels;

  protected TextIcons() {
  }

  public void paintIcon(Component c, Graphics g, int x, int y) {
    super.paintIcon( c, g, x, y );
    if (label != null) {
      FontMetrics fm = g.getFontMetrics();
          
      int offsetX = (getIconWidth()  - fm.stringWidth(label)) /2;
      int offsetY = (getIconHeight() - fm.getHeight()) /2 - 2;
      
      g.drawString(label, x + offsetX ,
                          y + offsetY + fm.getHeight());
    }
  }
  
  public static Icon getIcon(String str) {

    if (labels == null) {
      labels = new Hashtable();
      setDefaultSet();
    }
    TextIcons icon = new TextIcons();
    icon.label = (String)labels.get(str);
    return icon;
  }
  
  public static void setLabelSet(String ext, String label) {
    if (labels == null) {
      labels = new Hashtable();
      setDefaultSet();
    }
    labels.put(ext, label);   
  }
  
  private static void setDefaultSet() {
    labels.put("impasse"     ,"I");
    labels.put("file"  ,"F");
    labels.put("operator"  ,"O");
    labels.put("link", "L");
    
    // and so on
    /*
    labels.put("txt"   ,"TXT");   
    labels.put("TXT"   ,"TXT");   
    labels.put("cc"    ,"C++");   
    labels.put("C"     ,"C++");   
    labels.put("cpp"   ,"C++");   
    labels.put("exe"   ,"BIN");   
    labels.put("class" ,"BIN");   
    labels.put("gif"   ,"GIF");   
    labels.put("GIF"   ,"GIF");  
    
    labels.put("", "");   
    */
  }

}
