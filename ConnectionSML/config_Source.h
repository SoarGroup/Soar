#ifndef CONFIG_SOURCE_H
#define CONFIG_SOURCE_H

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

namespace config 
{
	class ConfigSource 
	{
	public:
		virtual ConfigSource* copy() = 0;
		virtual void save(std::string path) = 0;

		virtual bool hasKey(std::string key) = 0;
		virtual std::vector<std::string> getKeys(std::string root) = 0;
		virtual void removeKey(std::string key) = 0;
		virtual std::vector<std::string> keyList(std::string prefix) = 0;

		virtual std::vector<int>* getInts(std::string key) = 0;
		virtual void setInts(std::string key, std::vector<int>* v) = 0;

		virtual std::vector<std::string>* getStrings(std::string key) = 0;
		virtual void setStrings(std::string key, std::vector<std::string>* v) = 0;

		virtual std::vector<bool>* getBools(std::string key) = 0;
		virtual void setBools(std::string key, std::vector<bool>* v) = 0;

		virtual std::vector<double>* getDoubles(std::string key);
		virtual void setDoubles(std::string key, std::vector<double>* v) = 0;
	};
}

#endif // CONFIG_SOURCE_H
