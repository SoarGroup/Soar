package edu.umich.soar.visualsoar.datamap;

public class DoNothingMatcherErrorHandler implements MatcherErrorHandler {
	public void badConstraint(edu.umich.soar.visualsoar.parser.Triple triple) {}
  public void generatedIdentifier(edu.umich.soar.visualsoar.parser.Triple triple, String element) {}
  public void generatedFloat(edu.umich.soar.visualsoar.parser.Triple triple, String element) {}
  public void generatedInteger(edu.umich.soar.visualsoar.parser.Triple triple, String element) {}
  public void generatedEnumeration(edu.umich.soar.visualsoar.parser.Triple triple, String element) {}
  public void generatedAddToEnumeration(edu.umich.soar.visualsoar.parser.Triple triple, String attribute, String value) {}
	public void noStateVariable() {}
	public void tooManyStateVariables() {}
}
