/********************************************************************************************
*
* TraceType.java
* 
* Description:	
* 
* Created on 	May 22, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package helpers;

/************************************************************************
 * 
 * Constants defining different types of information in a trace
 * (which we can then use for filtering).
 * 
 ************************************************************************/
public class TraceType
{
	public static final long
		kPhase 		= 1 << 0,
		kPreference = 1 << 1,
		kWmeChange  = 1 << 2,
		kFiring    	= 1 << 3,
		kRetraction	= 1 << 4,
		kStack     	= 1 << 5,
		kRhsWrite  	= 1 << 6,
		kLearning	= 1 << 7,
		kTopLevel  	= 1 << 8,		// Generic top level output (e.g. from a print or an echo'd command)
		kFullLearning = 1 << 9,
		kNumericIndifferent = 1 << 10,
		
		kAllExceptTopLevel = kPhase | kPreference | kWmeChange | kFiring | kRetraction | kStack | kRhsWrite | kLearning | kFullLearning | kNumericIndifferent,
		kAll = kAllExceptTopLevel | kTopLevel ;
}
