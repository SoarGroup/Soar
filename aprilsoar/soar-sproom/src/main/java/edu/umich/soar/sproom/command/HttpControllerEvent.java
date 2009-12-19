package edu.umich.soar.sproom.command;

import java.util.List;

abstract class HttpControllerEvent {
	static class GainsChanged extends HttpControllerEvent {
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
