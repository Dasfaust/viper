#pragma once
#include "../Defines.hpp"
#include "../log/Logger.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>

namespace json
{
#define mkjsint(a) { json::JSTYPE::INT, a }
#define mkjsfloat(a) { json::JSTYPE::FLOAT, 0, a }
#define mkjsbool(a) { json::JSTYPE::BOOL, 0, 0.0f, a }
#define mkjsstring(a) { json::JSTYPE::STRING, 0, 0.0f, false, a }

#define addjsfield(a, b, c) a.fields[b] = c
#define addjsfieldarr(a, b, c) a.aFields[b].push_back(c)
#define addjsobject(a, b, c) a.objects[b] = c
#define addjsobjectarr(a, b, c) a.aObjects[b].push_back(c)

#define mkjsobj(a, b) json::jsobj a; b

#define getjsint(a, b) a.fields[b].ival
#define getjsuint(a, b) a.fields[b].uival
#define getjsdouble(a, b) a.fields[b].dval
#define getjsfloat(a, b) a.fields[b].fval
#define getjsbool(a, b) a.fields[b].bval
#define getjsstring(a, b) a.fields[b].sval
#define getjsobj(a, b) a.objects[b]

#define getjsfieldarr(a, b) a.aFields[b]
#define getjsobjectarr(a, b) a.aObjects[b]

#define doesfieldexist(a, b) (a.fields.count(b) || a.aFields.count(b))
#define doesobjectexist(a, b) (a.objects.count(b) || a.aObjects.count(b))

	enum JSTYPE
	{
		EMPTY = 0,
		INT = 1,
		FLOAT = 2,
		BOOL = 3,
		STRING = 4
	};

	struct jsval
	{
		uint32 type;
		int ival;
		float fval;
		bool bval;
		std::string sval;
	};

	struct jsobj
	{
		flatmap(std::string, jsval) fields;
		flatmap(std::string, std::vector<jsval>) aFields;
		flatmap(std::string, jsobj) objects;
		flatmap(std::string, std::vector<jsobj>) aObjects;
	};

	inline std::vector<std::string> splitStr(std::string string, char delimiter)
	{
		std::vector<std::string> tokens;
		std::string token;
		std::istringstream tokenStream(string);
		while (std::getline(tokenStream, token, delimiter))
		{
			tokens.push_back(token);
		}
		return tokens;
	};

	inline std::string valToStr(const jsval& val)
	{
		std::string out;
		switch(val.type)
		{
		case JSTYPE::INT:
			out = std::to_string(val.ival);
			break;
		case JSTYPE::FLOAT:
			out = std::to_string(val.fval);
			break;
		case JSTYPE::BOOL:
			out = val.bval ? "true" : "false";
			break;
		case JSTYPE::STRING:
			out = "\"" + val.sval + "\"";
			break;
		default:
			break;
		}
		return out;
	};

	inline std::string stringify(const jsobj& obj, int recursion = 0)
	{
		//debug("Serializing");

		std::string js;
		js.reserve(4096);

		std::string tabs;
		for (int i = 0; i < recursion; i++)
		{
			tabs += '\t';
		}

		js += tabs + "{" + '\n';

		for (auto it = obj.fields.begin(); it != obj.fields.end();)
		{
			auto key = it->first;
			auto val = it->second;

			js += tabs + "\"" + key + "\":" + '\n' + tabs + valToStr(val);

			bool end = ++it == obj.fields.end();
			if (!end)
			{
				js += ",\n";
			}
			else if (end && (obj.aFields.size() > 0 || obj.objects.size() > 0 || obj.aObjects.size() > 0))
			{
				js += ",\n";
			}
		}

		for (auto it = obj.aFields.begin(); it != obj.aFields.end();)
		{
			auto key = it->first;
			auto arr = it->second;

			js += tabs + "\"" + key + "\":" + '\n';
			js += tabs + "[" + '\n';
			for (int i = 0; i < arr.size(); i++)
			{
				js += tabs + valToStr(arr[i]);

				if (i != arr.size() - 1)
				{
					js += ",\n";
				}
			}
			js += '\n' + tabs + "]";

			bool end = ++it == obj.aFields.end();
			if (!end)
			{
				js += ",\n";
			}
			else if (end && (obj.objects.size() > 0 || obj.aObjects.size()))
			{
				js += ",\n";
			}
		}

		for (auto it = obj.objects.begin(); it != obj.objects.end();)
		{
			auto key = it->first;
			auto val = it->second;

			js += tabs + "\"" + key + "\":" + '\n' + stringify(val, recursion + 1);

			bool end = ++it == obj.objects.end();
			if (!end)
			{
				js += ",\n";
			}
			else if (end && (!obj.aObjects.empty()))
			{
				js += ",\n";
			}
		}

		for (auto it = obj.aObjects.begin(); it != obj.aObjects.end();)
		{
			auto key = it->first;
			auto arr = it->second;

			js += tabs + "\"" + key + "\":" + '\n';
			js += tabs + "[" + '\n';
			for (uint32 i = 0; i < arr.size(); i++)
			{
				js += stringify(arr[i], recursion + 1);

				if (i != arr.size() - 1)
				{
					js += ",\n";
				}
			}
			js += '\n' + tabs + "]";

			if (++it != obj.aObjects.end())
			{
				js += ",\n";
			}
		}

		js += '\n' + tabs + "}";

		return js;
	};

