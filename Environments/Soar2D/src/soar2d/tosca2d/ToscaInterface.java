/**
 * 
 */
package soar2d.tosca2d;

import soar2d.Soar2D;
import tosca.FunctionModule;
import tosca.Library;
import tosca.StateVariable;
import tosca.Value;
import tosca.Vector;

/**
 * @author Doug
 *
 * High level interface to the Tosca framework.
 * 
 * This class handles startup, shutdown etc.
 * The specific agent I/O happens in ToscaEater.
 */
public class ToscaInterface {
	// Test code -- remove once things are working
	private static ToscaInterface s_Tosca = null ;
	private LoaderJNI 		m_Loader = null ;
	private tosca.Library   m_EatersLibrary = null ;
	private int				m_AgentNumber = 0 ;
	private EatersInputFunctionModule m_InputModule ;	// BADBAD: I think we have to stop this from being garbage collected.  In C++ we pass it to Library and it takes ownership--that's not clear to SWIG.
	private boolean			m_ThreadsRunning = false ;
	
	public static ToscaInterface getTosca() {
		if (s_Tosca == null)
		{
			s_Tosca = new ToscaInterface() ;
			s_Tosca.loadLibrary() ;
		}
		
		return s_Tosca ;
	}
	
	public int generateNewAgentNumber() {
		m_AgentNumber++ ;
		return m_AgentNumber ;
	}
	
	protected void loadLibrary() {
		LoaderJNI loader = new LoaderJNI("EatersDLL") ;
		long rawLibrarian = loader.Librarian_GetExistingLibrarian() ;
		m_Loader = loader ;
		
	    //tosca.Librarian librarian = tosca.Librarian.GetLibrarian() ;
		tosca.Librarian librarian = tosca.Librarian.UseExistingLibrarian(rawLibrarian) ;
	    int libs = librarian.GetNumberLibraries() ;
	    System.out.println("Number of libraries is " + libs) ;
	    tosca.Library library = librarian.GetLibraryByName("eaters") ;
	    if (library == null)
	    	System.out.println("Failed to find C++ library") ;
	    else
	    	System.out.println("Found C++ library") ;

	    m_EatersLibrary = library ;
	    
	    if (m_EatersLibrary == null)
	    	throw new IllegalStateException("Failed to load C++ Tosca library for eaters") ;
	    
	    // Add in our EatersInputFunctionModule
		// BUGBUG: Shouldn't be per agent
	    EatersInputFunctionModule inputModule = new EatersInputFunctionModule("inputModule") ;
	    m_InputModule = inputModule ;	// Prevent garbage collection.  BUGBUG: Should inform SWIG of how this works
	    library.AddFunctionModule(inputModule) ;
	    
	    /*
	    // Initialize the input for the clustering code.
	    // This is unrelated to Eaters, but provides a test for now.
	    int size = 5 ;
	    Vector inputVector = new Vector(size) ;
	    for (int i = 0 ; i < size ; i++)
	    	inputVector.Set(i, 1.0) ;
	    Value inputValue = new Value(inputVector) ;
	    
	    final StateVariable input = library.GetStateVariable("Input") ;
	    input.SetValue(inputValue, 1) ;	  
	    */  
	}
	
	public Library getToscaLibrary() {
		return m_EatersLibrary ;
	}
	
	public void runStep() { 
		runTosca(true) ;
	}
	
	public void runForever() {
		runTosca(false) ;
	}
	
