#include "include.h"
#include <cstdlib>

time_t encendido = time(0);
Semaforo semaforo;

int main(int argc, char *argv[]) {

	config->Cargar();

	if (access("zeus.db", W_OK) != 0)
		db->IniciarDB();
	
	srand(time(0));
	
	datos->AddServer(NULL, config->Getvalue("serverName"), config->Getvalue("listen[0]ip"), 0);

	std::cout << "Mi Nombre es: " << config->Getvalue("serverName") << std::endl;
	
	if (ulimit(UL_SETFSIZE, MAX_USERS) < 0) {
		std::cout << "ULIMIT ERROR" << std::endl;
		exit(1);
	} else
		std::cout << "Limite de usuarios configurado a: " << MAX_USERS << std::endl;
	
	for (unsigned int i = 0; config->Getvalue("listen["+to_string(i)+"]ip").length() > 0; i++) {
		if (config->Getvalue("listen["+to_string(i)+"]class") == "client") {
			Socket *principal = new Socket();
			principal->ip = (char *) config->Getvalue("listen["+to_string(i)+"]ip").c_str();
			principal->port = (int) stoi(config->Getvalue("listen["+to_string(i)+"]port"));
			if (config->Getvalue("listen["+to_string(i)+"]ssl") == "1" || config->Getvalue("listen["+to_string(i)+"]ssl") == "true")
				principal->SSL = 1;
			else
				principal->SSL = 0;
			principal->IPv6 = 0;
			principal->tw = principal->MainThread();
			principal->tw.detach();
		} else if (config->Getvalue("listen["+to_string(i)+"]class") == "server") {
			Socket *servidores = new Socket();
			servidores->ip = (char *) config->Getvalue("listen["+to_string(i)+"]ip").c_str();
			servidores->port = (int) stoi(config->Getvalue("listen["+to_string(i)+"]port"));
			if (config->Getvalue("listen["+to_string(i)+"]ssl") == "1" || config->Getvalue("listen["+to_string(i)+"]ssl") == "true")
				servidores->SSL = 1;
			else
				servidores->SSL = 0;
			servidores->IPv6 = 0;
			servidores->tw = servidores->ServerThread();
			servidores->tw.detach();
			if (server->Existe(config->Getvalue("listen["+to_string(i)+"]ip")) == 0)
				datos->AddServer(NULL, config->Getvalue("serverName"), config->Getvalue("listen["+to_string(i)+"]ip"), 0);
		}
	}
	for (unsigned int i = 0; config->Getvalue("listen6["+to_string(i)+"]ip").length() > 0; i++) {
		if (config->Getvalue("listen6["+to_string(i)+"]class") == "client") {
			Socket *principal = new Socket();
			principal->ip = (char *) config->Getvalue("listen6["+to_string(i)+"]ip").c_str();
			principal->port = (int) stoi(config->Getvalue("listen6["+to_string(i)+"]port"));
			if (config->Getvalue("listen6["+to_string(i)+"]ssl") == "1" || config->Getvalue("listen6["+to_string(i)+"]ssl") == "true")
				principal->SSL = 1;
			else
				principal->SSL = 0;
			principal->IPv6 = 1;
			principal->tw = principal->MainThread();
			principal->tw.detach();
		} else if (config->Getvalue("listen6["+to_string(i)+"]class") == "server") {
			Socket *servidores = new Socket();
			servidores->ip = (char *) config->Getvalue("listen6["+to_string(i)+"]ip").c_str();
			servidores->port = (int) stoi(config->Getvalue("listen6["+to_string(i)+"]port"));
			if (config->Getvalue("listen6["+to_string(i)+"]ssl") == "1" || config->Getvalue("listen6["+to_string(i)+"]ssl") == "true")
				servidores->SSL = 1;
			else
				servidores->SSL = 0;
			servidores->IPv6 = 1;
			servidores->tw = servidores->ServerThread();
			servidores->tw.detach();
		}
	}
	std::cout << "Zeus iniciado ... OK" << std::endl;
	while (1) {
  		semaforo.wait();
		procesacola ();
	}
	return 0;
}
