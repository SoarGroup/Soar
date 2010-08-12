package edu.umich.soar.qna;


public abstract class ComputationalQueryState implements QueryState {
	
	protected boolean hasComputed;

	@Override
	public void dispose() {
		hasComputed = true;
	}

	@Override
	public boolean hasNext() {
		return !hasComputed;
	}

	public ComputationalQueryState() {
		hasComputed = true;
	}
}
