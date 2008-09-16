/**
 * 
 */
package soar2d.tosca2d;
import tosca.* ;

/**
 * @author Doug
 *
 */
public class J1 extends JavaFunctionModule {
	public J1(String name) {
		super(name) ;
	}
	
	@Override
	public boolean Initialize() {
		// Establish a connection to the input variables
		StateVariable a = GetController().ConnectStateVariable("A") ;
		StateVariable b = GetController().ConnectStateVariable("B") ;
		StateVariable c = GetController().ConnectStateVariable("C") ;
		StateVariable d = GetController().ConnectStateVariable("D") ;

		// Record the inputs and outputs so we can look them up by name locally, if we wish.
		AddInputVariable(a) ;
		AddInputVariable(b) ;
		AddInputVariable(c) ;

		AddOutputVariable(d) ;

		// Register with the clock
		GetClock().RegisterModule(this) ;

		// Ask to be woken up at time 1
		GetClock().RegisterWakeup(this, 1, true) ;

		return (a != null && b != null && c != null && d != null) ;
	}

	@Override
	public void ExecuteOneStep() {
		// Process any events we were sent while sleeping
		ProcessEvents() ;

		// Get the current clock time.  This value can't change until
		// we signal that it's ok for the clock to advance.
		int time = GetClock().GetTime() ;
		
		Log("Time is " + time) ;

		// Get the value of the inputs at the current time
		Value a = new Value() ;
		Value b = new Value() ;
		Value c = new Value() ;
		Value d = new Value() ;
		GetInput("A").GetValue(a, time) ;
		GetInput("B").GetValue(b, time) ;
		GetInput("C").GetValue(c, time) ;
		
		// Just for testing, loop D back into A
		GetOutput("D").GetValue(d, time) ;
		Log("Value of D is " + d.CastToDouble().GetDouble()) ;
		a.SetFromDouble(d.CastToDouble().GetDouble()) ;

		// We've read our inputs so the clock can continue while we work
		// but it can't go beyond the time when we generate output.
		// Changing the boundary time implicitly allows the clock to advance.
		GetClock().ClockCannotAdvanceBeyond(this, time+4) ;

		// Perform a calculation based on the inputs (could take long time)
		Value result = Calc(a, b, c) ;

		// Generate output 5ms after this function module was triggered
		GetOutput("D").SetValue(result, time+5) ;
		
		// Set up our next triggering event and allow the clock to advance
		// up to the time of this next trigger.
		GetClock().RegisterWakeup(this, time+10, true) ;
	}

	public Value Calc(Value a, Value b, Value c)
	{
		// Create a simple sum of 3 doubles
		Value result = new Value() ;
		result.SetFromDouble(a.CastToDouble().GetDouble() + b.CastToDouble().GetDouble() + c.CastToDouble().GetDouble()) ;
	
		// Simulate a long calculation
		try { java.lang.Thread.sleep(100) ; } catch (InterruptedException e) { }
		return result ;
	}
}

