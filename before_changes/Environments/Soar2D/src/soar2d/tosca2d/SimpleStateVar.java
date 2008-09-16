package soar2d.tosca2d;

import tosca.*;

public class SimpleStateVar extends JavaStateVariable {
	protected Value m_Value = new Value() ;

	public SimpleStateVar(String name) {
		super(name) ;
	}
	
	public void Initialize() {
		GetCurrentValue().SetFromDouble(200.0) ;
	}

	protected Value GetCurrentValue() { return m_Value ; }
}
