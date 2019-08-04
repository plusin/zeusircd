#include "ZeusBaseClass.h"
#include "Server.h"
#include <thread>

auto LogPrinter = [](const std::string& strLogMsg) { std::cout << strLogMsg << std::endl;  };

void PublicSock::Listen(std::string ip, std::string port)
{	
	for (;;) {
		CTCPServer socket(LogPrinter, ip, port);
		auto newclient = std::make_shared<PlainUser>(socket);
		socket.Listen(newclient->ConnectedClient);
		if (Server::CanConnect(newclient->IP(newclient->ConnectedClient)) == false) {
			newclient->Close();
			continue;
		}
		Server::ThrottleUP(newclient->IP(newclient->ConnectedClient));
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
		if (Server::CanConnect(newclient->IP(newclient->ConnectedClient)) == false) {
			newclient->Close();
			continue;
		}
		Server::ThrottleUP(newclient->IP(newclient->ConnectedClient));
		std::thread t([newclient] { newclient->start(); });
		t.detach();
	}
}

void PublicSock::WebListen(std::string ip, std::string port)
{
	LocalWebUser newclient(ip, port);
	newclient.run();
}

void PlainUser::start()
{
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
		
		for (unsigned int i = 0; i < str.size(); i++) {
			if (str[i].length() > 0) {
				this->LocalUser::Parse(str[i]);
			}
		}
	} while (quit == false);
	Close();
}

void PlainUser::Send(const std::string message)
{
	CTCPServer::Send(ConnectedClient, message + "\r\n");
}

void PlainUser::Close()
{
	CTCPServer::Disconnect(ConnectedClient);
}

void LocalSSLUser::start()
{
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
		} if (str.empty())
			break;
		
		for (unsigned int i = 0; i < str.size(); i++) {
			if (str[i].length() > 0) {
				this->LocalUser::Parse(str[i]);
			}
		}
	} while (quit == false);
	Close();
}

void LocalSSLUser::Send(const std::string message)
{
	CTCPSSLServer::Send(ConnectedClient, message + "\r\n");
}

void LocalSSLUser::Close()
{
	CTCPSSLServer::Disconnect(ConnectedClient);
}


LocalWebUser::LocalWebUser( std::string ip, std::string port ) : WebSocketServer( ip, port )
{
}

LocalWebUser::~LocalWebUser( )
{
}


void LocalWebUser::onConnect( int socketId )
{
	if (Server::CanConnect(IP(socketId)) == false) {
		Close();
		return;
	} else {
		Server::ThrottleUP(IP(socketId));
		SocketID = socketId;
	}
}

void LocalWebUser::onMessage( int socketID, const string& data )
{
    this->LocalUser::Parse ( data );
}

void LocalWebUser::onDisconnect( int socketID )
{

}

void LocalWebUser::onError( int socketID, const string& message )
{
	std::cout << "WebSocket ERROR: " << message << std::endl; 
}

void LocalWebUser::Send(const std::string message)
{
	this->send( SocketID, message );
}

void LocalWebUser::Close()
{
	WebSocketServer::onDisconnectWrapper(SocketID);
}
