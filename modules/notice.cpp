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
 *
 * This file include code from some part of github i can't remember.
*/

#include "ZeusiRCd.h"
#include "module.h"
#include "Utils.h"
#include "services.h"

std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ");
std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ");
std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ");

extern Memos MemoMsg;

class CMD_Notice : public Module
{
	public:
	CMD_Notice() : Module("NOTICE", 50, false) {};
	~CMD_Notice() {};
	virtual void command(User *user, std::string message) override {
		std::vector<std::string> results;
		Utils::split(message, results, " ");
		if (!user->bSentNick) {
			user->SendAsServer("461 ZeusiRCd :" + Utils::make_string(user->mLang, "You havent used the NICK command yet, you have limited access."));
			return;
		}
		else if (results.size() < 3) return;
		std::string mensaje = "";
		for (unsigned int i = 2; i < results.size(); ++i) { mensaje.append(results[i] + " "); }
		trim(mensaje);
		if (results[1][0] == '#') {
			Channel* chan = Channel::GetChannel(results[1]);
			if (chan) {
				if (ChanServ::IsRegistered(chan->name) == true && ChanServ::HasMode(chan->name, "MODERATED") &&
						!chan->IsOperator(user) && !chan->IsHalfOperator(user) && !chan->IsVoice(user) && user->getMode('o') == false) {
					user->SendAsServer("461 " + user->mNickName + " :" + Utils::make_string(user->mLang, "The channel is moderated, you cannot speak."));
					return;
				} else if (chan->isonflood() == true && ChanServ::Access(user->mNickName, chan->name) == 0) {
					user->SendAsServer("461 " + user->mNickName + " :" + Utils::make_string(user->mLang, "The channel is on flood, you cannot speak."));
					return;
				} else if (chan->HasUser(user) == false) {
					user->SendAsServer("461 " + user->mNickName + " :" + Utils::make_string(user->mLang, "You are not into the channel."));
					return;
				} else if (chan->IsBan(user->mNickName + "!" + user->mIdent + "@" + user->mCloak) == true) {
					user->SendAsServer("461 " + user->mNickName + " :" + Utils::make_string(user->mLang, "You are banned, cannot speak."));
					return;
				} else if (chan->IsBan(user->mNickName + "!" + user->mIdent + "@" + user->mvHost) == true) {
					user->SendAsServer("461 " + user->mNickName + " :" + Utils::make_string(user->mLang, "You are banned, cannot speak."));
					return;
				} else if (OperServ::IsSpam(mensaje, "C") == true && OperServ::IsSpam(mensaje, "E") == false && user->getMode('o') == false && strcasecmp(chan->name.c_str(), "#spam") != 0) {
					Oper oper;
					oper.GlobOPs(Utils::make_string("", "Nickname %s try to make SPAM into channel: %s", user->mNickName.c_str(), chan->name.c_str()));
					user->SendAsServer("461 " + user->mNickName + " :" + Utils::make_string(user->mLang, "The message of channel %s contains SPAM.", chan->name.c_str()));
					return;
				}
				chan->increaseflood();
				chan->broadcast_except_me(user->mNickName,
					user->messageHeader()
					+ "NOTICE "
					+ chan->name + " "
					+ mensaje);
				Server::Send(cmd + " " + user->mNickName + "!" + user->mIdent + "@" + user->mvHost + " " + chan->name + " " + mensaje);
			}
		}
		else {
			User* target = User::GetUser(results[1]);
			if (!target) {
				user->SendAsServer("461 " + user->mNickName + " :" + Utils::make_string(user->mLang, "The nick doesnt exists or cannot receive messages."));
				return;
			} else if (OperServ::IsSpam(mensaje, "P") == true && OperServ::IsSpam(mensaje, "E") == false && user->getMode('o') == false && target) {
				Oper oper;
				oper.GlobOPs(Utils::make_string("", "Nickname %s try to make SPAM to nick: %s", user->mNickName.c_str(), target->mNickName.c_str()));
				user->SendAsServer("461 " + user->mNickName + " :" + Utils::make_string(user->mLang, "Message to nick %s contains SPAM.", target->mNickName.c_str()));
				return;
			} else if (NickServ::GetOption("NOCOLOR", results[1]) == true && target && mensaje.find("\003") != std::string::npos) {
				user->SendAsServer("461 " + user->mNickName + " :" + Utils::make_string(user->mLang, "Message to nick %s contains colours.", target->mNickName.c_str()));
				return;
			} else if (target && NickServ::GetOption("ONLYREG", results[1]) == true && user->getMode('r') == false) {
				user->SendAsServer("461 " + user->mNickName + " :" + Utils::make_string(user->mLang, "The nick %s only can receive messages from registered nicks.", target->mNickName.c_str()));
				return;
			}
			if (target->is_local == true) {
				if (target->bAway == true) {
					user->deliver(target->messageHeader()
						+ "NOTICE "
						+ user->mNickName + " :AWAY " + target->mAway);
				}
				target->deliver(user->messageHeader()
					+ "NOTICE "
					+ target->mNickName + " "
					+ mensaje);
				return;
			} else {
				if (target->bAway == true) {
					user->deliver(target->messageHeader()
						+ "NOTICE "
						+ user->mNickName + " :AWAY " + target->mAway);
				}
				Server::Send(cmd + " " + user->mNickName + "!" + user->mIdent + "@" + user->mvHost + " " + target->mNickName + " " + mensaje);
				return;
			} if (!target && NickServ::IsRegistered(results[1]) == true && NickServ::MemoNumber(results[1]) < 50 && NickServ::GetOption("NOMEMO", results[1]) == 0) {
				Memo *memo = new Memo();
					memo->sender = user->mNickName;
					memo->receptor = results[1];
					memo->time = time(0);
					memo->mensaje = mensaje;
				MemoMsg.insert(memo);
				user->deliver(":NiCK!*@* NOTICE " + user->mNickName + " :" + Utils::make_string(user->mLang, "The nick is offline, MeMo has been sent."));
				Server::Send("MEMO " + memo->sender + " " + memo->receptor + " " + std::to_string(memo->time) + " " + memo->mensaje);
				return;
			} else
				user->SendAsServer("461 " + user->mNickName + " :" + Utils::make_string(user->mLang, "The nick doesnt exists or cannot receive messages."));
		}
	}
};

extern "C" Widget* factory(void) {
	return static_cast<Widget*>(new CMD_Notice);
}
