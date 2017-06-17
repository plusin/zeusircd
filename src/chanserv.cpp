#include "include.h"

using namespace std;

ChanServ *chanserv = new ChanServ();

void ChanServ::ProcesaMensaje(Socket *s, User *u, string mensaje) {
	if (mensaje.length() == 0 || mensaje == "\r\n" || mensaje == "\r" || mensaje == "\n" || mensaje == "||")
		return;
	vector<string> x;
	boost::split(x,mensaje,boost::is_any_of(" "));
	string cmd = x[0];
	mayuscula(cmd);
	
	if (cmd == "HELP") {
		s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :[ /chanserv register|drop|vop|hop|aop|sop|topic|akick|op|deop|halfop|dehalfop|voz|devoz ]" + "\r\n");
		return;
	} else if (cmd == "REGISTER") {
		if (x.size() < 2) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Necesito mas datos. [ /chanserv register #canal ]" + "\r\n");
			return;
		} else if (u->GetReg() == false) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :No te has registrado." + "\r\n");
			return;
		} else if (chanserv->IsRegistered(x[1]) == 1) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El canal ya esta registrado." + "\r\n");
			return;
		} else if (nickserv->IsRegistered(u->GetNick()) == 0) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Necesitas tener el nick registrado para registrar un canal." + "\r\n");
			return;
		} else if (server->HUBExiste() == 0) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El HUB no existe, las BDs estan en modo de solo lectura." + "\r\n");
			return;
		} else if (chan->IsInChan(u, x[1]) == 0) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Necesitas estar dentro del canal para registrarlo." + "\r\n");
			return;
		} else {
			string sql = "INSERT INTO CANALES VALUES ('" + x[1] + "', '" + u->GetNick() + "', '+r', 'El Canal ha sido registrado',  " + boost::to_string(time(0)) + ", " + boost::to_string(time(0)) + ");";
			if (db->SQLiteNoReturn(sql) == false) {
				s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El canal " + x[1] + " no ha sido registrado.\r\n");
				return;
			}
			sql = "DB " + db->GenerateID() + " " + sql;
			db->AlmacenaDB(sql);
			server->SendToAllServers(sql + "||");
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El canal " + x[1] + " ha sido registrado." + "\r\n");
			for (Chan *canal = canales.first(); canal != NULL; canal = canales.next(canal))
				if (boost::iequals(canal->GetNombre(), x[1], loc))
					if (canal != NULL) {
						if (canal->Tiene_Modo('r') == false) {
							canal->Fijar_Modo('r', true);
							chan->PropagarMODE("CHaN!*@*", "", x[1], 'r', 1);
						}
						chan->PropagarMODE("CHaN!*@*", u->GetNick(), x[1], 'o', 1);
					}
			return;
		}
	} else if (cmd == "DROP") {
		if (x.size() < 2) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Necesito mas datos. [ /chanserv drop #canal ]" + "\r\n");
			return;
		} else if (u->GetReg() == false) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :No te has registrado." + "\r\n");
			return;
		} else if (nickserv->IsRegistered(u->GetNick()) == 0) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Tu nick no esta registrado." + "\r\n");
			return;
		} else if (server->HUBExiste() == 0) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El HUB no existe, las BDs estan en modo de solo lectura." + "\r\n");
			return;
		} else if (u->Tiene_Modo('r') == false) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :No te has identificado, para hacer DROP necesitas tener el nick puesto." + "\r\n");
			return;
		} else if (chan->IsInChan(u, x[1]) == 0) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Necesitas estar dentro del canal para hacer DROP." + "\r\n");
			return;
		} else if (chanserv->IsFounder(u->GetNick(), x[1]) == 0) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :No eres el fundador del canal." + "\r\n");
			return;
		} else {
			string sql = "DELETE FROM CANALES WHERE NOMBRE='" + x[1] + "' COLLATE NOCASE;";
			if (db->SQLiteNoReturn(sql) == false) {
				s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El canal " + x[1] + " no ha sido borrado.\r\n");
				return;
			}
			sql = "DB " + db->GenerateID() + " " + sql;
			db->AlmacenaDB(sql);
			server->SendToAllServers(sql + "||");
			sql = "DELETE FROM ACCESS WHERE CANAL='" + x[1] + "' COLLATE NOCASE;";
			db->SQLiteNoReturn(sql);
			sql = "DB " + db->GenerateID() + " " + sql;
			db->AlmacenaDB(sql);
			server->SendToAllServers(sql + "||");
			sql = "DELETE FROM AKICK WHERE CANAL='" + x[1] + "' COLLATE NOCASE;";
			db->SQLiteNoReturn(sql);
			sql = "DB " + db->GenerateID() + " " + sql;
			db->AlmacenaDB(sql);
			server->SendToAllServers(sql + "||");
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El canal " + x[1] + " ha sido borrado.\r\n");
			for (Chan *canal = canales.first(); canal != NULL; canal = canales.next(canal))
				if (boost::iequals(canal->GetNombre(), x[1], loc)) {
					if (canal != NULL) {
						if (canal->Tiene_Modo('r') == true) {
							canal->Fijar_Modo('r', false);
							chan->PropagarMODE("CHaN!*@*", "", x[1], 'r', 0);
						}
						chan->PropagarMODE("CHaN!*@*", u->GetNick(), x[1], 'o', 0);
					}
				}
		}
	} else if (cmd == "VOP" || cmd == "HOP" || cmd == "AOP" || cmd == "SOP") {
		if (x.size() < 3) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Necesito mas datos. [ /chanserv vop|hop|aop|sop #canal add|del|list (nick) ]" + "\r\n");
			return;
		} else if (u->GetReg() == false) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :No te has registrado." + "\r\n");
			return;
		} else if (nickserv->IsRegistered(u->GetNick()) == 0) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Tu nick no esta registrado." + "\r\n");
			return;
		} else if (chanserv->IsRegistered(x[1]) == 0) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El canal no esta registrado." + "\r\n");
			return;
		} else if (server->HUBExiste() == 0) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El HUB no existe, las BDs estan en modo de solo lectura." + "\r\n");
			return;
		} else if (u->Tiene_Modo('r') == false) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :No te has identificado, para manejar las listas de acceso necesitas tener el nick puesto." + "\r\n");
			return;
		} else if (chanserv->Access(u->GetNick(), x[1]) < 4) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :No tienes suficiente acceso." + "\r\n");
			return;
		} else if (nickserv->GetOption("NOACCESS", u->GetNick()) == 1) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El nick tiene activada la opcion NOACCESS." + "\r\n");
			return;
		} else {
			if (boost::iequals(x[2], "ADD")) {
				mayuscula(cmd);
				if (x.size() < 4) {
					s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Necesito mas datos." + "\r\n");
					return;
				} else if (nickserv->IsRegistered(x[3]) == 0) {
					s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El nick no esta registrado." + "\r\n");
					return;
				}
				if (chanserv->Access(x[3], x[1]) != 0) {
					string sql = "UPDATE ACCESS SET ACCESO='" + cmd + "' FROM ACCESS WHERE NOMBRE='" + x[1] + "' COLLATE NOCASE;";
					if (db->SQLiteNoReturn(sql) == false) {
						s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El registro no se ha podido insertar.\r\n");
						return;
					}
					sql = "DB " + db->GenerateID() + " " + sql;
					db->AlmacenaDB(sql);
					server->SendToAllServers(sql + "||");
					s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Se ha cambiado el registro.\r\n");
				} else {
					string sql = "INSERT INTO ACCESS VALUES ('" + x[1] + "', '" + cmd + "', '" + x[3] + "', '" + u->GetNick() + "');";
					if (db->SQLiteNoReturn(sql) == false) {
						s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El registro no se ha podido insertar.\r\n");
						return;
					}
					sql = "DB " + db->GenerateID() + " " + sql;
					db->AlmacenaDB(sql);
					server->SendToAllServers(sql + "||");
					s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Se ha insertado el registro.\r\n");
					chanserv->CheckModes(x[3], x[1]);
				}
				s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Se ha dado " + cmd + " a " + x[3] + "\r\n");
			} else if (boost::iequals(x[2], "DEL")) {
				if (x.size() < 4) {
					s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Necesito mas datos." + "\r\n");
					return;
				}
				if (chanserv->Access(x[3], x[1]) == 0) {
					s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El usuario no tiene acceso.\r\n");
					return;
				}
				string sql = "DELETE FROM ACCESS WHERE USUARIO='" + x[3] + "' AND CANAL='" + x[1] + "' AND ACCESO='" + cmd + "' COLLATE NOCASE;";
				if (db->SQLiteNoReturn(sql) == false) {
					s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El registro no se ha podido borrar.\r\n");
					return;
				}
				sql = "DB " + db->GenerateID() + " " + sql;
				db->AlmacenaDB(sql);
				server->SendToAllServers(sql + "||");
				s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Se ha quitado " + cmd + " a " + x[3] + "\r\n");
			} else if (boost::iequals(x[2], "LIST")) {
				vector <string> usuarios;
				vector <string> who;
				string sql = "SELECT USUARIO FROM ACCESS WHERE CANAL='" + x[1] + "' AND ACCESO='" + cmd + "' COLLATE NOCASE;";
				usuarios = db->SQLiteReturnVector(sql);
				sql = "SELECT ADDED FROM ACCESS WHERE CANAL='" + x[1] + "' AND ACCESO='" + cmd + "' COLLATE NOCASE;";
				who = db->SQLiteReturnVector(sql);
				if (usuarios.size() == 0)
					s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :No hay accesos de " + cmd + ".\r\n");
				for (unsigned int i = 0; i < usuarios.size(); i++) {
					s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :\002" + usuarios[i] + "\002 accesado por " + who[i] + ".\r\n");
				}
			}
			return;
		}
	} else if (cmd == "TOPIC") {
		if (x.size() < 3) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Necesito mas datos. [ /chanserv topic #canal texto ]" + "\r\n");
			return;
		} else if (u->GetReg() == false) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :No te has registrado." + "\r\n");
			return;
		} else if (nickserv->IsRegistered(u->GetNick()) == 0) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Tu nick no esta registrado." + "\r\n");
			return;
		} else if (server->HUBExiste() == 0) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El HUB no existe, las BDs estan en modo de solo lectura." + "\r\n");
			return;
		} else if (u->Tiene_Modo('r') == false) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :No te has identificado, para cambiar el topic necesitas tener el nick puesto." + "\r\n");
			return;
		} else if (chanserv->IsRegistered(x[1]) == 0) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El canal no esta registrado." + "\r\n");
			return;
		} else if (chanserv->Access(u->GetNick(), x[1]) < 3) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :No tienes acceso para cambiar el topic." + "\r\n");
			return;
		} else {
			int pos = 7 + x[1].length();
			string topic = mensaje.substr(pos);
			if (topic.find(";") != std::string::npos || topic.find("'") != std::string::npos || topic.find("\"") != std::string::npos) {
				s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El topic contiene caracteres no validos." + "\r\n");
				return;
			}
			if (topic.length() > 255) {
				s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El topic es demasiado largo." + "\r\n");
				return;
			}
			string sql = "UPDATE CANALES SET TOPIC='" + topic + "' WHERE NOMBRE='" + x[1] + "' COLLATE NOCASE;";
			if (db->SQLiteNoReturn(sql) == false) {
				s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El topic no se ha podido cambiar.\r\n");
				return;
			}
			sql = "DB " + db->GenerateID() + " " + sql;
			db->AlmacenaDB(sql);
			server->SendToAllServers(sql + "||");
			for (UserChan *uc = usuarios.first(); uc != NULL; uc = usuarios.next(uc)) {
				Socket *sock = u->GetSocketByID(uc->GetID());
				if (sock != NULL)
					sock->Write(":CHaN!*@* 332 " + user->GetNickByID(uc->GetID()) + " " + user->GetNickByID(uc->GetID()) + " :" + topic + "\r\n");
			}
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El topic se ha cambiado.\r\n");
			return;
		}
	} else if (cmd == "AKICK") {
		if (x.size() < 3) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Necesito mas datos. [ /chanserv akick #canal add|del|list (motivo) ]" + "\r\n");
			return;
		} else if (u->GetReg() == false) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :No te has registrado." + "\r\n");
			return;
		} else if (nickserv->IsRegistered(u->GetNick()) == 0) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Tu nick no esta registrado." + "\r\n");
			return;
		} else if (server->HUBExiste() == 0) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El HUB no existe, las BDs estan en modo de solo lectura." + "\r\n");
			return;
		} else if (u->Tiene_Modo('r') == false) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :No te has identificado, para manejar los AKICK necesitas tener el nick puesto." + "\r\n");
			return;
		} else if (chanserv->IsRegistered(x[1]) == 0) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El canal no esta registrado." + "\r\n");
			return;
		} else if (chanserv->Access(u->GetNick(), x[1]) < 4) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :No tienes acceso para cambiar los AKICK." + "\r\n");
			return;
		} else {
			if (boost::iequals(x[2], "ADD")) {
				if (x.size() < 5) {
					s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Necesito mas datos." + "\r\n");
					return;
				}
				if (chanserv->IsAKICK(x[3], x[1]) != 0) {
					s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :La mascara ya tiene AKICK.\r\n");
					return;
				} else {
					int posicion = 4 + cmd.length() + x[1].length() + x[2].length() + x[3].length();
					string motivo = mensaje.substr(posicion);
					if (motivo.find(";") != std::string::npos || motivo.find("'") != std::string::npos || motivo.find("\"") != std::string::npos) {
						s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El motivo contiene caracteres no validos." + "\r\n");
						return;
					}
					string sql = "INSERT INTO AKICK VALUES ('" + x[1] + "', '" + x[3] + "', '" + motivo + "', '" + u->GetNick() + "');";
					if (db->SQLiteNoReturn(sql) == false) {
						s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El AKICK no se ha podido insertar.\r\n");
						return;
					}
					sql = "DB " + db->GenerateID() + " " + sql;
					db->AlmacenaDB(sql);
					server->SendToAllServers(sql + "||");
					s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Se ha insertado el AKICK.\r\n");
				}
			} else if (boost::iequals(x[2], "DEL")) {
				if (x.size() < 4) {
					s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Necesito mas datos." + "\r\n");
					return;
				}
				if (chanserv->IsAKICK(x[3], x[1]) == 0) {
					s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El usuario no tiene AKICK.\r\n");
					return;
				}
				string sql = "DELETE FROM AKICK WHERE MASCARA='" + x[3] + "' AND CANAL='" + x[1] + "' COLLATE NOCASE;";
				if (db->SQLiteNoReturn(sql) == false) {
					s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El AKICK no se ha podido borrar.\r\n");
					return;
				}
				sql = "DB " + db->GenerateID() + " " + sql;
				db->AlmacenaDB(sql);
				server->SendToAllServers(sql + "||");
				s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Se ha quitado " + cmd + " a " + x[3] + "\r\n");
			} else if (boost::iequals(x[2], "LIST")) {
				vector <string> akick;
				vector <string> who;
				string sql = "SELECT MASCARA FROM AKICK WHERE CANAL='" + x[1] + "' COLLATE NOCASE;";
				akick = db->SQLiteReturnVector(sql);
				sql = "SELECT ADDED FROM AKICK WHERE CANAL='" + x[1] + "' COLLATE NOCASE;";
				who = db->SQLiteReturnVector(sql);
				if (akick.size() == 0)
					s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :No hay AKICKS en " + x[1] + "\r\n");
				for (unsigned int i = 0; i < akick.size(); i++) {
					s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :\002" + akick[i] + "\002 realizado por " + who[i] + ".\r\n");
				}
			}
			return;
		}
	} else if (cmd == "OP" || cmd == "DEOP" || cmd == "HALFOP" || cmd == "DEHALFOP" || cmd == "VOZ" || cmd == "DEVOZ") {
		mayuscula(cmd);
		if (x.size() < 3) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Necesito mas datos. [ /chanserv op|deop|halfop|dehalfop|voz|devoz #canal nick ]" + "\r\n");
			return;
		} else if (u->GetReg() == false) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :No te has registrado." + "\r\n");
			return;
		} else if (nickserv->IsRegistered(u->GetNick()) == 0) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :Tu nick no esta registrado." + "\r\n");
			return;
		} else if (u->Tiene_Modo('r') == false) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :No te has identificado, para manejar los modos necesitas tener el nick puesto." + "\r\n");
			return;
		} else if (chanserv->IsRegistered(x[1]) == 0) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El canal no esta registrado." + "\r\n");
			return;
		} else if (chanserv->Access(u->GetNick(), x[1]) < 3) {
			s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :No tienes acceso para cambiar los modos." + "\r\n");
			return;
		} else if (u->FindNick(x[2]) == false) {
				s->Write(":" + config->Getvalue("serverName") + " 401 :El Nick no existe." + "\r\n");
				return;
		} else {
			char modo;
			bool action;
			if (cmd == "OP") {
				modo = 'o';
				action = 1;
			} else if (cmd == "DEOP") {
				modo = 'o';
				action = 0;
			} else if (cmd == "HALFOP") {
				modo = 'h';
				action = 1;
			} else if (cmd == "DEHALFOP") {
				modo = 'h';
				action = 0;
			} else if (cmd == "VOZ") {
				modo = 'v';
				action = 1;
			} else if (cmd == "DEVOZ") {
				modo = 'v';
				action = 0;
			} else
				return;

			for (UserChan *uc = usuarios.first(); uc != NULL; uc = usuarios.next(uc))
				if (boost::iequals(uc->GetNombre(), x[1]) && boost::iequals(user->GetNickByID(uc->GetID()), x[2])) {
					string channel = uc->GetNombre();
					string nickname = user->GetNickByID(uc->GetID());
					char mode = uc->GetModo();
					if (mode == 'v') {
						if (modo == 'h' && action == 1) {
							chan->PropagarMODE("CHaN!*@*", nickname, channel, 'v', 0);
							chan->PropagarMODE("CHaN!*@*", nickname, channel, 'h', 1);
							//datos->canales[id]->usuarios[pos]->modo = 'h';
						} else if (modo == 'o' && action == 1) {
							chan->PropagarMODE("CHaN!*@*", nickname, channel, 'v', 0);
							chan->PropagarMODE("CHaN!*@*", nickname, channel, 'o', 1);
							//datos->canales[id]->usuarios[pos]->modo = 'o';
						} else if (modo == 'h' && action == 0) {
							s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El nick no tiene HALFOP." + "\r\n");
						} else if (modo == 'o' && action == 0) {
							s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El nick no tiene OP." + "\r\n");
						} else if (modo == 'v' && action == 1) {
							s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El nick ya tiene VOZ." + "\r\n");
						} else if (modo == 'v' && action == 0) {
							chan->PropagarMODE("CHaN!*@*", nickname, channel, 'v', 0);
							//datos->canales[id]->usuarios[pos]->modo = 'x';
						}
					} else if (mode == 'h') {
						if (modo == 'v' && action == 1) {
							chan->PropagarMODE("CHaN!*@*", nickname, channel, 'h', 0);
							chan->PropagarMODE("CHaN!*@*", nickname, channel, 'v', 1);
							//datos->canales[id]->usuarios[pos]->modo = 'v';
						} else if (modo == 'o' && action == 1) {
							chan->PropagarMODE("CHaN!*@*", nickname, channel, 'h', 0);
							chan->PropagarMODE("CHaN!*@*", nickname, channel, 'o', 1);
							//datos->canales[id]->usuarios[pos]->modo = 'o';
						} else if (modo == 'v' && action == 0) {
							s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El nick no tiene VOZ." + "\r\n");
						} else if (modo == 'o' && action == 0) {
							s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El nick no tiene OP." + "\r\n");
						} else if (modo == 'h' && action == 1) {
							s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El nick ya tiene HALFOP." + "\r\n");
						} else if (modo == 'h' && action == 0) {
							chan->PropagarMODE("CHaN!*@*", nickname, channel, 'h', 0);
							//datos->canales[id]->usuarios[pos]->modo = 'x';
						}
					} else if (mode == 'o') {
						if (modo == 'v' && action == 1) {
							chan->PropagarMODE("CHaN!*@*", nickname, channel, 'o', 0);
							chan->PropagarMODE("CHaN!*@*", nickname, channel, 'v', 1);
							//datos->canales[id]->usuarios[pos]->modo = 'v';
						} else if (modo == 'h' && action == 1) {
							chan->PropagarMODE("CHaN!*@*", nickname, channel, 'o', 0);
							chan->PropagarMODE("CHaN!*@*", nickname, channel, 'h', 1);
							//datos->canales[id]->usuarios[pos]->modo = 'h';
						} else if (modo == 'v' && action == 0) {
							s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El nick no tiene VOZ." + "\r\n");
						} else if (modo == 'h' && action == 0) {
							s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El nick no tiene HALFOP." + "\r\n");
						} else if (modo == 'o' && action == 1) {
							s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El nick ya tiene OP." + "\r\n");
						} else if (modo == 'o' && action == 0) {
							chan->PropagarMODE("CHaN!*@*", nickname, channel, 'o', 0);
							//datos->canales[id]->usuarios[pos]->modo = 'x';
						}
					} else if (mode == 'x') {
						if (modo == 'v' && action == 1) {
							chan->PropagarMODE("CHaN!*@*", nickname, channel, 'v', 1);
							//datos->canales[id]->usuarios[pos]->modo = 'v';
						} else if (modo == 'h' && action == 1) {
							chan->PropagarMODE("CHaN!*@*", nickname, channel, 'h', 1);
							//datos->canales[id]->usuarios[pos]->modo = 'h';
						} else if (modo == 'o' && action == 1) {
							chan->PropagarMODE("CHaN!*@*", nickname, channel, 'o', 1);
							//datos->canales[id]->usuarios[pos]->modo = 'o';
						} else if (action == 0){
							s->Write(":CHaN!*@* NOTICE " + u->GetNick() + " :El nick no tiene modos." + "\r\n");
						}
					}
				}
				return;
		}
	}
}

