package edu.umich.soar.gridmap2d.players;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.Direction;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Names;


/**
 * @author voigtjr
 *
 * output required for interaction with the world
 */
public class CommandInfo {
	private static Logger logger = Logger.getLogger(CommandInfo.class);

	// all
	public boolean stopSim = false;	// stop the simulation by command
	
	// eaters + tanksoar
	public boolean move = false;	// move
	public Direction moveDirection = Direction.NONE;	// direction to move
	
	// eaters
	public boolean open = false;	// open the box on the current cell
	public boolean jump = false;	// jump if we move
	public boolean dontEat = false;	// don't eat food
	
	// tanksoar + room
	public boolean rotate = false;	// rotate
	public String rotateDirection;	// Which way to rotate, must be valid if rotate true
	
	// tanksoar
	public boolean fire = false;	// fire the tank cannon
	public boolean radar = false;	// change radar status
	public boolean radarSwitch = false;	// status to change radar to
	public boolean radarPower = false;	// change radar power setting
	public int radarPowerSetting = -1;	// setting to change radar power to
	public boolean shields = false;	// change shields status
	public boolean shieldsSetting = false;	// setting to change shields to
	
	// room
	public boolean forward = false;	// move forward
	public boolean backward = false;	// move backward
	public boolean rotateAbsolute = false;	// rotate to a heading
	public double rotateAbsoluteHeading;	// what heading to stop at
	public boolean rotateRelative = false;	// rotate tank
	public double rotateRelativeYaw;		// how far to rotate
	public boolean get = false;
	public int [] getLocation = null;
	public int getId;
	public boolean drop = false;
	public int dropId;

	// taxi
	public boolean pickup = false;
	public boolean putdown = false;
	public boolean fillup = false;
	
	public class Communication {
		public String to;
		public String message;
	}
	public List<Communication> messages = new ArrayList<Communication>();
	
	public CommandInfo() {
	}
	
	private static void parseError(Tokenizer t, String msg) {
		logger.error("Command script parse error: " + msg);
		logger.error("Near line " + t.lineNumber + ": " + t.line);
	}

	public static List<CommandInfo> loadScript(String path) throws IOException {
		CommandInfo.Tokenizer t = new CommandInfo.Tokenizer(path);
		List<CommandInfo> commands = new ArrayList<CommandInfo>();
		
		while (true) {
			if (!t.hasNext())
				return commands;

			if (!t.consume("(")) {
				parseError(t, "Expected ( got " + t.peek());
			}
			CommandInfo command = new CommandInfo();
			String tok = t.next();
			if (tok.equals(Names.kJumpID)) {
				command.jump = true;
			} else if (tok.equals(Names.kMoveID)) {
				command.move = true;
				if (!t.consume(":")) {
					parseError(t, "Expected :");
					return commands;
				}
				command.moveDirection = Direction.parse(t.next());
			} else if (tok.equals(Names.kDontEatID)) {
				command.dontEat = true;
			} else if (tok.equals(Names.kOpenID)) {
				command.open = true;
			}
			
			if (!t.consume(")")) {
				parseError(t, "Expected ) got " + t.peek());
			}
			
			commands.add(command);
		}
	}
	
	private static class Tokenizer {
		BufferedReader ins;

		// tokens belonging to the current line
		String line;
		int lineNumber = 0;
		Queue<String> tokens = new LinkedList<String>();

		public Tokenizer(String path) throws IOException {
			ins = new BufferedReader(new FileReader(path));
		}

