/**
 * 
 */
package soar2d.tosca2d;

import tosca.FunctionModule;

/**
 * Abstract base class for Function Module.  SWIG's auto generated
 * class isn't abstract when it should be.
 * 
 * @author Doug
 *
 */
public abstract class JavaFunctionModule extends FunctionModule {
	public JavaFunctionModule(String name) {
		super(name) ;
	}
	
	public abstract boolean Initialize() ;
	
	public abstract void ExecuteOneStep() ;
}
