package org.msoar.sps.config;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Queue;

/** Concrete implementation of ConfigSource using a file. **/
public class ConfigFile extends ConfigSource {
	String path;
	Map<String, String[]> keys = new HashMap<String, String[]>();

	public ConfigFile(String path) throws IOException {
		this.path = path;

		Tokenizer t = new Tokenizer(path);
		parse(t, "");
	}

	public ConfigFile() {
		this.path = "(in memory)";
	}

	public Config getConfig() {
		return new Config(this);
	}

	@Override
	public ConfigSource copy() {
		ConfigFile newConfigFile = new ConfigFile();
		newConfigFile.path = new String(this.path);
		newConfigFile.keys = new HashMap<String, String[]>(this.keys);
		
		return newConfigFile;
	}

	@Override
	public String[] getKeys(String rootWithDot) {
		List<String> subkeys = new ArrayList<String>();
		for (String key : keys.keySet()) {
			if (key.length() <= rootWithDot.length()) {
				continue;
			}
			if (key.startsWith(rootWithDot)) {
				String strippedKey = key.substring(rootWithDot.length());
				subkeys.add(strippedKey);
			}
		}
		return subkeys.toArray(new String[subkeys.size()]);
	}

	@Override
	public boolean hasKey(String key) {
		return keys.containsKey(key);
	}

	@Override
	public void removeKey(String key) {
		keys.remove(key);
	}
	
	@Override
	public int[] getInts(String key) {
		String vs[] = keys.get(key);
		if (vs == null)
			return null;

		int v[] = new int[vs.length];
		for (int i = 0; i < vs.length; i++) {
			v[i] = Integer.parseInt(vs[i]);
		}

		return v;
	}

	@Override
	public void setInts(String key, int v[]) {
		String s[] = new String[v.length];
		for (int i = 0; i < v.length; i++)
			s[i] = "" + v[i];
		keys.put(key, s);
	}

	@Override
	public boolean[] getBooleans(String key) {
		String vs[] = keys.get(key);
		if (vs == null)
			return null;

		boolean v[] = new boolean[vs.length];
		for (int i = 0; i < vs.length; i++) {
			v[i] = Boolean.parseBoolean(vs[i]);
		}

		return v;
	}

	@Override
	public void setBooleans(String key, boolean v[]) {
		String s[] = new String[v.length];
		for (int i = 0; i < v.length; i++)
			s[i] = "" + v[i];
		keys.put(key, s);
	}

	@Override
	public String[] getStrings(String key) {
		String vs[] = keys.get(key);
		if (vs == null)
			return null;

		return vs;
	}

	@Override
	public void setStrings(String key, String v[]) {
		if (v == null) {
			keys.remove(key);
		}
		keys.put(key, v);
	}

	@Override
	public double[] getDoubles(String key) {
		String vs[] = keys.get(key);
		if (vs == null)
			return null;

		double v[] = new double[vs.length];
		for (int i = 0; i < vs.length; i++) {
			v[i] = Double.parseDouble(vs[i]);
		}

		return v;
	}

	@Override
	public void setDoubles(String key, double v[]) {
		String s[] = new String[v.length];
		for (int i = 0; i < v.length; i++)
			s[i] = "" + v[i];
		keys.put(key, s);
	}

	@Override
	public byte[] getBytes(String key) {
		String lines[] = getStrings(key);
		if (lines == null)
			return null;

		return Base64.decode(lines);
	}

	@Override
	public void setBytes(String key, byte v[]) {
		keys.put(key, Base64.encode(v));
	}

	// ///////////////////////////////////////////////////////
	// File parsing below

	void parseError(Tokenizer t, String msg) {
		System.out.println("Parse error: " + msg);
		System.out.println("Near line " + t.lineNumber + ": " + t.line);
	}

	void parse(Tokenizer t, String keyroot) throws IOException {
		while (true) {

			if (!t.hasNext())
				return;

			// end of block?
			if (t.consume("}")) {
				if (keyroot.equals(""))
					parseError(t, "Unmatched } in input");

				return;
			}

			if (!t.hasNext())
				return;

			// parse a key block.
			String keypart = t.next();

			if (!t.hasNext()) {
				parseError(t, "Premature EOF");
				return;
			}

			// we have an enclosure block?
			if (t.consume("{")) {
				parse(t, keyroot + keypart + ".");
				continue;
			}

			// This is a key/value declaration.
			String[] valuesArray = null;
			String tok = t.next();
			if (tok.equals("=")) {
				List<String> values = new ArrayList<String>();

				if (t.consume("[")) {
					// read a list of values
					while (true) {
						tok = t.next();
						if (tok.equals("]"))
							break;
						values.add(tok);
						tok = t.peek();
						if (tok.equals(","))
							t.next();
					}
				} else {
					// read a single value
					values.add(t.next());
				}

				if (!t.consume(";")) {
					parseError(t, "Expected ; got " + tok);
					return;
				} 

				if (values.size() == 0) {
					parseError(t, "Expected values, didn't get any");
					return;
				}
				valuesArray = values.toArray(new String[values.size()]);

			} else {
				parseError(t, "Expected = got " + tok);
				return;
			}

			String key = keyroot + keypart;

			if (keys.get(key) != null) {
				parseError(t, "Duplicate key definition for: " + key);
				return;
			}
			
			keys.put(key, valuesArray);

			/*
			 * System.out.println(keyroot+keypart+" = "); for (String s :
			 * values) System.out.println("  "+s);
			 */
		}
	}

	class Tokenizer {
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
			String TOKSTOP = "[];{},=#";

			String tok = "";
			boolean in_string = false;

			for (int pos = 0; pos < line.length(); pos++) {
				char c = line.charAt(pos);

				if (in_string) {
					// in a string literal

//					if (c == '\\' && pos + 1 < line.length()) {
//						// escape sequence.
//						tok += line.charAt(pos + 1);
//						pos++;
//						continue;
//					}

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
	
	@Override
	public void save(String path, String rootWithDot) throws FileNotFoundException {
		PrintStream p = null;
		if (path != null) {
			FileOutputStream out = new FileOutputStream(path);
			p = new PrintStream(out);
		} else {
			p = System.out;
		}

		writeToStream(p, rootWithDot);
	 
		p.close();
	}

	@Override
	public void writeToStream(PrintStream p, String rootWithDot) {
		for (Map.Entry<String, String[]> entry : keys.entrySet()) {
			if (entry.getKey().length() <= rootWithDot.length()) {
				continue;
			}
			if (entry.getKey().startsWith(rootWithDot)) {
				String strippedKey = entry.getKey().substring(rootWithDot.length());
				p.print(strippedKey + " = ");
				
				assert entry.getValue().length > 0;
				if (entry.getValue().length < 2) {
					p.print("\"" + entry.getValue()[0] + "\"");
				} else {
					p.print("[");
					for ( String value : entry.getValue()) {
						p.print("\"" + value + "\"");
						p.print(",");
					}
					p.print("]");
				}
				
				p.println(";");
			}
		}
	}
}
