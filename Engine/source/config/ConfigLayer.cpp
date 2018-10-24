#include "ConfigLayer.h"
#include <fstream>

ConfigLayer::ConfigLayer(std::string workingDir, std::shared_ptr<EventLayer> events)
{
	infof("ConfigLayer: %s", workingDir.c_str());
	this->workingDir = workingDir;
	this->events = events;
	stringValues = std::make_shared<StringMap>();
	intValues = std::make_shared<IntMap>();
	floatValues = std::make_shared<FloatMap>();
	boolValues = std::make_shared<BoolMap>();

	load("engine", TYPE_CONFIG);
}

ConfigLayer::~ConfigLayer()
{
	info("ConfigLayer destroyed");
}

std::vector<std::string> ConfigLayer::splitString(std::string string, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(string);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}

bool ConfigLayer::isNumber(std::string string)
{
	return !string.empty() && string.find_first_not_of("0123456789.-") == std::string::npos;
}

ConfigLayer::Variant ConfigLayer::parse(std::string string)
{
	Variant var;

	boost::replace_all(string, "\t", "");
	boost::replace_all(string, "\n", "");
	boost::replace_all(string, " ", "");

	//debugf("Config: parsing serialized value: [%s]", string.c_str());

	try
	{
		if (!string.compare("true") || !string.compare("false"))
		{
			!string.compare("true") ? var = true : var = false;
			//debugf(" -> <bool> %d", boost::get<bool>(var));
		}
		else if (isNumber(string))
		{
			if (string.find(".") != std::string::npos)
			{
				float f = boost::numeric_cast<float>(boost::lexical_cast<float>(string));
				var = f;
				//debugf(" -> <float> %f", boost::get<float>(var));
			}
			else
			{
				int i = boost::numeric_cast<int>(boost::lexical_cast<int>(string));
				var = i;
				//debugf(" -> <int> %i", boost::get<int>(var));
			}
		}
		else
		{
			var = string;
			//debugf(" -> <string> %s", boost::get<std::string>(var).c_str());
		}
	}
	catch (std::exception &ex)
	{
		criticalf("Configuration parse error: %s", ex.what())
	}

	return var;
}

void ConfigLayer::load(std::string file, Type type)
{
	std::string path;
	if (type == TYPE_CONFIG)
	{
		path = workingDir + "/config/" + file + ".conf";
	}
	else
	{
		path = file + ".save.conf";
	}

	infof("Loading .conf file: %s", path.c_str());
	std::ifstream doc(path);

	if (!doc.good()) throw std::runtime_error("Configuration file doesn't exist: " + path);

	std::stringstream buffer;
	std::string ln;
	while (getline(doc, ln))
	{
		buffer << ln << "\n";
	}
	doc.close();

	try
	{
		std::istringstream iss(buffer.str());

		std::string multi;
		for (std::string line; std::getline(iss, line); )
		{
			if (multi.size() > 0 || line.find("[") != std::string::npos)
			{
				if (line.find("]") != std::string::npos)
				{
					if (multi.size() == 0)
					{
						multi = line;
					}
					else
					{
						multi += line;
					}

					std::vector<std::string> split = splitString(multi, '=');

					std::string name = split[0];
					boost::replace_all(name, "[", "");

					std::string value;
					for (int i = 1; i < split.size(); i++)
					{
						value += split[i];
						if (i + 1 < split.size())
						{
							value += "=";
						}
					}
					boost::replace_all(value, "]", "");

					std::vector<std::string> vals;
					if (!boost::starts_with(value, "\"") && value.find(",") != std::string::npos)
					{
						std::vector<std::string> vsplit = splitString(value, ',');
						for (std::string str : vsplit)
						{
							vals.push_back(str);
						}
					}
					else
					{
						vals.push_back(value);
					}

					if (vals.size() > 0)
					{
						std::vector<Variant> vars;
						for (std::string val : vals)
						{
							boost::trim(val);
							boost::replace_all(val, "\t", "");
							boost::replace_all(val, "\n", "");
							if (val.size() > 0)
							{
								vars.push_back(parse(val));
							}
						}

						if (vars[0].which() == 0)
						{
							IntSegment segment;
							tbb::concurrent_vector<int> list;
							for (auto v : vars)
							{
								list.push_back(boost::get<int>(v));
							}
							segment[name] = list;
							(*intValues)[file].push_back(segment);
						}
						else if (vars[0].which() == 1)
						{
							FloatSegment segment;
							tbb::concurrent_vector<float> list;
							for (auto v : vars)
							{
								list.push_back(boost::get<float>(v));
							}
							segment[name] = list;
							(*floatValues)[file].push_back(segment);
						}
						else if (vars[0].which() == 2)
						{
							BoolSegment segment;
							tbb::concurrent_vector<bool> list;
							for (auto v : vars)
							{
								list.push_back(boost::get<bool>(v));
							}
							segment[name] = list;
							(*boolValues)[file].push_back(segment);
						}
						else if (vars[0].which() == 3)
						{
							StringSegment segment;
							tbb::concurrent_vector<std::string> list;
							for (auto v : vars)
							{
								list.push_back(boost::get<std::string>(v));
							}
							segment[name] = list;
							(*stringValues)[file].push_back(segment);
						}
					}

					multi = "";
				}
				else
				{
					multi += line;
				}
			}
		}
	}
	catch (const std::exception &ex)
	{
		criticalf("Configuration format error: %s", ex.what())
	}
}

tbb::concurrent_vector<int> ConfigLayer::getInts(std::string section, std::string segment)
{
	for (auto kv : (*intValues)[section])
	{
		if (kv.count(segment))
		{
			return kv[segment];
		}
	}

	throw std::runtime_error("Config definition doesn't exist: " + section + "." + segment);
}

void ConfigLayer::setInts(std::string section, std::string segment, int val)
{
	std::vector<int> ints;
	ints.push_back(val);
	setInts(section, segment, ints);
}

void ConfigLayer::setInts(std::string section, std::string segment, std::vector<int> ints)
{
	std::shared_ptr<Event::OnConfigChangedData> data = std::make_shared<Event::OnConfigChangedData>();
	data->section = section;
	data->segment = segment;
	std::vector<Variant> vars;
	for (int i : ints)
	{
		Variant var = i;
		vars.push_back(var);
	}
	data->values = vars;
	events.lock()->getOnConfigChanged()->triggerEvent(data);
}

tbb::concurrent_vector<float> ConfigLayer::getFloats(std::string section, std::string segment)
{
	for (auto kv : (*floatValues)[section])
	{
		if (kv.count(segment))
		{
			return kv[segment];
		}
	}

	throw std::runtime_error("Config definition doesn't exist: " + section + "." + segment);
}

tbb::concurrent_vector<bool> ConfigLayer::getBools(std::string section, std::string segment)
{
	for (auto kv : (*boolValues)[section])
	{
		if (kv.count(segment))
		{
			return kv[segment];
		}
	}

	throw std::runtime_error("Config definition doesn't exist: " + section + "." + segment);
}

tbb::concurrent_vector<std::string> ConfigLayer::getStrings(std::string section, std::string segment)
{
	for (auto kv : (*stringValues)[section])
	{
		if (kv.count(segment))
		{
			return kv[segment];
		}
	}

	throw std::runtime_error("Config definition doesn't exist: " + section + "." + segment);
}
