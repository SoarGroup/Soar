package edu.umich.soar.visualsoar.parser;

public interface TripleFactory {
	Triple createTriple(Pair variable,Pair attribute,Pair value,boolean hasState,boolean isChecking);
  Triple createTriple(Pair variable,Pair attribute,Pair value,boolean hasState,boolean isChecking,boolean isCondition);
}
