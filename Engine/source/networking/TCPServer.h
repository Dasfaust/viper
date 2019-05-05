#pragma once
#include "../Threadable.h"
#include "../Logger.h"
#include "../util/FileUtils.h"
#include "concurrentqueue.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "../util/Time.h"

namespace js
{
	struct JsonValue
	{
		unsigned int type;
		int int_val;
		double double_val;
		float float_val;
		bool bool_val;
		std::string string_val;
	};

	struct JsonObj
	{
		boost::container::flat_map<std::string, JsonValue> fields;
		boost::container::flat_map<std::string, std::vector<JsonValue>> fieldsArr;
		boost::container::flat_map<std::string, JsonObj> children;
		boost::container::flat_map<std::string, std::vector<JsonObj>> childrenArr;
	};

	std::string valToStr(JsonValue& val)
	{
		if (val.type == 0)
		{
			return std::to_string(val.int_val);
		}
		else if (val.type == 1)
		{
			return std::to_string(val.double_val);
		}
		else if (val.type == 2)
		{
			return std::to_string(val.float_val);
		}
		else if (val.type == 3)
		{
			return std::to_string(val.bool_val);
		}
		else if (val.type == 4)
		{
			return "\"" + val.string_val + "\"";
		}

		return "";
	};

	std::string stringify(JsonObj& obj)
	{
		std::string json;
		json.reserve(4096);

		json += "{";

		for (auto it = obj.fields.begin(); it != obj.fields.end();)
		{
			auto key = it->first;
			auto val = it->second;

			json += "\"" + key + "\":" + valToStr(val);

			bool end = ++it == obj.fields.end();
			if (!end)
			{
				json += ",";
			}
			else if (end && (obj.fieldsArr.size() > 0 || obj.children.size() > 0 || obj.childrenArr.size() > 0))
			{
				json += ",";
			}
		}

		for (auto it = obj.fieldsArr.begin(); it != obj.fieldsArr.end();)
		{
			auto key = it->first;
			auto arr = it->second;

			json += "\"" + key + "\":";
			json += "[";
			for (int i = 0; i < arr.size(); i++)
			{
				json += valToStr(arr[i]);

				if (i != arr.size() - 1)
				{
					json += ",";
				}
			}
			json += "]";

			bool end = ++it == obj.fieldsArr.end();
			if (!end)
			{
				json += ",";
			}
			else if (end && (obj.children.size() > 0 || obj.childrenArr.size()))
			{
				json += ",";
			}
		}

		for (auto it = obj.children.begin(); it != obj.children.end();)
		{
			auto key = it->first;
			auto val = it->second;

			json += "\"" + key + "\":" += stringify(val);

			bool end = ++it == obj.children.end();
			if (!end)
			{
				json += ",";
			}
			else if (end && (obj.childrenArr.size() > 0))
			{
				json += ",";
			}
		}

		for (auto it = obj.childrenArr.begin(); it != obj.childrenArr.end();)
		{
			auto key = it->first;
			auto arr = it->second;

			json += "\"" + key + "\":";
			json += "[";
			for (int i = 0; i < arr.size(); i++)
			{
				json += stringify(arr[i]);

				if (i != arr.size() - 1)
				{
					json += ",";
				}
			}
			json += "]";

			if (++it != obj.childrenArr.end())
			{
				json += ",";
			}
		}

		json += "}";

		return json;
	};

	void set(JsonObj& ob, std::string field, JsonValue val)
	{
		ob.fields[field] = val;
	}

	void set(JsonObj& ob, std::string field, std::vector<JsonValue> val)
	{
		ob.fieldsArr[field] = val;
	}

	void set(JsonObj& ob, std::string field, JsonObj val)
	{
		ob.children[field] = val;
	}

	void set(JsonObj& ob, std::string field, std::vector<JsonObj> val)
	{
		ob.childrenArr[field] = val;
	}

	JsonValue i(int i)
	{
		JsonValue val = { 0 };
		val.int_val = i;
		return val;
	};

	JsonValue d(double i)
	{
		JsonValue val = { 1 };
		val.double_val = i;
		return val;
	};

	JsonValue f(float i)
	{
		JsonValue val = { 2 };
		val.float_val = i;
		return val;
	};

	JsonValue b(bool i)
	{
		JsonValue val = { 3 };
		val.bool_val = i;
		return val;
	};

	JsonValue s(std::string i)
	{
		JsonValue val = { 4 };
		val.string_val = i;
		return val;
	};
}

struct Packet
{
	int socket;
	std::string message;
	int call;
};

