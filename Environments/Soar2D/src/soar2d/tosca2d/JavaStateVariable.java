/**
 * 
 */
package soar2d.tosca2d;

import tosca.StateVariable;
import tosca.Value;

/**
 * JavaStateVariable provides the correct abstract behavior for StateVariable.
 * We should eventually figure out how to have SWIG generate this.
 * 
 * @author Doug
 *
 */
public abstract class JavaStateVariable extends StateVariable {
	public JavaStateVariable(String name) {
		super(name) ;
	}
	
	protected abstract Value GetCurrentValue() ;
	
	public abstract void Initialize() ;
}
