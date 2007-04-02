/**
 * 
 */
package soar2d.tosca2d;

import soar2d.Soar2D;

/**
 * @author Doug
 *
 * This function module runs the eaters simulation when Tosca is running things.
 * It's main job is to call tickEvent() which in turn calls the update methods
 * and causes the input state variable to change value.
 * 
 */
public class EatersInputFunctionModule extends JavaFunctionModule {
	private Thread	m_JavaThread ;
	
	public EatersInputFunctionModule(String name) {
		super(name) ;
	}
	
	public int getStepSize() {
		return 20 ;
	}

	@Override
	public boolean Initialize() {
		GetClock().RegisterModule(this) ;
		
		// Initial wakeup
		GetClock().RegisterWakeup(this, 1, true) ;
		
		return false;
	}
	
	@Override
	public void ExecuteOneStep() {
		int time = GetClock().GetTime() ;

		// Just make things move slow enough that I can see what's happening
		try { java.lang.Thread.sleep(100) ; } catch (Exception ex) { }
		
		Soar2D.control.tickEvent() ;

		// Next wakeup
		GetClock().RegisterWakeup(this, time+getStepSize(), true) ;
	}

}
