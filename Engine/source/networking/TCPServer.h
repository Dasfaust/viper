#pragma once
#include "../Threadable.h"
#include "../Logger.h"
#include "../util/FileUtils.h"
#include <minwinbase.h>
#include "../util/json.hpp"

class TCPServer : public Threadable
{
public:
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

				std::string msg = "Welcome!";
				send(client, msg.c_str(), msg.size() + 1, 0);
			}
			else
			{
				// Receiving
				char buff[4096];
				ZeroMemory(buff, 4096);

				int bytesIn = recv(sock, buff, 4096, 0);
				if (bytesIn <= 0)
				{
					// Disconnected
					closesocket(sock);
					FD_CLR(sock, &master);
					info("Client disconnected");
				}
				else
				{
					std::string msg(buff);
					if (msg.length() > 1)
					{
						debugf("Client: %s", msg.c_str());
					}
				}
			}
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