void ChanServ::CheckModes(string nickname, string channel) {
	for (UserChan *uc = usuarios.first(); uc != NULL; uc = usuarios.next(uc))
		if (boost::iequals(uc->GetNombre(), channel) && boost::iequals(user->GetNickByID(uc->GetID()), nickname)) {
			string channel = uc->GetNombre();
			string nickname = user->GetNickByID(uc->GetID());
			char mode = uc->GetModo();
			int access = chanserv->Access(nickname, channel);
			if (mode == 'v') {
				if (access == 2) {
					chan->PropagarMODE("CHaN!*@*", nickname, channel, 'v', 0);
					chan->PropagarMODE("CHaN!*@*", nickname, channel, 'h', 1);
					//datos->canales[id]->usuarios[pos]->modo = 'h';
				} else if (access > 2) {
					chan->PropagarMODE("CHaN!*@*", nickname, channel, 'v', 0);
					chan->PropagarMODE("CHaN!*@*", nickname, channel, 'o', 1);
					//datos->canales[id]->usuarios[pos]->modo = 'o';
				}
			} else if (mode == 'h') {
				if (access == 1) {
					chan->PropagarMODE("CHaN!*@*", nickname, channel, 'h', 0);
					chan->PropagarMODE("CHaN!*@*", nickname, channel, 'v', 1);
					//datos->canales[id]->usuarios[pos]->modo = 'v';
				} else if (access > 2) {
					chan->PropagarMODE("CHaN!*@*", nickname, channel, 'h', 0);
					chan->PropagarMODE("CHaN!*@*", nickname, channel, 'o', 1);
					//datos->canales[id]->usuarios[pos]->modo = 'o';
				}
			} else if (mode == 'o') {
				if (access == 1) {
					chan->PropagarMODE("CHaN!*@*", nickname, channel, 'o', 0);
					chan->PropagarMODE("CHaN!*@*", nickname, channel, 'v', 1);
					//datos->canales[id]->usuarios[pos]->modo = 'v';
				} else if (access == 2) {
					chan->PropagarMODE("CHaN!*@*", nickname, channel, 'o', 0);
					chan->PropagarMODE("CHaN!*@*", nickname, channel, 'h', 1);
					//datos->canales[id]->usuarios[pos]->modo = 'h';
				}
			} else {
				if (access == 1) {
					chan->PropagarMODE("CHaN!*@*", nickname, channel, 'v', 1);
					//datos->canales[id]->usuarios[pos]->modo = 'v';
				} else if (access == 2) {
					chan->PropagarMODE("CHaN!*@*", nickname, channel, 'h', 1);
					//datos->canales[id]->usuarios[pos]->modo = 'h';
				} else if (access > 2) {
					chan->PropagarMODE("CHaN!*@*", nickname, channel, 'o', 1);
					//datos->canales[id]->usuarios[pos]->modo = 'o';
				}
			}
	}
	return;
}

