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

class CMD_Pong : public Module
{
	public:
	CMD_Pong() : Module("PONG", 50, false) {};
	~CMD_Pong() {};
	virtual void command(User *user, std::string message) override {
		user->bPing = time(0);
	}
};

extern "C" Widget* factory(void) {
	return static_cast<Widget*>(new CMD_Pong);
}
