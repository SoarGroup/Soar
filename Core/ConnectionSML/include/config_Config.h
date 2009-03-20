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
#include <iostream>
#include <exception>
#include <assert.h>

#include "config_Source.h"

namespace config 
{
	class Config 
	{
	private:
		ConfigSource* source;
		Config* parent;

		std::string prefix; // has a trailing "." as necessary.

	public:
		Config(ConfigSource* source) 
			:source(source)
			,parent(0)
		{
		}

		void save(std::string path)
		{
			source->save(path);
		}
		
		Config* copy() 
		{
			return new Config(this->source->copy());
		}

		Config* getParent() 
		{
			return parent;
		}

		Config* getChild(std::string childprefix) 
		{
			Config* child = new Config(source);
			child->parent = this;

			child->prefix = this->prefix;
			if (child->prefix.length() > 0 && child->prefix.at(child->prefix.length() - 1) != '.')
				child->prefix = child->prefix + ".";
			child->prefix = child->prefix + childprefix + ".";

			return child;
		}

		bool hasKey(std::string key) 
		{
			return source->hasKey(prefix + key);
		}
		
		void removeKey(std::string key) 
		{
			source->removeKey(key);
		}

		std::vector<std::string> keyList() {
			return source->keyList(this->prefix);
		}

		void missingRequired(std::string key) 
		{
			std::string message = "Config: Required key '" + key + "' missing.";
			std::cout << message << std::endl;
			assert (false);
			throw std::exception(/*message.c_str()*/);
		}

		// //////////////////////////
		// int
		std::vector<int>* getInts(std::string key) 
		{
			return getInts(key, 0);
		}

		std::vector<int>* getInts(std::string key, std::vector<int>* defaults) 
		{
			std::vector<int>* v = source->getInts(prefix + key);
			return (v == 0) ? defaults : v;
		}

		std::vector<int>* requireInts(std::string key) 
		{
			std::vector<int>* v = source->getInts(prefix + key);
			if (v == 0)
				missingRequired(prefix + key);
			return v;
		}

		int getInt(std::string key, int def) 
		{
			std::vector<int>* v = source->getInts(prefix + key);
			return (v == 0) ? def : v->at(0);
		}

		int requireInt(std::string key) 
		{
			std::vector<int>* v = source->getInts(prefix + key);
			if (v == 0)
				missingRequired(prefix + key);
			return v->at(0);
		}

		void setInt(std::string key, int v) 
		{
			std::vector<int> va;
			va.push_back(v);
			source->setInts(prefix + key, &va);
		}

		void setInts(std::string key, std::vector<int>* v) 
		{
			source->setInts(prefix + key, v);
		}

		// /////////////////////////
		// std::string
		std::vector<std::string>* getStrings(std::string key) 
		{
			return getStrings(key, 0);
		}

		std::vector<std::string>* getStrings(std::string key, std::vector<std::string>* defaults) 
		{
			std::vector<std::string>* v = source->getStrings(prefix + key);
			return (v == 0) ? defaults : v;
		}

		std::vector<std::string>* requireStrings(std::string key) 
		{
			std::vector<std::string>* v = source->getStrings(prefix + key);
			if (v == 0)
				missingRequired(prefix + key);
			return v;
		}

		std::string getString(std::string key, std::string def) 
		{
			std::vector<std::string>* v = source->getStrings(prefix + key);
			return (v == 0) ? def : v->at(0);
		}

		std::string requireString(std::string key) 
		{
			std::vector<std::string>* v = source->getStrings(prefix + key);
			if (v == 0)
				missingRequired(prefix + key);
			return v->at(0);
		}

		void setString(std::string key, std::string v) 
		{
			std::vector<std::string> va;
			va.push_back(v);
			source->setStrings(prefix + key, &va);
		}

		void setStrings(std::string key, std::vector<std::string>* v) 
		{
			source->setStrings(prefix + key, v);
		}

		// //////////////////////////
		// bool
		std::vector<bool>* getBools(std::string key) 
		{
			return getBools(key, 0);
		}

		std::vector<bool>* getBools(std::string key, std::vector<bool>* defaults) 
		{
			std::vector<bool>* v = source->getBools(prefix + key);
			return (v == 0) ? defaults : v;
		}

		std::vector<bool>* requireBools(std::string key) 
		{
			std::vector<bool>* v = source->getBools(prefix + key);
			if (v == 0)
				missingRequired(prefix + key);
			return v;
		}

		bool getBool(std::string key, bool def) 
		{
			std::vector<bool>* v = source->getBools(prefix + key);
			return (v == 0) ? def : v->at(0);
		}

		bool requireBool(std::string key) 
		{
			std::vector<bool>* v = source->getBools(prefix + key);
			if (v == 0)
				missingRequired(prefix + key);
			return v->at(0);
		}

		void setBool(std::string key, bool v) 
		{
			std::vector<bool> va;
			va.push_back(v);
			source->setBools(prefix + key, &va);
		}

		void setBools(std::string key, std::vector<bool>* v) 
		{
			source->setBools(prefix + key, v);
		}

		// //////////////////////////
		// double
		std::vector<double>* getDoubles(std::string key) 
		{
			return getDoubles(key, 0);
		}

		std::vector<double>* getDoubles(std::string key, std::vector<double>* defaults) 
		{
			std::vector<double>* v = source->getDoubles(prefix + key);
			return (v == 0) ? defaults : v;
		}

		std::vector<double>* requireDoubles(std::string key) 
		{
			std::vector<double>* v = source->getDoubles(prefix + key);
			if (v == 0)
				missingRequired(prefix + key);
			return v;
		}

		double getDouble(std::string key, double def) 
		{
			std::vector<double>* v = source->getDoubles(prefix + key);
			return (v == 0) ? def : v->at(0);
		}

		double requireDouble(std::string key) 
		{
			std::vector<double>* v = source->getDoubles(prefix + key);
			if (v == 0)
				missingRequired(prefix + key);
			return v->at(0);
		}

		void setDouble(std::string key, double v) 
		{
			std::vector<double> va;
			va.push_back(v);
			source->setDoubles(prefix + key, &va);
		}

		void setDoubles(std::string key, std::vector<double>* v) 
		{
			source->setDoubles(prefix + key, v);
		}
	};
}

#endif // CONFIG_CONFIG_H
