/* 
 * This file is part of the ZeusiRCd distribution (https://github.com/Pryancito/zeusircd).
 * Copyright (c) 2019 Rodrigo Santidrian AKA Pryan.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "ZeusiRCd.h"
#include "module.h"
#include "db.h"
#include "Server.h"
#include "oper.h"
#include "Utils.h"
#include "services.h"
#include "base64.h"
#include "Config.h"
#include "sha256.h"

using namespace std;

class CMD_CS : public Module
{
	public:
	CMD_CS() : Module("CS", 50, false) {};
	~CMD_CS() {};
	virtual void command(User *user, std::string message) override {
		message=message.substr(message.find_first_of(" \t")+1);
		
		std::vector<std::string> split;
		Utils::split(message, split, " ");
		
		if (split.size() == 0)
			return;

		std::string cmd = split[0];
		std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
		
		if (cmd == "HELP") {
			if (split.size() == 1) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :[ /chanserv register|drop|vop|hop|aop|sop|topic|key|akick|op|deop|halfop|dehalfop|voice|devoice|transfer ]");
				return;
			} else if (split.size() > 1) {
				std::string comando = split[1];
				std::transform(comando.begin(), comando.end(), comando.begin(), ::toupper);
				if (comando == "REGISTER") {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :[ /chanserv register <#channel> ]");
					return;
				} else if (comando == "DROP") {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :[ /chanserv drop <#channel> ]");
					return;
				} else if (comando == "VOP") {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :[ /chanserv vop #channel <add|del|list> [nick] ]");
					return;
				} else if (comando == "HOP") {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :[ /chanserv hop #channel <add|del|list> [nick] ]");
					return;
				} else if (comando == "AOP") {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :[ /chanserv aop #channel <add|del|list> [nick] ]");
					return;
				} else if (comando == "SOP") {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :[ /chanserv sop #channel <add|del|list> [nick] ]");
					return;
				} else if (comando == "TOPIC") {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :[ /chanserv topic #channel <topic> ]");
					return;
				} else if (comando == "KEY") {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :[ /chanserv key #channel <key|off> ]");
					return;
				} else if (comando == "AKICK") {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :[ /chanserv akick #channel <add|del|list> [mask] [reason] ]");
					return;
				} else if (comando == "OP") {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :[ /chanserv op #channel <nick> ]");
					return;
				} else if (comando == "DEOP") {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :[ /chanserv deop #channel <nick> ]");
					return;
				} else if (comando == "HALFOP") {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :[ /chanserv halfop #channel <nick> ]");
					return;
				} else if (comando == "DEHALFOP") {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :[ /chanserv dehalfop #channel <nick> ]");
					return;
				} else if (comando == "VOICE") {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :[ /chanserv voice #channel <nick> ]");
					return;
				} else if (comando == "DEVOICE") {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :[ /chanserv devoice #channel <nick> ]");
					return;
				} else if (comando == "TRANSFER") {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :[ /chanserv transfer #channel <nick> ]");
					return;
				} else {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "There is no help for that command."));
					return;
				}
			}
		} else if (cmd == "REGISTER") {
			if (split.size() < 2) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "More data is needed."));
				return;
			} else if (ChanServ::IsRegistered(split[1]) == true) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The channel %s is already registered.", split[1].c_str()));
				return;
			} else if (user->getMode('r') == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "To make this action, you need identify first."));
				return;
			} else if (Server::HUBExiste() == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The HUB doesnt exists, DBs are in read-only mode."));
				return;
			} else if (ChanServ::CanRegister(user, split[1]) == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "You need to be into the channel and got @ to make %s.", "REGISTER"));
				return;
			} else {
				string topic = Utils::make_string("", "The channel has been registered.");
				string sql = "INSERT INTO CANALES VALUES ('" + split[1] + "', '" + user->mNickName + "', '+r', '', '" + encode_base64((const unsigned char*)topic.c_str(), topic.length()) + "',  " + std::to_string(time(0)) + ", " + std::to_string(time(0)) + ");";
				if (DB::SQLiteNoReturn(sql) == false) {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The channel %s cannot be registered. Please contact with an iRCop.", split[1].c_str()));
					return;
				}
				if (config["database"]["cluster"].as<bool>() == false) {
					sql = "DB " + DB::GenerateID() + " " + sql;
					DB::AlmacenaDB(sql);
					Server::Send(sql);
				}
				sql = "INSERT INTO CMODES (CANAL) VALUES ('" + split[1] + "');";
				if (DB::SQLiteNoReturn(sql) == false) {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The channel %s cannot be registered. Please contact with an iRCop.", split[1].c_str()));
					return;
				}
				if (config["database"]["cluster"].as<bool>() == false) {
					sql = "DB " + DB::GenerateID() + " " + sql;
					DB::AlmacenaDB(sql);
					Server::Send(sql);
				}
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string("", "The channel %s has been registered.", split[1].c_str()));
				Channel* chan = Channel::GetChannel(split[1]);
				if (chan) {
					if (chan->getMode('r') == false) {
						chan->setMode('r', true);
						chan->broadcast(":" + config["chanserv"].as<std::string>() + " MODE " + chan->name + " +r");
						Server::Send("CMODE " + config["chanserv"].as<std::string>() + " " + chan->name + " +r");
					}
				}
				return;
			}
		} else if (cmd == "DROP") {
			if (split.size() < 2) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "More data is needed."));
				return;
			} else if (Server::HUBExiste() == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The HUB doesnt exists, DBs are in read-only mode."));
				return;
			} else if (user->getMode('r') == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "To make this action, you need identify first."));
				return;
			} else if (ChanServ::CanRegister(user, split[1]) == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "You need to be into the channel and got @ to make %s.", "DROP"));
				return;
			} else if (ChanServ::IsFounder(user->mNickName, split[1]) == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "You are not the founder of the channel."));
				return;
			} else {
				string sql = "DELETE FROM CANALES WHERE NOMBRE='" + split[1] + "';";
				if (DB::SQLiteNoReturn(sql) == false) {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The channel %s cannot be deleted. Please contact with an iRCop.", split[1].c_str()));
					return;
				}
				if (config["database"]["cluster"].as<bool>() == false) {
					sql = "DB " + DB::GenerateID() + " " + sql;
					DB::AlmacenaDB(sql);
					Server::Send(sql);
				}
				sql = "DELETE FROM ACCESS WHERE CANAL='" + split[1] + "';";
				if (DB::SQLiteNoReturn(sql) == false) {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The channel %s cannot be deleted. Please contact with an iRCop.", split[1].c_str()));
					return;
				}
				if (config["database"]["cluster"].as<bool>() == false) {
					sql = "DB " + DB::GenerateID() + " " + sql;
					DB::AlmacenaDB(sql);
					Server::Send(sql);
				}
				sql = "DELETE FROM AKICK WHERE CANAL='" + split[1] + "';";
				if (DB::SQLiteNoReturn(sql) == false) {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The channel %s cannot be deleted. Please contact with an iRCop.", split[1].c_str()));
					return;
				}
				if (config["database"]["cluster"].as<bool>() == false) {
					sql = "DB " + DB::GenerateID() + " " + sql;
					DB::AlmacenaDB(sql);
					Server::Send(sql);
				}
				sql = "DELETE FROM CMODES WHERE CANAL='" + split[1] + "';";
				if (DB::SQLiteNoReturn(sql) == false) {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The channel %s cannot be deleted. Please contact with an iRCop.", split[1].c_str()));
					return;
				}
				if (config["database"]["cluster"].as<bool>() == false) {
					sql = "DB " + DB::GenerateID() + " " + sql;
					DB::AlmacenaDB(sql);
					Server::Send(sql);
				}
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The channel %s has been deleted.", split[1].c_str()));
				Channel* chan = Channel::GetChannel(split[1]);
				if (chan) {
					if (chan->getMode('r') == true) {
						chan->setMode('r', false);
						chan->broadcast(":" + config["chanserv"].as<std::string>() + " MODE " + chan->name + " -r");
						Server::Send("CMODE " + config["chanserv"].as<std::string>() + " " + chan->name + " -r");
					}
				}
			}
		} else if (cmd == "VOP" || cmd == "HOP" || cmd == "AOP" || cmd == "SOP") {
			if (split.size() < 3) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "More data is needed."));
				return;
			} else if (ChanServ::IsRegistered(split[1]) == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The channel %s is not registered.", split[1].c_str()));
				return;
			} else if (Server::HUBExiste() == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The HUB doesnt exists, DBs are in read-only mode."));
				return;
			} else if (user->getMode('r') == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "To make this action, you need identify first."));
				return;
			} else if (ChanServ::Access(user->mNickName, split[1]) < 4) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "You do not have enough access."));
				return;
			} else {
				if (strcasecmp(split[2].c_str(), "ADD") == 0) {
					if (split.size() < 4) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "More data is needed."));
						return;
					} else if (NickServ::IsRegistered(split[3]) == false) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The nick %s is not registered.", split[3].c_str()));
						return;
					} else if (NickServ::GetOption("NOACCESS", split[3]) == true) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The option NOACCESS is enabled for this user."));
						return;
					}
					if (ChanServ::Access(split[3], split[1]) != 0) {
						string acceso;
						switch (ChanServ::Access(split[3], split[1])) {
							case 1: acceso = "VOP"; break;
							case 2: acceso = "HOP"; break;
							case 3: acceso = "AOP"; break;
							case 4: acceso = "SOP"; break;
							case 5: acceso = "FUNDADOR"; break;
							default: acceso = "NINGUNO"; break;
						}
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The nickname has already access: %s", acceso.c_str()));
						return;
					} else {
						string sql = "INSERT INTO ACCESS VALUES ('" + split[1] + "', '" + cmd + "', '" + split[3] + "', '" + user->mNickName + "');";
						if (DB::SQLiteNoReturn(sql) == false) {
							user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "Cannot insert the record."));
							return;
						}
						if (config["database"]["cluster"].as<bool>() == false) {
							sql = "DB " + DB::GenerateID() + " " + sql;
							DB::AlmacenaDB(sql);
							Server::Send(sql);
						}
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The record has been inserted."));
						User *target = User::GetUser(split[3]);
						if (target)
							ChanServ::CheckModes(target, split[1]);
					}
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "%s has been given to %s", cmd.c_str(), split[3].c_str()));
				} else if (strcasecmp(split[2].c_str(), "DEL") == 0) {
					if (split.size() < 4) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "More data is needed."));
						return;
					}
					if (ChanServ::Access(split[3], split[1]) == 0) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The user do not have access."));
						return;
					}
					string sql = "DELETE FROM ACCESS WHERE USUARIO='" + split[3] + "'  AND CANAL='" + split[1] + "'  AND ACCESO='" + cmd + "';";
					if (DB::SQLiteNoReturn(sql) == false) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The record cannot be deleted."));
						return;
					}
					if (config["database"]["cluster"].as<bool>() == false) {
						sql = "DB " + DB::GenerateID() + " " + sql;
						DB::AlmacenaDB(sql);
						Server::Send(sql);
					}
					User *target = User::GetUser(split[3]);
					if (target)
						ChanServ::CheckModes(target, split[1]);
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "%s has been removed to %s", cmd.c_str(), split[3].c_str()));
				} else if (strcasecmp(split[2].c_str(), "LIST") == 0) {
					vector<vector<string> > result;
					string sql = "SELECT USUARIO, ADDED FROM ACCESS WHERE CANAL='" + split[1] + "' AND ACCESO='" + cmd + "' ORDER BY USUARIO;";
					result = DB::SQLiteReturnVectorVector(sql);
					if (result.size() == 0) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "There is no access of %s", cmd.c_str()));
						return;
					}
					for(vector<vector<string> >::iterator it = result.begin(); it < result.end(); ++it)
					{
						vector<string> row = *it;
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "\002%s\002 accessed by %s", row.at(0).c_str(), row.at(1).c_str()));
					}
				}
				return;
			}
		} else if (cmd == "TOPIC") {
			if (split.size() < 3) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "More data is needed."));
				return;
			} else if (Server::HUBExiste() == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The HUB doesnt exists, DBs are in read-only mode."));
				return;
			} else if (user->getMode('r') == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "To make this action, you need identify first."));
				return;
			} else if (ChanServ::IsRegistered(split[1]) == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The channel %s is not registered.", split[1].c_str()));
				return;
			} else if (ChanServ::Access(user->mNickName, split[1]) < 3) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "You do not have enough access."));
				return;
			} else {
				int pos = 7 + split[1].length();
				string topic = message.substr(pos);
				if (topic.length() > 250) {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The topic is too long."));
					return;
				}
				string sql = "UPDATE CANALES SET TOPIC='" + encode_base64((const unsigned char*)topic.c_str(), topic.length()) + "' WHERE NOMBRE='" + split[1] + "';";
				if (DB::SQLiteNoReturn(sql) == false) {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The topic can not be changed."));
					return;
				}
				if (config["database"]["cluster"].as<bool>() == false) {
					sql = "DB " + DB::GenerateID() + " " + sql;
					DB::AlmacenaDB(sql);
					Server::Send(sql);
				}
				Channel* chan = Channel::GetChannel(split[1]);
				if (chan) {
					chan->mTopic = topic;
					chan->broadcast(":" + config["chanserv"].as<std::string>() + " 332 " + user->mNickName + " " + chan->name + " :" + topic);
					chan->broadcast(":" + config["chanserv"].as<std::string>() + " 333 " + user->mNickName + " " + chan->name + " " + config["chanserv"].as<std::string>() + " " + std::to_string(time(0)));
				}
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The topic has changed."));
				return;
			}
		} else if (cmd == "AKICK") {
			if (split.size() < 3) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "More data is needed."));
				return;
			} else if (Server::HUBExiste() == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The HUB doesnt exists, DBs are in read-only mode."));
				return;
			} else if (user->getMode('r') == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "To make this action, you need identify first."));
				return;
			} else if (ChanServ::IsRegistered(split[1]) == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The channel %s is not registered.", split[1].c_str()));
				return;
			} else if (ChanServ::Access(user->mNickName, split[1]) < 4) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "You do not have enough access."));
				return;
			} else {
				if (strcasecmp(split[2].c_str(), "ADD") == 0) {
					if (split.size() < 5) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "More data is needed."));
						return;
					}
					if (ChanServ::IsAKICK(split[3], split[1]) != 0) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The mask already got AKICK."));
						return;
					} else {
						int posicion = 4 + cmd.length() + split[1].length() + split[2].length() + split[3].length();
						string motivo = message.substr(posicion);
						if (DB::EscapeChar(motivo) == true || DB::EscapeChar(split[3]) == true) {
							user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The reason or the mask contains no-valid characters."));
							return;
						}
						string sql = "INSERT INTO AKICK VALUES ('" + split[1] + "', '" + split[3] + "', '" + motivo + "', '" + user->mNickName + "');";
						if (DB::SQLiteNoReturn(sql) == false) {
							user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The AKICK can not be inserted."));
							return;
						}
						if (config["database"]["cluster"].as<bool>() == false) {
							sql = "DB " + DB::GenerateID() + " " + sql;
							DB::AlmacenaDB(sql);
							Server::Send(sql);
						}
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The record has been inserted."));
					}
				} else if (strcasecmp(split[2].c_str(), "DEL") == 0) {
					if (split.size() < 4) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "More data is needed."));
						return;
					}
					if (ChanServ::IsAKICK(split[3], split[1]) == 0) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The user do not have AKICK."));
						return;
					}
					string sql = "DELETE FROM AKICK WHERE MASCARA='" + split[3] + "'  AND CANAL='" + split[1] + "';";
					if (DB::SQLiteNoReturn(sql) == false) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The record cannot be deleted."));
						return;
					}
					if (config["database"]["cluster"].as<bool>() == false) {
						sql = "DB " + DB::GenerateID() + " " + sql;
						DB::AlmacenaDB(sql);
						Server::Send(sql);
					}
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The AKICK for %s has been deleted.", split[3].c_str()));
				} else if (strcasecmp(split[2].c_str(), "LIST") == 0) {
					vector<vector<string> > result;
					string sql = "SELECT MASCARA, ADDED, MOTIVO FROM AKICK WHERE CANAL='" + split[1] + "' ORDER BY MASCARA;";
					result = DB::SQLiteReturnVectorVector(sql);
					if (result.size() == 0) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "AKICK list of channel %s is empty.", split[1].c_str()));
						return;
					}
					for(vector<vector<string> >::iterator it = result.begin(); it < result.end(); ++it)
					{
						vector<string> row = *it;
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "\002%s\002 updated by %s. Reason: %s", row.at(0).c_str(), row.at(1).c_str(), row.at(2).c_str()));
					}
				}
				return;
			}
		} else if (cmd == "OP" || cmd == "DEOP" || cmd == "HALFOP" || cmd == "DEHALFOP" || cmd == "VOICE" || cmd == "DEVOICE") {
			if (split.size() < 3) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "More data is needed."));
				return;
			} else if (user->getMode('r') == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "To make this action, you need identify first."));
				return;
			} else if (ChanServ::IsRegistered(split[1]) == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The channel %s is not registered.", split[1].c_str()));
				return;
			} else if (ChanServ::Access(user->mNickName, split[1]) < 3) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "You do not have enough access."));
				return;
			} else if (!User::FindUser(split[2])) {
				user->deliver(":" + config["serverName"].as<std::string>() + " 401 " + user->mNickName + " :" + Utils::make_string(user->mLang, "The nick does not exist."));
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
				} else if (cmd == "VOICE") {
					modo = 'v';
					action = 1;
				} else if (cmd == "DEVOICE") {
					modo = 'v';
					action = 0;
				} else
					return;

				Channel* chan = Channel::GetChannel(split[1]);
				User *target = User::GetUser(split[2]);
				
				if (!chan || !target)
					return;
				if (chan->IsVoice(target)) {
					if (modo == 'h' && action == 1) {
						chan->broadcast(":" + config["chanserv"].as<std::string>() + " MODE " + chan->name + " -v " + target->mNickName);
						chan->RemoveVoice(target);
						Server::Send("CMODE " + config["chanserv"].as<std::string>() + " " + chan->name + " -v " + target->mNickName);
						chan->broadcast(":" + config["chanserv"].as<std::string>() + " MODE " + chan->name + " +h " + target->mNickName);
						chan->GiveHalfOperator(target);
						Server::Send("CMODE " + config["chanserv"].as<std::string>() + " " + chan->name + " +h " + target->mNickName);
					} else if (modo == 'o' && action == 1) {
						chan->broadcast(":" + config["chanserv"].as<std::string>() + " MODE " + chan->name + " -v " + target->mNickName);
						chan->RemoveVoice(target);
						Server::Send("CMODE " + config["chanserv"].as<std::string>() + " " + chan->name + " -v " + target->mNickName);
						chan->broadcast(":" + config["chanserv"].as<std::string>() + " MODE " + chan->name + " +o " + target->mNickName);
						chan->GiveOperator(target);
						Server::Send("CMODE " + config["chanserv"].as<std::string>() + " " + chan->name + " +o " + target->mNickName);
					} else if (modo == 'v' && action == 1) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The nick already got %s.", "VOICE"));
					} else if (modo == 'v' && action == 0) {
						chan->broadcast(":" + config["chanserv"].as<std::string>() + " MODE " + chan->name + " -v " + target->mNickName);
						chan->RemoveVoice(target);
						Server::Send("CMODE " + config["chanserv"].as<std::string>() + " " + chan->name + " -v " + target->mNickName);
					}
				} else if (chan->IsHalfOperator(target)) {
					if (modo == 'v' && action == 1) {
						chan->broadcast(":" + config["chanserv"].as<std::string>() + " MODE " + chan->name + " -h " + target->mNickName);
						chan->RemoveHalfOperator(target);
						Server::Send("CMODE " + config["chanserv"].as<std::string>() + " " + chan->name + " -h " + target->mNickName);
						chan->broadcast(":" + config["chanserv"].as<std::string>() + " MODE " + chan->name + " +v " + target->mNickName);
						chan->GiveVoice(target);
						Server::Send("CMODE " + config["chanserv"].as<std::string>() + " " + chan->name + " +v " + target->mNickName);
					} else if (modo == 'o' && action == 1) {
						chan->broadcast(":" + config["chanserv"].as<std::string>() + " MODE " + chan->name + " -h " + target->mNickName);
						chan->RemoveHalfOperator(target);
						Server::Send("CMODE " + config["chanserv"].as<std::string>() + " " + chan->name + " -h " + target->mNickName);
						chan->broadcast(":" + config["chanserv"].as<std::string>() + " MODE " + chan->name + " +o " + target->mNickName);
						chan->GiveOperator(target);
						Server::Send("CMODE " + config["chanserv"].as<std::string>() + " " + chan->name + " +o " + target->mNickName);
					} else if (modo == 'h' && action == 1) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The nick already got %s.", "HALFOP"));
					} else if (modo == 'h' && action == 0) {
						chan->broadcast(":" + config["chanserv"].as<std::string>() + " MODE " + chan->name + " -h " + target->mNickName);
						chan->RemoveHalfOperator(target);
						Server::Send("CMODE " + config["chanserv"].as<std::string>() + " " + chan->name + " -h " + target->mNickName);
					}
				} else if (chan->IsOperator(target)) {
					if (modo == 'v' && action == 1) {
						chan->broadcast(":" + config["chanserv"].as<std::string>() + " MODE " + chan->name + " -o " + target->mNickName);
						chan->RemoveOperator(target);
						Server::Send("CMODE " + config["chanserv"].as<std::string>() + " " + chan->name + " -o " + target->mNickName);
						chan->broadcast(":" + config["chanserv"].as<std::string>() + " MODE " + chan->name + " +v " + target->mNickName);
						chan->GiveVoice(target);
						Server::Send("CMODE " + config["chanserv"].as<std::string>() + " " + chan->name + " +v " + target->mNickName);
					} else if (modo == 'h' && action == 1) {
						chan->broadcast(":" + config["chanserv"].as<std::string>() + " MODE " + chan->name + " -o " + target->mNickName);
						chan->RemoveOperator(target);
						Server::Send("CMODE " + config["chanserv"].as<std::string>() + " " + chan->name + " -o " + target->mNickName);
						chan->broadcast(":" + config["chanserv"].as<std::string>() + " MODE " + chan->name + " +h " + target->mNickName);
						chan->GiveHalfOperator(target);
						Server::Send("CMODE " + config["chanserv"].as<std::string>() + " " + chan->name + " +h " + target->mNickName);
					} else if (modo == 'o' && action == 1) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The nick already got %s.", "OP"));
					} else if (modo == 'o' && action == 0) {
						chan->broadcast(":" + config["chanserv"].as<std::string>() + " MODE " + chan->name + " -o " + target->mNickName);
						chan->RemoveOperator(target);
						Server::Send("CMODE " + config["chanserv"].as<std::string>() + " " + chan->name + " -o " + target->mNickName);
					}
				} else if (chan->HasUser(target)) {
					if (modo == 'v' && action == 1) {
						chan->broadcast(":" + config["chanserv"].as<std::string>() + " MODE " + chan->name + " +v " + target->mNickName);
						chan->GiveVoice(target);
						Server::Send("CMODE " + config["chanserv"].as<std::string>() + " " + chan->name + " +v " + target->mNickName);
					} else if (modo == 'h' && action == 1) {
						chan->broadcast(":" + config["chanserv"].as<std::string>() + " MODE " + chan->name + " +h " + target->mNickName);
						chan->GiveHalfOperator(target);
						Server::Send("CMODE " + config["chanserv"].as<std::string>() + " " + chan->name + " +h " + target->mNickName);
					} else if (modo == 'o' && action == 1) {
						chan->broadcast(":" + config["chanserv"].as<std::string>() + " MODE " + chan->name + " +o " + target->mNickName);
						chan->GiveOperator(target);
						Server::Send("CMODE " + config["chanserv"].as<std::string>() + " " + chan->name + " +o " + target->mNickName);
					} else if (action == 0){
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The nick do not have any mode."));
					}
				}
			}
		} else if (cmd == "KEY") {
			if (split.size() < 3) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "More data is needed."));
				return;
			} else if (Server::HUBExiste() == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The HUB doesnt exists, DBs are in read-only mode."));
				return;
			} else if (user->getMode('r') == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "To make this action, you need identify first."));
				return;
			} else if (ChanServ::IsRegistered(split[1]) == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The channel %s is not registered.", split[1].c_str()));
				return;
			} else if (ChanServ::Access(user->mNickName, split[1]) < 5) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "You do not have enough access."));
				return;
			} else {
				string key = split[2];
				if (DB::EscapeChar(key) == true) {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The key contains no-valid characters."));
					return;
				}
				if (key.length() > 32) {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The key is too long."));
					return;
				}
				if (strcasecmp(key.c_str(), "OFF") == 0) {
					string sql = "UPDATE CANALES SET CLAVE='' WHERE NOMBRE='" + split[1] + "';";
					if (DB::SQLiteNoReturn(sql) == false) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The key can not be changed."));
						return;
					}
					if (config["database"]["cluster"].as<bool>() == false) {
						sql = "DB " + DB::GenerateID() + " " + sql;
						DB::AlmacenaDB(sql);
						Server::Send(sql);
					}
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The key has been removed.", key.c_str()));
				} else {
					string sql = "UPDATE CANALES SET CLAVE='" + key + "' WHERE NOMBRE='" + split[1] + "';";
					if (DB::SQLiteNoReturn(sql) == false) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The key can not be changed."));
						return;
					}
					if (config["database"]["cluster"].as<bool>() == false) {
						sql = "DB " + DB::GenerateID() + " " + sql;
						DB::AlmacenaDB(sql);
						Server::Send(sql);
					}
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The key has changed to: %s", key.c_str()));
				}
			}
		} else if (cmd == "MODE") {
			if (split.size() < 3) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "More data is needed."));
				return;
			} else if (Server::HUBExiste() == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The HUB doesnt exists, DBs are in read-only mode."));
				return;
			} else if (user->getMode('r') == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "To make this action, you need identify first."));
				return;
			} else if (ChanServ::IsRegistered(split[1]) == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The channel %s is not registered.", split[1].c_str()));
				return;
			} else if (ChanServ::Access(user->mNickName, split[1]) < 4) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "You do not have enough access."));
				return;
			} else {
				string mode = split[2].substr(1);
				std::transform(mode.begin(), mode.end(), mode.begin(), ::toupper);
				if (strcasecmp("LIST", split[2].c_str()) == 0) {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The available modes are: flood, onlyreg, autovoice, moderated, onlysecure, nonickchange, onlyweb, country, onlyaccess"));
					return;
				} else if (strcasecmp("FLOOD", mode.c_str()) && strcasecmp("ONLYREG", mode.c_str()) && strcasecmp("AUTOVOICE", mode.c_str()) &&
							strcasecmp("MODERATED", mode.c_str()) && strcasecmp("ONLYSECURE", mode.c_str()) && strcasecmp("NONICKCHANGE", mode.c_str()) &&
							strcasecmp("ONLYWEB", mode.c_str()) && strcasecmp("COUNTRY", mode.c_str()) && strcasecmp("ONLYACCESS", mode.c_str())) {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "Unknown mode."));
					return;
				} if (split[2][0] == '+') {
					std::string sql;
					if (ChanServ::HasMode(split[1], mode) == true) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The channel %s already got the mode %s", split[1].c_str(), mode.c_str()));
						return;
					} if ((mode == "COUNTRY" || mode == "FLOOD") && split.size() != 4) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The flood mode got parameters."));
						return;
					} else if (mode == "FLOOD" && (!Utils::isnum(split[3]) || stoi(split[3]) < 1 || stoi(split[3]) > 999)) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The parameter of flood mode is incorrect."));
						return;
					} else if (mode == "FLOOD")
						sql = "UPDATE CMODES SET " + mode + "=" + split[3] + " WHERE CANAL='" + split[1] + "';";
					else if (mode == "COUNTRY")
						sql = "UPDATE CMODES SET " + mode + "='" + split[3] + "' WHERE CANAL='" + split[1] + "';";
					else
						sql = "UPDATE CMODES SET " + mode + "=1 WHERE CANAL='" + split[1] + "';";
					if (DB::SQLiteNoReturn(sql) == false) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The mode can not be setted."));
						return;
					}
					if (config["database"]["cluster"].as<bool>() == false) {
						sql = "DB " + DB::GenerateID() + " " + sql;
						DB::AlmacenaDB(sql);
						Server::Send(sql);
					}
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The mode is setted."));
					return;
				} else if (split[2][0] == '-') {
					if (ChanServ::HasMode(split[1], mode) == false) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The channel %s doesnt got the mode %s", split[1].c_str(), mode.c_str()));
						return;
					}
					string sql;
					if (mode == "COUNTRY")
						sql = "UPDATE CMODES SET " + mode + "='' WHERE CANAL='" + split[1] + "';";
					else
						sql = "UPDATE CMODES SET " + mode + "=0 WHERE CANAL='" + split[1] + "';";
					if (DB::SQLiteNoReturn(sql) == false) {
						user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The mode cannot be removed."));
						return;
					}
					if (config["database"]["cluster"].as<bool>() == false) {
						sql = "DB " + DB::GenerateID() + " " + sql;
						DB::AlmacenaDB(sql);
						Server::Send(sql);
					}
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The mode has been removed."));
					return;
				}
			}
		} else if (cmd == "TRANSFER") {
			if (split.size() < 3) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "More data is needed."));
				return;
			} else if (NickServ::IsRegistered(split[2]) == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The target nick is not registered."));
				return;
			} else if (Server::HUBExiste() == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The HUB doesnt exists, DBs are in read-only mode."));
				return;
			} else if (user->getMode('r') == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "To make this action, you need identify first."));
				return;
			} else if (ChanServ::IsRegistered(split[1]) == false) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The channel %s is not registered.", split[1].c_str()));
				return;
			} else if (ChanServ::Access(user->mNickName, split[1]) < 5) {
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "You do not have enough access to change founder."));
				return;
			} else {
				string sql = "UPDATE CANALES SET OWNER='" + split[2] + "' WHERE NOMBRE='" + split[1] + "';";
				if (DB::SQLiteNoReturn(sql) == false) {
					user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The founder cannot be changed."));
					return;
				}
				if (config["database"]["cluster"].as<bool>() == false) {
					sql = "DB " + DB::GenerateID() + " " + sql;
					DB::AlmacenaDB(sql);
					Server::Send(sql);
				}
				user->deliver(":" + config["chanserv"].as<std::string>() + " NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The founder has changed to: %s", split[2].c_str()));
				return;
			}
		}
	}
};

extern "C" Widget* factory(void) {
	return static_cast<Widget*>(new CMD_CS);
}
