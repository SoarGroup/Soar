#ifndef CONFIG_CONFIG_H
#define CONFIG_CONFIG_H

/**
 * Port of a configuration file utility used in Soar2D so that it is available
 * for use in C++. Not sure if we're even going to need it. This is essentially
 * an exercise in porting Java straight to C++.
 *
 * Original: SoarSuite/Environments/Soar2D/src/soar2d/config/
 *   Config.java
 *   ConfigFile.java
 *   ConfigSource.java
 *   ConfigUtil.java
 *
 * Until the initial port is done and working:
 *  * Memory issues such as who owns what pointers when is getting pushed aside
 *  * Everything is going to throw std::exception on errors, will sort out later
 */

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <exception>
#include <assert.h>

#include "config_Source.h"

namespace config {

	class Tokenizer {
	public:
	//		BufferedReader ins;

	//		// tokens belonging to the current line
	//		std::string line;
	//		int lineNumber = 0;
	//		Queue<std::string> tokens = new LinkedList<std::string>();

			Tokenizer(std::string path) {
	//			ins = new BufferedReader(new FileReader(path));
			}

	//		// doesn't support string literals spread across multiple lines.
	//		void tokenizeLine(std::string line) {
	//			std::string TOKSTOP = "[];{},=#";

	//			std::string tok = "";
	//			boolean in_string = false;

	//			for (int pos = 0; pos < line.length(); pos++) {
	//				char c = line.charAt(pos);

	//				if (in_string) {
	//					// in a string literal

	////					if (c == '\\' && pos + 1 < line.length()) {
	////						// escape sequence.
	////						tok += line.charAt(pos + 1);
	////						pos++;
	////						continue;
	////					}

	//					if (c == '\"') {
	//						// end of string.
	//						tokens.add(tok);
	//						in_string = false;
	//						tok = "";
	//						continue;
	//					}

	//					tok += c;

	//				} else {
	//					// NOT in a string literal

	//					// strip spaces when not in a string literal
	//					if (Character.isWhitespace(c))
	//						continue;

	//					// starting a string literal
	//					if (c == '\"' && tok.length() == 0) {
	//						in_string = true;
	//						continue;
	//					}

	//					// does this character end a token?
	//					if (TOKSTOP.indexOf(c) < 0) {
	//						// nope, add it to our token so far
	//						tok += c;
	//						continue;
	//					}

	//					// produce (up to) two new tokens: the accumulated token
	//					// which has just ended, and a token corresponding to the
	//					// new character.
	//					tok = tok.trim();
	//					if (tok.length() > 0) {
	//						tokens.add(tok);
	//						tok = "";
	//					}

	//					if (c == '#')
	//						return;

	//					// add this terminator character
	//					tok = "" + c;
	//					tok = tok.trim();
	//					if (tok.length() > 0) {
	//						tokens.add(tok);
	//						tok = "";
	//					}
	//				}
	//			}

	//			tok = tok.trim();
	//			if (tok.length() > 0)
	//				tokens.add(tok);

	//		}

	//		boolean hasNext() throws IOException {
	//			while (true) {
	//				if (tokens.size() > 0)
	//					return true;

	//				line = ins.readLine();
	//				lineNumber++;
	//				if (line == null)
	//					return false;

	//				tokenizeLine(line);
	//			}
	//		}

	//		// If the next token is s, consume it.
	//		boolean consume(std::string s) throws IOException {
	//			if (peek().equals(s)) {
	//				next();
	//				return true;
	//			}
	//			return false;
	//		}

	//		std::string peek() throws IOException {
	//			if (!hasNext())
	//				return null;

	//			return tokens.peek();
	//		}

	//		std::string next() throws IOException {
	//			if (!hasNext())
	//				return null;

	//			std::string tok = tokens.poll();
	//			return tok;
	//		}
	};

	class ConfigFile : ConfigSource 
	{
	private:
		std::string path;
		std::map< std::string, std::vector< std::string > > keys;

