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
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/asio/strand.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "mainframe.h"
#include "user.h"
#include "parser.h"
#include "websocket.h"
#include "utils.h"
#include "services.h"
#include "server.h"

#define GC_THREADS
#define GC_ALWAYS_MULTITHREADED
#include <gc_cpp.h>
#include <gc.h>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;
extern boost::asio::io_context channel_user_context;

void
fail(boost::system::error_code ec, const std::string &what)
{
    std::cout << "ERROR WebSockets: " << what << ": " << ec.message() << std::endl;
}

class listener : public std::enable_shared_from_this<listener>
{
    net::io_context& ioc_;
    ssl::context& ctx_;
    tcp::acceptor acceptor_;
    boost::asio::deadline_timer deadline;

public:
    listener(
        net::io_context& ioc,
        ssl::context& ctx,
        tcp::endpoint endpoint)
        : ioc_(ioc)
        , ctx_(ctx)
        , acceptor_(net::make_strand(ioc))
        , deadline(channel_user_context)
    {
        beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if(ec)
        {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if(ec)
        {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if(ec)
        {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(
            net::socket_base::max_listen_connections, ec);
        if(ec)
        {
            fail(ec, "listen");
            return;
        }
    }

    // Start accepting incoming connections
    void
    run()
    {
        do_accept();
    }

private:
    void
    do_accept()
    {
		boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
		ctx.set_options(
		boost::asio::ssl::context::default_workarounds
		| boost::asio::ssl::context::no_sslv2);
		ctx.use_certificate_file("server.pem", boost::asio::ssl::context::pem);
		ctx.use_certificate_chain_file("server.pem");
		ctx.use_private_key_file("server.key", boost::asio::ssl::context::pem);
		ctx.use_tmp_dh_file("dh.pem");
		std::shared_ptr<Session> newclient(new (GC) Session(acceptor_.get_executor(), ctx));
		newclient->websocket = true;
		acceptor_.async_accept(
            net::make_strand(ioc_),
            boost::bind(
                &listener::on_accept,
                shared_from_this(),
				newclient,
				boost::asio::placeholders::error));
    }
	void
	handle_handshake(const std::shared_ptr<Session>& newclient, const boost::system::error_code& error) {
		deadline.cancel();
		if (error){
			newclient->close();
		} else {
			if (stoi(config->Getvalue("maxUsers")) <= Mainframe::instance()->countusers()) {
				newclient->sendAsServer("465 ZeusiRCd :" + Utils::make_string("", "The server has reached maximum number of connections.") + config->EOFMessage);
				newclient->close();
			} else if (Server::CheckClone(newclient->ip()) == true) {
				newclient->sendAsServer("465 ZeusiRCd :" + Utils::make_string("", "You have reached the maximum number of clones.") + config->EOFMessage);
				newclient->close();
			} else if (Server::CheckDNSBL(newclient->ip()) == true) {
				newclient->sendAsServer("465 ZeusiRCd :" + Utils::make_string("", "Your IP is in our DNSBL lists.") + config->EOFMessage);
				newclient->close();
			} else if (Server::CheckThrottle(newclient->ip()) == true) {
				newclient->sendAsServer("465 ZeusiRCd :" + Utils::make_string("", "You connect too fast, wait 30 seconds to try connect again.") + config->EOFMessage);
				newclient->close();
			} else if (OperServ::IsGlined(newclient->ip()) == true) {
				newclient->sendAsServer("465 ZeusiRCd :" + Utils::make_string("", "You are G-Lined. Reason: %s", OperServ::ReasonGlined(newclient->ip()).c_str()) + config->EOFMessage);
				newclient->close();
			} else if (OperServ::CanGeoIP(newclient->ip()) == false) {
				newclient->sendAsServer("465 ZeusiRCd :" + Utils::make_string("", "You can not connect from your country.") + config->EOFMessage);
				newclient->close();
			} else {
				Server::ThrottleUP(newclient->ip());
				newclient->websocket = true;
				newclient->start();
			}
		}
	}
	void check_deadline(const std::shared_ptr<Session>& newclient, const boost::system::error_code &e)
	{
		if (e) {
			newclient->close();
		}
	}
    void
    on_accept(const std::shared_ptr<Session>& newclient, const boost::system::error_code &ec)
    {
		do_accept();
        if(ec)
        {
            newclient->close();
        } else {
			newclient->socket_wss().next_layer().async_handshake(boost::asio::ssl::stream_base::server, boost::bind(&listener::handle_handshake,   this,   newclient,  boost::asio::placeholders::error));
			deadline.expires_from_now(boost::posix_time::seconds(10));
			deadline.async_wait(boost::bind(&listener::check_deadline, this, newclient, boost::asio::placeholders::error));
		}
    }
};

WebSocket::WebSocket(boost::asio::io_context& io_context, std::string ip, int port, bool ssl, bool ipv6)
{
	auto const address = net::ip::make_address(ip);
    auto const port_ = static_cast<unsigned short>(port);
	boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
	ctx.set_options(
        boost::asio::ssl::context::default_workarounds
        | boost::asio::ssl::context::no_sslv2);
	ctx.use_certificate_file("server.pem", boost::asio::ssl::context::pem);
	ctx.use_certificate_chain_file("server.pem");
	ctx.use_private_key_file("server.key", boost::asio::ssl::context::pem);
	ctx.use_tmp_dh_file("dh.pem");
	std::make_shared<listener>(io_context, ctx, tcp::endpoint{address, port_})->run();
}
