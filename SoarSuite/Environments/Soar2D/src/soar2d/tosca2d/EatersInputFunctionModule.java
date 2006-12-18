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
	
	public void startJavaThread() {
		// This works for Java created objects.  Using StartOwnThread the
		// thread doesn't terminate cleanly.  Odd.
		Runnable inputRunnable = new Runnable() {
			public void run()
			{
				EatersInputFunctionModule.this.Run() ;
			}
		} ;
		m_JavaThread = new Thread(inputRunnable) ;
		m_JavaThread.start() ;		
	}
	
	public void stopJavaThread(boolean waitTilStopped) {
		
		// Signal the thread to stop
		this.Shutdown() ;
		
		// Wait for stop (if requested)
		while (waitTilStopped && m_JavaThread.isAlive())
		{
			try { Thread.sleep(10) ; } catch (Exception ex) { }
		}
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
		
		Soar2D.control.tickEvent() ;

		// Next wakeup
		GetClock().RegisterWakeup(this, time+10, true) ;
	}

}
