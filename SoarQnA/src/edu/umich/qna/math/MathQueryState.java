package edu.umich.qna.math;

import edu.umich.qna.QueryState;

public abstract class MathQueryState implements QueryState {
	
	boolean hasComputed;
	
	MathQueryState() {
		hasComputed = true;
	}

	@Override
	public void dispose() {
		hasComputed = true;
	}

	@Override
	public boolean hasNext() {
		return !hasComputed;
	}

}
