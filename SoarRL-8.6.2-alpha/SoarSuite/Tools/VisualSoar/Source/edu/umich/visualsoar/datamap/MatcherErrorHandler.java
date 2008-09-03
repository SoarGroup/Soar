package edu.umich.visualsoar.datamap;
import edu.umich.visualsoar.parser.Triple;

public interface MatcherErrorHandler {
	void badConstraint(Triple triple);
  void generatedIdentifier(Triple triple, String element);
  void generatedFloat(Triple triple, String element);
  void generatedInteger(Triple triple, String element);
  void generatedEnumeration(Triple triple, String element);
  void generatedAddToEnumeration(Triple triple, String attribute, String value);
	void noStateVariable();
	void tooManyStateVariables();
}
