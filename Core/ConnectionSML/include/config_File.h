#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

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
#include <sstream>
#include <exception>
#include <assert.h>
#include <queue>
#include <fstream>

#include "config_Source.h"
#include "config_Config.h"

namespace config {

	class Tokenizer {
	private:
		static std::ifstream ins;

		// tokens belonging to the current line
		std::string line;
		int lineNumber;
		std::queue< std::string > tokens;

	public:
		Tokenizer(std::string path) 
			: lineNumber(0) 
		{
			ins.open(path.c_str());
		}

		// doesn't support string literals spread across multiple lines.
		void tokenizeLine(std::string line) {
			std::string TOKSTOP = "[];{},=#";

			std::string tok;
			bool in_string = false;

			for (unsigned pos = 0; pos < line.length(); pos++) {
				char c = line.at(pos);

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
						tokens.push(tok);
						in_string = false;
						tok = "";
						continue;
					}

					tok.append(1, c);

				} else {
					// NOT in a string literal

					// strip spaces when not in a string literal
					if (isspace(c)) {
						continue;
					}

					// starting a string literal
					if (c == '\"' && tok.length() == 0) {
						in_string = true;
						continue;
					}

					// does this character end a token?
					if (TOKSTOP.find(c) != std::string::npos) {
						// nope, add it to our token so far
						tok.append(1, c);
						continue;
					}

					// produce (up to) two new tokens: the accumulated token
					// which has just ended, and a token corresponding to the
					// new character.
					tok = trim(tok);
					if (tok.length() > 0) {
						tokens.push(tok);
						tok = "";
					}

					if (c == '#')
						return;

					// add this terminator character
					tok.append(1, c);
					tok = trim(tok);
					if (tok.length() > 0) {
						tokens.push(tok);
						tok = "";
					}
				}
			}

