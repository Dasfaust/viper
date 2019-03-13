#pragma once
#include "../Macros.h"
#include "../util/FileUtils.h"
#include "../Module.h"
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>
#include "tbb/concurrent_unordered_map.h"
#include "tbb/concurrent_vector.h"

class ConfigLayer : public Module
{
public:
	enum Type
	{
		TYPE_CONFIG,
		TYPE_DATASTORE
	};

	typedef boost::variant<int, float, bool, std::string> Variant;

	V3API ConfigLayer();
	virtual V3API ~ConfigLayer();

	void V3API load(std::string file, Type type);
	tbb::concurrent_vector<int> V3API getInts(std::string section, std::string segment);
	void V3API setInts(std::string section, std::string segment, int val);
	void V3API setInts(std::string section, std::string segment, std::vector<int> ints);
	tbb::concurrent_vector<float> V3API getFloats(std::string section, std::string segment);
	tbb::concurrent_vector<bool> V3API getBools(std::string section, std::string segment);
	tbb::concurrent_vector<std::string> V3API getStrings(std::string section, std::string segment);
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

