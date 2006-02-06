/********************************************************************************************
*
* LabelPanel.java
* 
* Created on 	Nov 22, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package general;

import javax.swing.*;
import java.awt.*;

/********************************************************************************************
* 
* Given a label and a component, creates a new panel to contain both with the label (by default)
* above the panel being labeled.
* 
********************************************************************************************/
public class LabelPanel extends JPanel
{
	public LabelPanel(String label, Component mainPanel)
	{
		this(label, mainPanel, BorderLayout.NORTH) ;
	}
	
	public LabelPanel(String label, Component mainPanel, String borderLayout)
	{
		JLabel labelPanel = new JLabel(label) ;
		
		this.setLayout(new BorderLayout()) ;
		this.add(labelPanel, borderLayout) ;
		this.add(mainPanel, BorderLayout.CENTER) ;
	}
}
