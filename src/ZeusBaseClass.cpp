#include "ZeusBaseClass.h"

auto LogPrinter = [](const std::string& strLogMsg) { std::cout << strLogMsg << std::endl;  };

void PublicSock::Listen(std::string ip, std::string port)
{	
	for (;;) {
		CTCPServer socket(LogPrinter, ip, port);
		auto newclient = std::make_shared<LocalUser>(socket);
		socket.Listen(newclient->ConnectedClient);
		std::thread t([newclient] { newclient->start(); });
		t.detach();
	}
}

void PublicSock::SSListen(std::string ip, std::string port)
{
	for (;;) {
		CTCPSSLServer socket(LogPrinter, ip, port);
		socket.SetSSLCertFile("server.pem");
		socket.SetSSLKeyFile("server.key");
		auto newclient = std::make_shared<LocalSSLUser>(socket);
		socket.Listen(newclient->ConnectedClient);
		std::thread t([newclient] { newclient->start(); });
		t.detach();
	}
}

void LocalUser::start()
{
	UserSock::Receive();
}

void LocalSSLUser::start()
{
	UserSSLSock::Receive();
}

void UserSock::Receive()
{
	bool quit = false;
	do {
		char buffer[1024] = {};
		CTCPServer::Receive(ConnectedClient, buffer, 1023, false);
		std::string message = buffer;
		std::vector<std::string> str;
		size_t pos;
		while ((pos = message.find("\r\n")) != std::string::npos) {
			str.push_back(message.substr(0, pos));
			message.erase(0, pos + 2);
		} if (str.empty()) {
			while ((pos = message.find("\n")) != std::string::npos) {
				str.push_back(message.substr(0, pos));
				message.erase(0, pos + 1);
			}
		}
		
		for (unsigned int i = 0; i < str.size(); i++)
			if (str[i].length() > 0)
				Send(str[i]);
		if (strcasecmp(buffer, "QUIT") >= 0)
			quit = true;
	} while (quit == false);
	Close();
}

void UserSock::Send(const std::string message)
{
	CTCPServer::Send(ConnectedClient, message + "\r\n");
}

void UserSock::Close()
{
	CTCPServer::Disconnect(ConnectedClient);
}

void UserSSLSock::Receive()
{
	bool quit = false;
	do {
		char buffer[1024] = {};
		CTCPSSLServer::Receive(ConnectedClient, buffer, 1023, false);
		std::string message = buffer;
		std::vector<std::string> str;
		size_t pos;
		while ((pos = message.find("\r\n")) != std::string::npos) {
			str.push_back(message.substr(0, pos));
			message.erase(0, pos + 2);
		} if (str.empty()) {
			while ((pos = message.find("\n")) != std::string::npos) {
				str.push_back(message.substr(0, pos));
				message.erase(0, pos + 1);
			}
		}
		
		for (unsigned int i = 0; i < str.size(); i++)
			if (str[i].length() > 0)
				Send(str[i]);
		if (strcasecmp(buffer, "QUIT") >= 0)
			quit = true;
	} while (quit == false);
	Close();
}

void UserSSLSock::Send(const std::string message)
{
	CTCPSSLServer::Send(ConnectedClient, message + "\r\n");
}

void UserSSLSock::Close()
{
	CTCPSSLServer::Disconnect(ConnectedClient);
}
