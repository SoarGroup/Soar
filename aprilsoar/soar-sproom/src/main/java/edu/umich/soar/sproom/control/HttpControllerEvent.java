package edu.umich.soar.sproom.control;

import java.util.List;

import edu.umich.soar.sproom.control.PIDController.Gains;

abstract class HttpControllerEvent {
	static class GainsChanged extends HttpControllerEvent {
		private final Gains hgains;
		private final Gains agains;
		private final Gains lgains;
		
		GainsChanged(Gains hgains, Gains agains, Gains lgains) {
			this.hgains = hgains;
			this.agains = agains;
			this.lgains = lgains;
		}
		
		Gains getHGains() {
			return hgains;
		}
		
		Gains getAGains() {
			return agains;
		}
		
		Gains getLGains() {
			return lgains;
		}
	}
	
	static class DDCChanged extends HttpControllerEvent {
		private final DifferentialDriveCommand ddc;
		
		DDCChanged(DifferentialDriveCommand ddc) {
			this.ddc = ddc;
		}
		
		DifferentialDriveCommand getDdc() {
			return ddc;
		}
	}
	
	static class MessageChanged extends HttpControllerEvent {
		private final List<String> tokens;
		
		MessageChanged(List<String> tokens) {
			this.tokens = tokens;
		}
		
		List<String> getTokens() {
			return tokens;
		}
	}
	
	
	static class SoarChanged extends HttpControllerEvent {
	}
}
