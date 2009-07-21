package edu.umich.soar.gridmap2d.players;

public interface Commander {
	public CommandInfo nextCommand() throws Exception;
	public void reset() throws Exception;
	public void shutdown() throws Exception;
}
