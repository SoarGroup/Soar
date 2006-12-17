/**
 * 
 */
package soar2d.tosca2d;

/**
 * @author Doug
 *
 */

import soar2d.World;
import tosca.Double;
import tosca.Value;

public class EatersInputStateVariable extends JavaStateVariable {
	protected Value m_Value = new Value() ;

	public EatersInputStateVariable(String name) {
		super(name) ;
	}
	
	public void Initialize() {
		GetCurrentValue().SetFromDouble(200.0) ;
	}

	public void update(int time, World world, java.awt.Point location) {
		Double val = new Double() ;
		val.SetFromDouble(time) ;
		Value value = new Value(val) ;
		
		SetValue(value, time) ;
	}
	
	protected Value GetCurrentValue() { return m_Value ; }
}