bool ChanServ::IsRegistered(string channel) {
	string sql = "SELECT NOMBRE from CANALES WHERE NOMBRE='" + channel + "' COLLATE NOCASE;";
	string retorno = db->SQLiteReturnString(sql);
	if (boost::iequals(retorno, channel))
		return true;
	return false;
}

bool ChanServ::IsFounder(string nickname, string channel) {
	string sql = "SELECT OWNER from CANALES WHERE NOMBRE='" + channel + "' COLLATE NOCASE;";
	string retorno = db->SQLiteReturnString(sql);
	if (boost::iequals(retorno, nickname))
		return true;
	else
		return false;
}

int ChanServ::Access (string nickname, string channel) {
	string sql = "SELECT ACCESO from ACCESS WHERE USUARIO='" + nickname + "' AND CANAL='" + channel + "' COLLATE NOCASE;";
	string retorno = db->SQLiteReturnString(sql);
	if (boost::iequals(retorno, "VOP"))
		return 1;
	else if (boost::iequals(retorno, "HOP"))
		return 2;
	else if (boost::iequals(retorno, "AOP"))
		return 3;
	else if (boost::iequals(retorno, "SOP"))
		return 4;
	else if (chanserv->IsFounder(nickname, channel) == 1 || chan->Tiene_Modo('o') == 1)
		return 5;
	return 0;
}

bool ChanServ::IsAKICK(string mascara, string canal) {
	vector <string> akicks;
	string sql = "SELECT MASCARA from AKICK WHERE CANAL='" + canal + "' COLLATE NOCASE;";
	akicks = db->SQLiteReturnVector(sql);
	for (unsigned int i = 0; i < akicks.size(); i++) {
		boost::algorithm::to_lower(akicks[i]);
		boost::algorithm::to_lower(mascara);
		if (user->Match(akicks[i].c_str(), mascara.c_str()) == 1)
			return true;
	}
	return false;
}

int ChanServ::GetChans () {
	string sql = "SELECT COUNT(*) FROM CANALES;";
	return db->SQLiteReturnInt(sql);
}
