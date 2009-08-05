package edu.umich.soar.gridmap2d.players;

public interface Commander {
	public CommandInfo nextCommand();
	public void reset();
	public void shutdown();
}
