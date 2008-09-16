/**
 * 
 */
package soar2d.tosca2d;

import tosca.Group;
import tosca.Value;

/**
 * @author Doug
 *
 */
public class EatersOutputStateVariable extends JavaStateVariable {
	protected Value m_Value = new Value() ;

	public EatersOutputStateVariable(String name) {
		super(name) ;
		this.SetLogChanges(true);
	}
	
	public void Initialize() {
		Group g = new Group();
		g.AddNamedValue("_initialized", new tosca.Boolean(false));
		GetCurrentValue().TakeGroup(g);
	}

	public boolean IsInitialized(Value v) {
		return v.CastToGroup().GetNamedValue("_initialized").CastToBoolean().GetBoolean();
	}
	
	public int GetActionValue(Value v) {
		if (IsInitialized(v))
			return v.CastToGroup().GetNamedValue("value").CastToInteger().GetInteger();
		else
		{
			System.out.println("UNINITIALIZED");
			return 0;
		}
	}
	
	/** 
	 * Note: It's ONLY valid to call this within the tickEvent() handler -- which is called by InputFunctionModule while the clock is holding waiting for the function module to respond
	 * Returns 1-4 to make a move.  0 to not go anywhere.
	 * NAG: 5 opens a box with no argument (code) 
	 **/
	public int GetDirection()
	{
		int time = GetClock().GetTime() ;
		
		Value value = new Value() ;
		this.GetValue(value, time) ;
		
		int action = GetActionValue(value);			
		if (action < 5)
			return action;
		else
			return 0;			

	}
	
	public Boolean IsOpening()
	{
		int time = GetClock().GetTime() ;
		
		Value value = new Value() ;
		this.GetValue(value, time) ;
		
		if (GetActionValue(value) >= 5)
			return true;
		else
			return false;
	}
	
	public int GetOpenCode()
	{
		int time = GetClock().GetTime() ;
		
		Value value = new Value() ;
		this.GetValue(value, time) ;
		
		int action = GetActionValue(value);
		if (action == 5)
			return 0;
		else if (action == 6)
			return 1;
		else if (action == 7)
			return 2;
		else
			return 0;			
	}
	
	protected Value GetCurrentValue() { return m_Value ; }

}
