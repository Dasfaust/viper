#pragma once
#include "../V3Macros.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>
#include "tbb/concurrent_unordered_map.h"
#include "tbb/concurrent_vector.h"

class ConfigLayer
{
public:
	enum Type
	{
		TYPE_CONFIG,
		TYPE_DATASTORE
	};

	typedef boost::variant<int, float, bool, std::string> Variant;

	ConfigLayer(std::string workingDir);
	virtual ~ConfigLayer();

	void load(std::string file, Type type);
	tbb::concurrent_vector<int> getInts(std::string section, std::string segment);
	tbb::concurrent_vector<float> getFloats(std::string section, std::string segment);
	tbb::concurrent_vector<bool> getBools(std::string section, std::string segment);
	tbb::concurrent_vector<std::string> getStrings(std::string section, std::string segment);
private:
	std::string workingDir;
	// filename, (config address: value)[]
	typedef tbb::concurrent_unordered_map<std::string, tbb::concurrent_vector<std::string>> StringSegment;
	typedef tbb::concurrent_unordered_map<std::string, tbb::concurrent_vector<StringSegment>> StringMap;

	typedef tbb::concurrent_unordered_map<std::string, tbb::concurrent_vector<int>> IntSegment;
	typedef tbb::concurrent_unordered_map<std::string, tbb::concurrent_vector<IntSegment>> IntMap;

	typedef tbb::concurrent_unordered_map<std::string, tbb::concurrent_vector<float>> FloatSegment;
	typedef tbb::concurrent_unordered_map<std::string, tbb::concurrent_vector<FloatSegment>> FloatMap;

	typedef tbb::concurrent_unordered_map<std::string, tbb::concurrent_vector<bool>> BoolSegment;
	typedef tbb::concurrent_unordered_map<std::string, tbb::concurrent_vector<BoolSegment>> BoolMap;

	std::shared_ptr<StringMap> stringValues;
	std::shared_ptr<IntMap> intValues;
	std::shared_ptr<FloatMap> floatValues;
	std::shared_ptr<BoolMap> boolValues;

	std::vector<std::string> splitString(std::string string, char delimiter);
	bool isNumber(std::string string);
	Variant parse(std::string string);
};