	protected void runTosca(boolean step) {
		tosca.Clock clock = getToscaLibrary().GetClock() ;
		
		Soar2D.control.startEvent() ;
		
		// BADBAD: For now make sure we don't really run forever
		if (step)
		{
			int time = clock.GetTime() ;

			int stepSize = m_InputModule.getStepSize() ;
			clock.SetTimeLimit(time + stepSize) ;
		}
		else
		{
			// Run forever
			clock.SetTimeLimit(-1) ;
		}
		
		clock.ReleaseClock() ;

		if (!m_ThreadsRunning)
		{
			m_ThreadsRunning = true ;

			// Note: Had to hold off on this until the input variable has been created
			// so the other C++ modules can find it and hook up to it successfully.
			getToscaLibrary().InitializeAll() ;
			getToscaLibrary().StartAllThreads(true) ;
		}

		// Wait until we're stopped.
		while (!Soar2D.control.isStopped() && !clock.IsHolding())
		{
			try { Thread.sleep(50) ; } catch (InterruptedException ex) { }
		}
		
		// Stop the run if we get here.
		clock.HoldClock() ;

		/*
			System.out.println("**** Stopping threads") ;
			//clock.Shutdown() ;
			//clock.StopOwnThread(true) ;
			//m_InputModule.stopJavaThread(true) ;
			getToscaLibrary().StopAllThreads(true, true) ;
			System.out.println("**** Stopped all threads") ;
		*/
		
		Soar2D.control.stopEvent() ;		
	}
		
	public void testCode() {
		// Loads our test C++ code and gets a pointer to it
		LoaderJNI loader = new LoaderJNI("EatersDLL") ;
		long rawLibrarian = loader.Librarian_GetExistingLibrarian() ;
		m_Loader = loader ;
		
	    //tosca.Librarian librarian = tosca.Librarian.GetLibrarian() ;
		tosca.Librarian librarian = tosca.Librarian.UseExistingLibrarian(rawLibrarian) ;
	    int libs = librarian.GetNumberLibraries() ;
	    System.out.println("Number of libraries is " + libs) ;
	    tosca.Library library = librarian.GetLibraryByName("eaters") ;
	    if (library == null)
	    	System.out.println("Failed to find C++ library") ;
	    else
	    	System.out.println("Found C++ library") ;

	    // Initialize the input in Java (this is the key process--providing input from Java as an environment to C++ clustering code)
	    int size = 5 ;
	    Vector inputVector = new Vector(size) ;
	    for (int i = 0 ; i < size ; i++)
	    	inputVector.Set(i, 1.0) ;
	    Value inputValue = new Value(inputVector) ;
	    
	    // BUGBUG: If this gets garbage collected after threads have shutdown/everything's over we seem
	    // to get a crash.  Need to explore the behavior of Value objects in Java better.
	    // OK, in C++ the ref count for the inputVector's internal object would increase.
	    // The problem is perhaps that we delete that object rather than releasing it.
	    //m_Value = inputValue ;
	    
	    final StateVariable input = library.GetStateVariable("Input") ;
	    input.SetValue(inputValue, 1) ;
	    
		final tosca.Clock	clock = library.GetClock() ;

		boolean start = true ;
		
		// Print out all function modules
		int nModules = library.GetNumberFunctionModules() ;
		for (int i = 0 ; i < nModules ; i++)
			System.out.println("Module " + i + " has name " + library.GetFunctionModuleName(i)) ;
		
		if (start)
		{
			clock.SetTimeLimit(100) ;
			//clock.StartOwnThread() ;
			
			/*
			final FunctionModule findWinner = library.GetFunctionModule("findWinner") ;
			final FunctionModule updateWeights = library.GetFunctionModule("updateWeights") ;
			final FunctionModule updateInput = library.GetFunctionModule("updateInput") ;
			findWinner.StartOwnThread() ;
			updateWeights.StartOwnThread() ;
			updateInput.StartOwnThread() ;
			*/
			
			library.StartAllThreads(true);
	
			// Sleep for 2 secs, 0ms
			try { Thread.sleep(2000) ; } catch (InterruptedException ex) { }

			library.StopAllThreads(true, true);
			//System.exit(0) ;
		}
		
		System.out.println("===================================") ;
		System.out.println("====    Finished TestGroup     ====") ;
		System.out.println("===================================") ;
	}
}