class TCPServer : public Threadable
{
public:
	moodycamel::ConcurrentQueue<Packet> outgoing;
	moodycamel::ConcurrentQueue<Packet> incoming;
	moodycamel::ConcurrentQueue<int> socketConnected;
	moodycamel::ConcurrentQueue<int> socketDisconnected;
	double last = 0.0;

	void onStart() override
	{
		WSADATA data;
		int ok = WSAStartup(MAKEWORD(2, 2), &data);
		if (ok != 0)
		{
			critf("Can't start WinSock: error code %d", ok);
		}

		listening = socket(AF_INET, SOCK_STREAM, 0);

		lhint.sin_family = AF_INET;
		lhint.sin_port = htons(54000);
		lhint.sin_addr.S_un.S_addr = INADDR_ANY;
		bind(listening, (sockaddr*)&lhint, sizeof(lhint));

		listen(listening, SOMAXCONN);

		FD_ZERO(&master);
		FD_SET(listening, &master);

		js::JsonObj obj;
		js::JsonObj obj2;
		js::JsonObj obj3;
		js::set(obj3, "dick", js::s("butt"));
		js::set(obj2, "dragon", js::s("butts"));
		js::set(obj, "field", js::d(tnow()));
		js::set(obj, "fieldArr", { js::i(42), js::i(69) });
		js::set(obj, "obj", obj2);
		js::set(obj, "objArr", { obj2, obj3 });
		debug(js::stringify(obj).c_str());

		info("Listening on 127.0.0.1:54000");
	};

	void tick() override
	{
		//auto ts = tnow();
		fd_set copy = master;
		TIMEVAL timeout = { 0, 1000 };
		int sockets = select(0, &copy, nullptr, nullptr, &timeout);

		for (int i = 0; i < sockets; i++)
		{
			SOCKET sock = copy.fd_array[i];

			if (sock == listening)
			{
				// Connecting
				SOCKET client = accept(listening, nullptr, nullptr);

				FD_SET(client, &master);
				info("Initiated client connection");

				socketConnected.enqueue(client);

				//nlohmann::json js = { { "socket", client }, { "call", 0 } };
				//outgoing.enqueue(js);
			}
			else
			{
				// Receiving
				char buff[4096];
				ZeroMemory(buff, 4096);

				int bytesIn = recv(sock, buff, 4096, 0);
				if (bytesIn <= 0)
				{
					socketDisconnected.enqueue(sock);

					// Disconnected
					closesocket(sock);
					FD_CLR(sock, &master);
					info("Client disconnected");
				}
				else
				{
					std::string msg(buff);
					boost::trim(msg);
					if (msg.length() > 1 && msg.c_str()[0] != ' ')
					{
						boost::trim(msg);
						debugf("-> %s", msg.c_str());
						/*rapidjson::Document js;
						try
						{
							js.Parse(reinterpret_cast<const char*>(msg.c_str()));
						}
						catch(const std::exception& ex)
						{
							critf("JSON malformed");
							break;
						}
						js["socket"] = sock;*/
						incoming.enqueue({ (int)sock, msg });
					}
				}
			}
		}

		Packet packet;
		while(outgoing.try_dequeue(packet))
		{
			if (packet.socket == -1)
			{
				for (int i = 0; i < master.fd_count; i++)
				{
					SOCKET out = master.fd_array[i];
					if (out != listening)
					{
						/*rapidjson::StringBuffer buff;
						rapidjson::Writer<rapidjson::StringBuffer> writer(buff);
						js.Accept(writer);
						auto str = buff.GetString();*/
						auto s = tnow();
						//debugf("<- %s", packet.message.c_str());
						send(out, packet.message.c_str(), packet.message.size(), 0);
						//debugf("Packet dispatch took %.2f ms", tnow() - s);

						if (packet.call == 3)
						{
							debugf("Call 3: %.2fms", tnow() - last);
							last = tnow();
						}
					}
				}
			}
			else
			{
				for (int i = 0; i < master.fd_count; i++)
				{
					SOCKET out = master.fd_array[i];
					if (out == packet.socket)
					{
						/*rapidjson::StringBuffer buff;
						rapidjson::Writer<rapidjson::StringBuffer> writer(buff);
						js.Accept(writer);
						auto str = buff.GetString();*/
						//debugf("<- %s", packet.message.c_str());
						send(out, packet.message.c_str(), packet.message.size(), 0);
						break;
					}
				}
			}
		}

		/*if (sockets == 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}*/

		//debugf("TCP tick took %.2f ms", tnow() - ts);
	};

	void onStop() override
	{
		closesocket(listening);
		WSACleanup();
	};
private:
	sockaddr_in lhint;
	SOCKET listening;
	fd_set master;
};
