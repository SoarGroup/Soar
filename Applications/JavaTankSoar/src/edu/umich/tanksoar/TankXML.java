/********************************************************************************************
*
* TankXML.java
* 
* Description:	
* 
* Created on 	Dec 4, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package edu.umich.tanksoar;

import sml.Agent;
import sml.ElementXML;

/************************************************************************
 * 
 * Class for converting tank information to and from XML so it can
 * be shared directly with another process (currently Redux).
 * 
 ************************************************************************/
public class TankXML
{
	public static final boolean kBroadcastState = true ;
	
	/********************************************************************************************
	 * 
	 * Takes the input link description of a tank and converts it into XML.
	 * This may not be sufficient to describe the tank but it's a baseline.
	 * 
	 * @param info
	 * @return
	********************************************************************************************/
	private static ElementXML ConvertToXML(TankInputInfo info)
	{
		ElementXML xml = new ElementXML() ;
		xml.SetTagName("tank-input") ;
		xml.AddAttribute("X", Integer.toString(info.X)) ;
		xml.AddAttribute("Y", Integer.toString(info.Y)) ;
		
		return xml ;
	}
	
	/********************************************************************************************
	 * 
	 * Send the XML description of a tank over to Redux.
	 * 
	 * @param info
	 * @return
	********************************************************************************************/
	private static String SendXMLToRedux(Agent agent, ElementXML xml)
	{
		String xmlString = xml.GenerateXMLString(true) ;
		String result = agent.GetKernel().SendClientMessage(agent, "redux", xmlString) ;
		
		return result ;
	}
	
	/********************************************************************************************
	 * 
	 * Package up the description of the input link as XML and send it to Redux.
	 * 
	 * @param info
	 * @return
	********************************************************************************************/
	public static String SendInputToRedux(Agent agent, TankInputInfo info)
	{
		if (!kBroadcastState)
			return null ;
		
		ElementXML xml = ConvertToXML(info) ;
		String result = SendXMLToRedux(agent, xml) ;
		xml.delete() ;
		
		return result ;
	}
}