	public:
		ConfigFile(std::string path) 
		{
			this->path = path;

			Tokenizer t(path);
			parse(t, std::string(""));
		}

		ConfigFile() 
		{
			this->path = "(in memory)";
		}

	//	Config getConfig() {
	//		return new Config(this);
	//	}

	//	@Override
	//	ConfigSource copy() {
	//		ConfigFile newConfigFile = new ConfigFile();
	//		newConfigFile.path = new std::string(this->path);
	//		newConfigFile.keys = new HashMap<std::string, std::vector<std::string>>(this->keys);
	//		
	//		return newConfigFile;
	//	}

	//	@Override
	//	std::vector<std::string> getKeys(std::string root) {
	//		if (!root.equals(""))
	//			root += ".";

	//		List<std::string> subkeys = new ArrayList<std::string>();

	//		for (std::string key : keys.keySet()) {
	//			if (key.startsWith(root))
	//				subkeys.add(key);
	//		}

	//		return subkeys.toArray(new std::string[0]);
	//	}

	//	@Override
	//	boolean hasKey(std::string key) {
	//		return keys.containsKey(key);
	//	}

	//	@Override
	//	void removeKey(std::string key) {
	//		keys.remove(key);
	//	}
	//	
	//	@Override
	//	std::vector<std::string> keyList(std::string prefix) {
	//		List<std::string> keyList = new ArrayList<std::string>();
	//		for (std::string key : keys.keySet()) {
	//			if (key.length() <= prefix.length()) {
	//				continue;
	//			}
	//			if (key.startsWith(prefix)) {
	//				std::string strippedKey = key.substring(prefix.length());
	//				keyList.add(strippedKey);
	//			}
	//		}
	//		return keyList.toArray(new std::string[keyList.size()]);
	//	}
	//	
	//	@Override
	//	int[] getInts(std::string key) {
	//		std::string vs[] = keys.get(key);
	//		if (vs == null)
	//			return null;

	//		int v[] = new int[vs.length];
	//		for (int i = 0; i < vs.length; i++) {
	//			v[i] = Integer.parseInt(vs[i]);
	//		}

	//		return v;
	//	}

	//	@Override
	//	void setInts(std::string key, int v[]) {
	//		std::string s[] = new std::string[v.length];
	//		for (int i = 0; i < v.length; i++)
	//			s[i] = "" + v[i];
	//		keys.put(key, s);
	//	}

	//	@Override
	//	boolean[] getBooleans(std::string key) {
	//		std::string vs[] = keys.get(key);
	//		if (vs == null)
	//			return null;

	//		boolean v[] = new boolean[vs.length];
	//		for (int i = 0; i < vs.length; i++) {
	//			v[i] = Boolean.parseBoolean(vs[i]);
	//		}

	//		return v;
	//	}

	//	@Override
	//	void setBooleans(std::string key, boolean v[]) {
	//		std::string s[] = new std::string[v.length];
	//		for (int i = 0; i < v.length; i++)
	//			s[i] = "" + v[i];
	//		keys.put(key, s);
	//	}

	//	@Override
	//	std::vector<std::string> getStrings(std::string key) {
	//		std::string vs[] = keys.get(key);
	//		if (vs == null)
	//			return null;

	//		return vs;
	//	}

	//	@Override
	//	void setStrings(std::string key, std::string v[]) {
	//		keys.put(key, v);
	//	}

	//	@Override
	//	double[] getDoubles(std::string key) {
	//		std::string vs[] = keys.get(key);
	//		if (vs == null)
	//			return null;

	//		double v[] = new double[vs.length];
	//		for (int i = 0; i < vs.length; i++) {
	//			v[i] = Double.parseDouble(vs[i]);
	//		}

	//		return v;
	//	}

	//	@Override
	//	void setDoubles(std::string key, double v[]) {
	//		std::string s[] = new std::string[v.length];
	//		for (int i = 0; i < v.length; i++)
	//			s[i] = "" + v[i];
	//		keys.put(key, s);
	//	}

