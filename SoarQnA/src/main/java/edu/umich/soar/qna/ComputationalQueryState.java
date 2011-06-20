package edu.umich.soar.qna;


public abstract class ComputationalQueryState implements QueryState {
	
	protected boolean hasComputed;

	public void dispose() {
		hasComputed = true;
	}

	public boolean hasNext() {
		return !hasComputed;
	}

	public ComputationalQueryState() {
		hasComputed = true;
	}
}
