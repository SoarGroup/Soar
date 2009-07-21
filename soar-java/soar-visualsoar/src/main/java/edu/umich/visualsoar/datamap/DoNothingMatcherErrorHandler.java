package edu.umich.visualsoar.datamap;

public class DoNothingMatcherErrorHandler implements MatcherErrorHandler {
	public void badConstraint(edu.umich.visualsoar.parser.Triple triple) {}
  public void generatedIdentifier(edu.umich.visualsoar.parser.Triple triple, String element) {}
  public void generatedFloat(edu.umich.visualsoar.parser.Triple triple, String element) {}
  public void generatedInteger(edu.umich.visualsoar.parser.Triple triple, String element) {}
  public void generatedEnumeration(edu.umich.visualsoar.parser.Triple triple, String element) {}
  public void generatedAddToEnumeration(edu.umich.visualsoar.parser.Triple triple, String attribute, String value) {}
	public void noStateVariable() {}
	public void tooManyStateVariables() {}
}
