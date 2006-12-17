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
 * The SWIG class is fine, but the methods that are abstract in the C++ class aren't
 * abstract in the Java class, so you're not forced to override them as you should be.
 * Using this class ensures that the correct overrides exist.
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