		// doesn't support string literals spread across multiple lines.
		void tokenizeLine(String line) {
			String TOKSTOP = "():";

			String tok = "";
			boolean in_string = false;

			for (int pos = 0; pos < line.length(); pos++) {
				char c = line.charAt(pos);

				if (in_string) {
					// in a string literal
					if (c == '\"') {
						// end of string.
						tokens.add(tok);
						in_string = false;
						tok = "";
						continue;
					}

					tok += c;

				} else {
					// NOT in a string literal

					// strip spaces when not in a string literal
					if (Character.isWhitespace(c))
						continue;

					// starting a string literal
					if (c == '\"' && tok.length() == 0) {
						in_string = true;
						continue;
					}

					// does this character end a token?
					if (TOKSTOP.indexOf(c) < 0) {
						// nope, add it to our token so far
						tok += c;
						continue;
					}

					// produce (up to) two new tokens: the accumulated token
					// which has just ended, and a token corresponding to the
					// new character.
					tok = tok.trim();
					if (tok.length() > 0) {
						tokens.add(tok);
						tok = "";
					}

					if (c == '#')
						return;

					// add this terminator character
					tok = "" + c;
					tok = tok.trim();
					if (tok.length() > 0) {
						tokens.add(tok);
						tok = "";
					}
				}
			}

			tok = tok.trim();
			if (tok.length() > 0)
				tokens.add(tok);

		}

		public boolean hasNext() throws IOException {
			while (true) {
				if (tokens.size() > 0)
					return true;

				line = ins.readLine();
				lineNumber++;
				if (line == null)
					return false;

				tokenizeLine(line);
			}
		}

		// If the next token is s, consume it.
		public boolean consume(String s) throws IOException {
			if (peek().equals(s)) {
				next();
				return true;
			}
			return false;
		}

		public String peek() throws IOException {
			if (!hasNext())
				return null;

			return tokens.peek();
		}

		public String next() throws IOException {
			if (!hasNext())
				return null;

			String tok = tokens.poll();
			return tok;
		}
	}
	
	public String toString() {
		String output = new String();
		
		switch(Gridmap2D.config.game()) {
		case EATERS:
			if (jump) {
				output += "(" + Names.kJumpID + ")";
			} 
			if (move) {
				output += "(" + Names.kMoveID + ": " + moveDirection.id() + ")";
			}
			
			if (dontEat) {
				output += "(" + Names.kDontEatID + ")";
			}
			if (open) {
				output += "(" + Names.kOpenID + ")";
			}
			break;
			
		case TANKSOAR:
			if (move) {
				output += "(" + Names.kMoveID + ": " + moveDirection.id() + ")";
			}
			if (rotate) {
				output += "(" + Names.kRotateID + ": " + rotateDirection + ")";			
			}
			if (fire) {
				output += "(" + Names.kFireID + ")";
			}
			if (radar) {
				output += "(" + Names.kRadarID + ": " + (radarSwitch ? "on" : "off") + ")";
			}
			if (radarPower) {
				output += "(" + Names.kRadarPowerID + ": " + Integer.toString(radarPowerSetting) + ")";
			}
			if (shields) {
				output += "(" + Names.kShieldsID + ": " + (shieldsSetting ? "on" : "off") + ")";
			}
			break;
			
		case ROOM:
			if (forward) {
				output += "(" + Names.kForwardID + ")";
			}
			if (backward) {
				output += "(" + Names.kBackwardID + ")";
			}
			if (rotate) {
				output += "(" + Names.kRotateID + ": " + rotateDirection + ")";			
			}
			if (rotateAbsolute) {
				output += "(" + Names.kRotateAbsoluteID + ": " + rotateAbsoluteHeading + ")";			
			}
			if (rotateRelative) {
				output += "(" + Names.kRotateRelativeID + ": " + rotateRelativeYaw + ")";			
			}
			if (get) {
				output += "(" + Names.kGetID + ": " + getId + ": " + getLocation[0] + "," + getLocation[1] + ")";
			}
			if (drop) {
				output += "(" + Names.kDropID + ": " + dropId + ")";
			}
			Iterator<Communication> iter = messages.iterator();
			while (iter.hasNext()) {
				Communication comm = iter.next();
				output += "(comm: " + comm.to + ": " + comm.message + ")";
			}
			break;

		case TAXI:
			if (move) {
				output += "(" + Names.kMoveID + ": " + moveDirection.id() + ")";
			}
			if (pickup) {
				output += "(" + Names.kPickUpID + ")";
			}
			if (putdown) {
				output += "(" + Names.kPutDownID + ")";
			}
			if (fillup) {
				output += "(" + Names.kFillUpID + ")";
			}
			break;
			
		}
		
		if (stopSim) {
			output += "(" + Names.kStopSimID + ")";
		}
		
		return output;
	}

}
