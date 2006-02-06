package edu.umich.visualsoar.parser;

/**
 * @author Brad Jones
 * @version 0.75 3 Mar 2000
 */
public final class SoarProduction {
	// Data Members
	private String d_name;
	private int d_startLine;
	private String d_comment;
	private String d_productionType;
	private ConditionSide d_conditionSide;
	private ActionSide d_actionSide;
	
	// Constructors
	public SoarProduction() {}
		
	// Accessors
	public final void setName(String name) {
		d_name = new String(name);
	}
	
	public final void setComment(String comment) {
		d_comment = new String(comment);
	}
	
	public final void setProductionType(String productionType) {
		d_productionType = new String(productionType);
	}
	
	public final void setStartLine(int startLine) {
		d_startLine = startLine;
	}
	
	public final void setConditionSide(ConditionSide cs) {
		d_conditionSide = cs;
	}
	
	public final void setActionSide(ActionSide as) {
		d_actionSide = as;
	}
	
	public final int getStartLine() {
		return d_startLine;
	}
	
	public final String getName() {
		return d_name;
	}
	
	public final String getComment() {
		return d_comment;
	}
	
	public final String getProductionType() {
		return d_productionType;
	}
	
	public final ConditionSide getConditionSide() {
		return d_conditionSide;
	}
	
	public final ActionSide getActionSide() {
		return d_actionSide;
	}
}
