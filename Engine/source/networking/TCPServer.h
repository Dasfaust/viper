#pragma once
#include "../Threadable.h"
#include "../Logger.h"
#include "../util/FileUtils.h"
#include "concurrentqueue.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "boost/container/flat_map.hpp"
#include <iostream>

typedef boost::container::flat_map<std::string, std::vector<boost::any>> packet;

class TCPServer : public Threadable
{
public:
	moodycamel::ConcurrentQueue<packet> outgoing;
	moodycamel::ConcurrentQueue<packet> incoming;
	moodycamel::ConcurrentQueue<int> socketConnected;
	moodycamel::ConcurrentQueue<int> socketDisconnected;

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

		info("Listening on 127.0.0.1:54000");
	};

	void tick() override
	{
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
						rapidjson::Document js;
						try
						{
							js.Parse(reinterpret_cast<const char*>(msg.c_str()));
						}
						catch(const std::exception& ex)
						{
							critf("JSON malformed");
							break;
						}
						js["socket"] = sock;
						incoming.enqueue(js);
					}
				}
			}
		}

		rapidjson::Document js;
		while(outgoing.try_dequeue(js))
		{
			if (js["socket"].GetInt() == -1)
			{
				for (int i = 0; i < master.fd_count; i++)
				{
					SOCKET out = master.fd_array[i];
					if (out != listening)
					{
						rapidjson::StringBuffer buff;
						rapidjson::Writer<rapidjson::StringBuffer> writer(buff);
						js.Accept(writer);
						auto str = buff.GetString();
						//debugf("<- %s", str.c_str());
						send(out, str, buff.GetSize(), 0);
					}
				}
			}
			else
			{
				for (int i = 0; i < master.fd_count; i++)
				{
					SOCKET out = master.fd_array[i];
					if (out == js["socket"].GetInt())
					{
						rapidjson::StringBuffer buff;
						rapidjson::Writer<rapidjson::StringBuffer> writer(buff);
						js.Accept(writer);
						auto str = buff.GetString();
						//debugf("<- %s", str.c_str());
						send(out, str, buff.GetSize(), 0);
						break;
					}
				}
			}
		}

		if (sockets == 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
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