			tok = trim(tok);
			if (tok.length() > 0)
				tokens.push(tok);

		}

		std::string trim(std::string str) {
			size_t left = str.find_first_of(" \t");
			str = str.substr(left);
			while( str.length() && isspace(str.at(str.length() - 1))) {
				str.erase(str.length() - 1);
			}
			return str;
		}

		bool hasNext() {
			while (true) {
				if (tokens.size() > 0)
					return true;

				std::getline(ins, line);
				if (!ins)
					return false;
				lineNumber++;

				tokenizeLine(line);
			}
		}

		// If the next token is s, consume it.
		bool consume(std::string s) {
			std::string peekTok = peek();
			if (peek() == s) {
				next();
				return true;
			}
			return false;
		}

		std::string peek() {
			if (!hasNext())
				return "";

			return tokens.front();
		}

		std::string next() {
			if (!hasNext())
				return "";

			std::string tok = tokens.front();
			tokens.pop();
			return tok;
		}
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

		Config* getConfig() {
			return new Config(this);
		}

		ConfigSource* copy() {
			ConfigFile* newConfigFile = new ConfigFile();
			newConfigFile->path = this->path;
			
			std::map< std::string, std::vector< std::string > >::const_iterator iter = this->keys.begin();
			for (; iter != this->keys.end(); ++iter) {
				newConfigFile->keys[ iter->first ] = std::vector< std::string >(iter->second);
			}

			return newConfigFile;
		}

		std::vector<std::string> getKeys(std::string root) {
			if (root.size())
				root.append(".");

			std::vector<std::string> subkeys;

			std::map< std::string, std::vector< std::string > >::const_iterator iter = this->keys.begin();
			for (; iter != this->keys.end(); ++iter) {
				if (iter->first.compare(0, root.length(), root) == 0) {
					subkeys.push_back(iter->first);
				}
			}

			return subkeys;
		}

		bool hasKey(std::string key) {
			return keys.find(key) != keys.end();
		}

		void removeKey(std::string key) {
			keys.erase(key);
		}

		std::vector<std::string> keyList(std::string prefix) {
			std::vector<std::string> keyList;

			std::map< std::string, std::vector< std::string > >::const_iterator iter = this->keys.begin();
			for (; iter != this->keys.end(); ++iter) {
				if (iter->first.length() <= prefix.length()) {
					continue;
				}
				if (iter->first.compare(0, prefix.length(), prefix) == 0) {
					std::string strippedKey = iter->first.substr(0, prefix.length());
					keyList.push_back(strippedKey);
				}
			}

			return keyList;
		}

		std::vector<int>* getInts(std::string key) {
			std::map< std::string, std::vector< std::string > >::const_iterator iter;
			iter = keys.find(key);
			if (iter == keys.end()) {
				return 0;
			}

			std::vector< std::string > vs = iter->second;

			std::vector<int>* v = new std::vector<int>(vs.size());
			for (unsigned i = 0; i < vs.size(); i++) {
				int vi = 0;
				if (sscanf(vs.at(i).c_str(), "%d", &vi) != 1) {
					throw std::exception(/*"Failed to parse int."*/);
				}
				v->push_back(vi);
			}

			return v;
		}

		void setInts(std::string key, std::vector<int>* v) {
			std::vector< std::string > s;

			std::vector<int>::const_iterator iter = v->begin();
			for (; iter != v->end(); ++iter) {
				std::stringstream vs;
				vs << *iter;
				s.push_back( vs.str() );
			}

			keys[key] = s;
		}

		std::vector<bool>* getBools(std::string key) {
			std::map< std::string, std::vector< std::string > >::const_iterator iter;
			iter = keys.find(key);
			if (iter == keys.end()) {
				return 0;
			}

			std::vector< std::string > vs = iter->second;

			std::vector<bool>* v = new std::vector<bool>(vs.size());
			for (unsigned i = 0; i < vs.size(); i++) {
				// FIXME: Expand to more than just lowercase true and false.
				if (vs.at(i).compare("true")) {
					v->push_back(true);
				} else if (vs.at(i).compare("false")) {
					v->push_back(false);
				} else {
					throw std::exception(/*"Failed to parse bool."*/);
				}
			}

			return v;
		}

		void setBools(std::string key, std::vector<bool>* v) {
			std::vector< std::string > s;

			std::vector<bool>::const_iterator iter = v->begin();
			for (; iter != v->end(); ++iter) {
				if (*iter) {
					s.push_back( "true" );
				} else {
					s.push_back( "false" );
				}
			}

			keys[key] = s;
		}

		std::vector<std::string>* getStrings(std::string key) {
			std::map< std::string, std::vector< std::string > >::const_iterator iter;
			iter = keys.find(key);
			if (iter == keys.end()) {
				return 0;
			}

			std::vector< std::string > vs = iter->second;

			std::vector< std::string >* v = new std::vector< std::string >(vs.size());
			for (unsigned i = 0; i < vs.size(); i++) {
				v->push_back(vs.at(i));
			}

			return v;
		}

		void setStrings(std::string key, std::vector<std::string>* v) {
			keys[key] = *v;
		}

		std::vector<double>* getDoubles(std::string key) {
			std::map< std::string, std::vector< std::string > >::const_iterator iter;
			iter = keys.find(key);
			if (iter == keys.end()) {
				return 0;
			}

			std::vector< std::string > vs = iter->second;

			std::vector<double>* v = new std::vector<double>(vs.size());
			for (unsigned i = 0; i < vs.size(); i++) {
				double vi = 0;
				if (sscanf(vs.at(i).c_str(), "%f", &vi) != 1) {
					throw std::exception(/*"Failed to parse double."*/);
				}
				v->push_back(vi);
			}

			return v;
		}

		void setDoubles(std::string key, std::vector<double>* v) {
			std::vector< std::string > s;

			std::vector<double>::const_iterator iter = v->begin();
			for (; iter != v->end(); ++iter) {
				std::stringstream vs;
				vs << *iter;
				s.push_back( vs.str() );
			}

			keys[key] = s;
		}

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

		void save(std::string path) {
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
		}

	};
}

#endif // CONFIG_FILE_H
