package edu.umich.soar.sps.control;


interface InputLinkInterface {
	void addWaypoint(double[] pos, String id, boolean useFloatYawWmes);
	void clearMessages();
	boolean disableWaypoint(String id);
	boolean enableWaypoint(String id, SplinterState splinter);
	boolean removeMessage(int id);
	boolean removeWaypoint(String id);
}