	//	@Override
	//	byte[] getBytes(std::string key) {
	//		std::string lines[] = getStrings(key);
	//		if (lines == null)
	//			return null;

	//		return Base64.decode(lines);
	//	}

	//	@Override
	//	void setBytes(std::string key, byte v[]) {
	//		keys.put(key, Base64.encode(v));
	//	}

	//	// ///////////////////////////////////////////////////////
	//	// File parsing below

	//	void parseError(Tokenizer t, std::string msg) {
	//		System.out.println("Parse error: " + msg);
	//		System.out.println("Near line " + t.lineNumber + ": " + t.line);
	//	}

		void parse(Tokenizer /*t*/, std::string /*keyroot*/) {
	//		while (true) {

	//			if (!t.hasNext())
	//				return;

	//			// end of block?
	//			if (t.consume("}")) {
	//				if (keyroot.equals(""))
	//					parseError(t, "Unmatched } in input");

	//				return;
	//			}

	//			if (!t.hasNext())
	//				return;

	//			// parse a key block.
	//			std::string keypart = t.next();

	//			if (!t.hasNext()) {
	//				parseError(t, "Premature EOF");
	//				return;
	//			}

	//			// we have an enclosure block?
	//			if (t.consume("{")) {
	//				parse(t, keyroot + keypart + ".");
	//				continue;
	//			}

	//			// This is a key/value declaration.
	//			std::vector<std::string> valuesArray = null;
	//			std::string tok = t.next();
	//			if (tok.equals("=")) {
	//				List<std::string> values = new ArrayList<std::string>();

	//				if (t.consume("[")) {
	//					// read a list of values
	//					while (true) {
	//						tok = t.next();
	//						if (tok.equals("]"))
	//							break;
	//						values.add(tok);
	//						tok = t.peek();
	//						if (tok.equals(","))
	//							t.next();
	//					}
	//				} else {
	//					// read a single value
	//					values.add(t.next());
	//				}

	//				if (!t.consume(";")) {
	//					parseError(t, "Expected ; got " + tok);
	//					return;
	//				} 
	//				
	//				valuesArray = values.toArray(new std::string[values.size()]);

	//			} else if (tok.equals(";")) {
	//				// use null valuesArray
	//				valuesArray = null; // redundant, but more clear
	//				
	//			} else {
	//				parseError(t, "Expected = or ; got " + tok);
	//				return;
	//			}

	//			std::string key = keyroot + keypart;

	//			if (keys.get(key) != null) {
	//				parseError(t, "Duplicate key definition for: " + key);
	//			}

	//			keys.put(key, valuesArray);

	//			/*
	//			 * System.out.println(keyroot+keypart+" = "); for (std::string s :
	//			 * values) System.out.println("  "+s);
	//			 */
	//		}
		}

	//	
	//	@Override
	//	void save(std::string path) throws FileNotFoundException {
	//		PrintStream p = null;
	//		if (path != null) {
	//			FileOutputStream out = new FileOutputStream(path);
	//			p = new PrintStream(out);
	//		} else {
	//			p = System.out;
	//		}
	//		
	//		
	//		for (Map.Entry<std::string, std::vector<std::string>> entry : keys.entrySet()) {
	//			p.print(entry.getKey() + " = ");
	//			
	//			assert entry.getValue().length > 0;
	//			if (entry.getValue().length < 2) {
	//				p.print("\"" + entry.getValue()[0] + "\"");
	//			} else {
	//				p.print("[");
	//				for ( std::string value : entry.getValue()) {
	//					p.print("\"" + value + "\"");
	//					p.print(",");
	//				}
	//				p.print("]");
	//			}
	//			
	//			p.println(";");
	//		}
	//	 
	//		p.close();
	//	}

	};
}

#endif // CONFIG_FILE_H