	inline umap(std::string, std::string) tokenize(std::string js)
	{
		//debug("Tokenizing");

		auto split = splitStr(js, '\n');

		std::string currentMember;
		std::string currentData;
		umap(std::string, std::string) kvs;

		for (uint32 i = 0; i < split.size(); i++)
		{
			//debug(split[i]);
			if (boost::ends_with(split[i], ":") && !boost::starts_with(split[i], "\t"))
			{
				if (currentMember.length() != 0 && currentData.length() != 0)
				{
					kvs[currentMember] = currentData;
				}

				currentMember = boost::trim_copy(boost::replace_all_copy(boost::replace_all_copy(split[i], ":", ""), "\"", ""));
				currentData = std::string();
				continue;
			}

			if (i == split.size() - 1)
			{
				if (currentMember.length() != 0 && currentData.length() != 0)
				{
					kvs[currentMember] = currentData;
				}
			}
			else
			{
				auto sub = split[i];
				currentData += boost::starts_with(split[i], "\t") ? sub.erase(0, 1) : split[i];
				if ((boost::starts_with(split[i], "\t") || boost::ends_with(split[i], ",") || !boost::ends_with(split[i], ",")) && i + 1 < split.size() - 1 && !boost::ends_with(split[i], "\n"))
				{
					currentData += "\n";
				}
			}
		}

		return kvs;
	};

	inline void trimString(std::string& str)
	{
		boost::replace_first(str, "\"", "");
		boost::replace_last(str, "\"", "");
		boost::replace_last(str, ",", "");
	};

	inline bool isNumber(std::string string)
	{
		return !string.empty() && string.find_first_not_of("0123456789.-") == std::string::npos;
	};

	inline jsobj parse(std::string js)
	{
		//debug("Parsing");

		jsobj obj;

		auto kvs = tokenize(js);

		for (auto&& kv : kvs)
		{
			auto data = boost::ends_with(kv.second, "\n") ? boost::replace_last_copy(kv.second, "\n", "") : kv.second;

			debug(kv.first + " -> \n" + data);

			if (data == "true" || data == "true,")
			{
				addjsfield(obj, kv.first, mkjsbool(true));
				continue;
			}
			else if (data == "false" || data == "false,")
			{
				addjsfield(obj, kv.first, mkjsbool(false));
				continue;
			}

			if (boost::starts_with(kv.second, "\""))
			{
				trimString(data);

				addjsfield(obj, kv.first, mkjsstring(data));
				continue;
			}

			if (boost::starts_with(data, "{"))
			{
				addjsobject(obj, kv.first, parse(data));
				continue;
			}

			if (boost::starts_with(data, "["))
			{
				boost::replace_first(data, "[", "");
				boost::replace_last(data, "]", "");

				auto split = splitStr(data, '\n');
				if (split.empty())
				{
					split.push_back(data);
				}

				for (uint32 i = 0; i < split.size(); i++)
				{
					auto s = split[i];

					if (s == "true")
					{
						addjsfieldarr(obj, kv.first, mkjsbool(true));
						continue;
					}
					else if (s == "false")
					{
						addjsfieldarr(obj, kv.first, mkjsbool(false));
						continue;
					}

					if (boost::starts_with(s, "\""))
					{
						trimString(s);
						addjsfieldarr(obj, kv.first, mkjsstring(s));
						continue;
					}

					if (boost::starts_with(s, "{"))
					{
						std::string aobj;
						for (uint32 j = i; j < split.size(); j++)
						{
							aobj += split[j] + '\n';
							if (boost::starts_with(split[j], "\t}"))
							{
								i = j + 1;
								break;
							}
						}
						//debug(aobj);
						addjsobjectarr(obj, kv.first, parse(aobj));
						continue;
					}

					trimString(s);
					if (isNumber(s))
					{
						if (boost::contains(s, "."))
						{
							addjsfieldarr(obj, kv.first, mkjsfloat(boost::numeric_cast<float>(boost::lexical_cast<float>(s))));
						}
						else
						{
							addjsfieldarr(obj, kv.first, mkjsint(boost::numeric_cast<int>(boost::lexical_cast<int>(s))));
						}
						continue;
					}
				}
				continue;
			}

			trimString(data);
			if (isNumber(data))
			{
				if (boost::contains(data, "."))
				{
					addjsfield(obj, kv.first, mkjsfloat(boost::numeric_cast<float>(boost::lexical_cast<float>(data))));
				}
				else
				{
					addjsfield(obj, kv.first, mkjsint(boost::numeric_cast<int>(boost::lexical_cast<int>(data))));
				}
				continue;
			}
		}

		return obj;
	};

