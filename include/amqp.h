/*
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */
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

#pragma once

#include "options.h"
#include "Server.h"
#include "Config.h"

#include <proton/container.hpp>
#include <proton/default_container.hpp>
#include <proton/listen_handler.hpp>
#include <proton/listener.hpp>
#include <proton/message.hpp>
#include <proton/message_id.hpp>
#include <proton/messaging_handler.hpp>
#include <proton/sender.hpp>
#include <proton/sender_options.hpp>
#include <proton/source_options.hpp>
#include <proton/tracker.hpp>
#include <proton/connection.hpp>
#include <proton/delivery.hpp>
#include <proton/receiver_options.hpp>
#include <proton/connection_options.hpp>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <cctype>
#include <vector>

extern std::set <Server*> Servers;
extern std::string OwnAMQP;

class client : public proton::messaging_handler {
    std::string url;
    std::string ip;
    std::string queue;
    proton::sender sender;

  public:
	client(const std::string &u, const std::string &ipe, std::string m) : url(u), ip(ipe), queue(m) {}
	void on_container_start(proton::container &c) override;
	void on_connection_open(proton::connection& c) override;
	void on_sendable(proton::sender &s) override;
	void on_tracker_accept(proton::tracker &t) override;
	void on_sender_error(proton::sender &s) override;
	void on_error (const proton::error_condition &c) override;
};

class serveramqp : public proton::messaging_handler {
  private:
	std::string url;
	proton::listener listener;

  public:
	serveramqp(const std::string &u) : url(u) {};
	~serveramqp() {};
	void on_container_start(proton::container &c) override;
	void on_message(proton::delivery &d, proton::message &m) override;
};