	void test()
	{
		time_val start;
		time_val end;

		jsobj ob;
		addjsfield(ob, "string", mkjsstring("test"));
		addjsfield(ob, "int", mkjsint(24));
		addjsfield(ob, "float", mkjsfloat(3.14f));
		addjsfield(ob, "bool", mkjsbool(true));

		addjsfieldarr(ob, "array", mkjsstring("test"));
		addjsfieldarr(ob, "array", mkjsint(24));
		addjsfieldarr(ob, "array", mkjsfloat(3.14f));
		addjsfieldarr(ob, "array", mkjsbool(true));

		addjsobject(ob, "object", jsobj(ob));

		addjsobjectarr(ob, "objectarr", jsobj(ob));

		start = tnowns();
		auto s2 = stringify(ob);
		end = tnowns();

		debug("Serialize: %.4f", (end - start) / 1000000.0);

		//debug(s2);

		start = tnowns();
		auto p = parse(s2);
		end = tnowns();

		debug("Parse: %.4f", (end - start) / 1000000.0);

		start = tnowns();

		end = tnowns();

		debug("RapidJSON parse: %.4f", (end - start) / 1000000.0);

		assert(getjsstring(p, "string") == "test");
		assert(getjsint(p, "int") == 24);
		assert(getjsfloat(p, "float") == 3.14f);
		assert(getjsbool(p, "bool"));

		assert(getjsfieldarr(p, "array")[0].sval == "test");
		assert(getjsfieldarr(p, "array")[1].ival == 24);
		assert(getjsfieldarr(p, "array")[2].fval == 3.14f);
		assert(getjsfieldarr(p, "array")[3].bval);

		assert(p.fields.size() == 4);
		assert(p.aFields.size() == 1);
		assert(p.aFields["array"].size() == 4);

		assert(getjsstring(getjsobj(p, "object"), "string") == "test");
		assert(getjsint(getjsobj(p, "object"), "int") == 24);
		assert(getjsfloat(getjsobj(p, "object"), "float") == 3.14f);
		assert(getjsbool(getjsobj(p, "object"), "bool"));

		assert(p.objects.size() == 1);
		assert(getjsobj(p, "object").fields.size() == 4);
		assert(getjsobj(p, "object").aFields.size() == 1);
		assert(getjsobj(p, "object").aFields["array"].size() == 4);

		assert(getjsfieldarr(getjsobj(p, "object"), "array")[0].sval == "test");
		assert(getjsfieldarr(getjsobj(p, "object"), "array")[1].ival == 24);
		assert(getjsfieldarr(getjsobj(p, "object"), "array")[2].fval == 3.14f);
		assert(getjsfieldarr(getjsobj(p, "object"), "array")[3].bval);

		assert(getjsstring(getjsobjectarr(p, "objectarr")[0], "string") == "test");
		assert(getjsint(getjsobjectarr(p, "objectarr")[0], "int") == 24);
		assert(getjsfloat(getjsobjectarr(p, "objectarr")[0], "float") == 3.14f);
		assert(getjsbool(getjsobjectarr(p, "objectarr")[0], "bool"));

		assert(p.aObjects.size() == 1);
		assert(p.objects.size() == 1);
		assert(getjsobjectarr(p, "objectarr")[0].fields.size() == 4);
		assert(getjsobjectarr(p, "objectarr")[0].aFields.size() == 1);
		assert(getjsobjectarr(p, "objectarr")[0].aFields["array"].size() == 4);

		assert(getjsfieldarr(getjsobjectarr(p, "objectarr")[0], "array")[0].sval == "test");
		assert(getjsfieldarr(getjsobjectarr(p, "objectarr")[0], "array")[1].ival == 24);
		assert(getjsfieldarr(getjsobjectarr(p, "objectarr")[0], "array")[2].fval == 3.14f);
		assert(getjsfieldarr(getjsobjectarr(p, "objectarr")[0], "array")[3].bval);
	};
};