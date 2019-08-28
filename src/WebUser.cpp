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

#include "ZeusBaseClass.h"

#include <boost/range/algorithm/remove_if.hpp>
#include <boost/algorithm/string/classification.hpp>

void LocalWebUser::Send(std::string message)
{
	message.append("\r\n");
	if (get_lowest_layer(Socket).socket().is_open())
		Socket.write(boost::asio::buffer(message, message.length()));
}

void LocalWebUser::Close()
{
	Exit();
	boost::beast::close_socket(get_lowest_layer(Socket));
}

std::string LocalWebUser::ip()
{
	try
	{
		if (get_lowest_layer(Socket).socket().is_open())
			return get_lowest_layer(Socket).socket().remote_endpoint().address().to_string();
	}
	catch (boost::system::system_error &e)
	{
		std::cout << "ERROR getting IP in WSS mode" << std::endl;
	}
	return "127.0.0.0";
}

void LocalWebUser::start()
{
	read();
	deadline.cancel();
	deadline.expires_from_now(boost::posix_time::seconds(60));
	deadline.async_wait(boost::bind(&LocalWebUser::check_ping, this, boost::asio::placeholders::error));
	mHost = ip();
}

void LocalWebUser::check_ping(const boost::system::error_code &e)
{
	if (!e)
	{
		if (bPing + 200 < time(0))
		{
			deadline.cancel();
			Close();
		}
		else
		{
			deadline.cancel();
			deadline.expires_from_now(boost::posix_time::seconds(60));
		        deadline.async_wait(boost::bind(&LocalWebUser::check_ping, this, boost::asio::placeholders::error));
		}
	}
}

void LocalWebUser::read()
{
	if (get_lowest_layer(Socket).socket().is_open())
		boost::asio::async_read_until(Socket, mBuffer, '\n', boost::asio::bind_executor(strand,
			boost::bind(&LocalWebUser::handleRead, shared_from_this(),
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)));
}

void LocalWebUser::handleRead(const boost::system::error_code &error, std::size_t bytes)
{
	if (handshake == false)
	{
		Socket.accept();
		handshake = true;
		read();
	}
	else if (!error)
	{
		std::string message;
		std::istream istream(&mBuffer);
		std::getline(istream, message);

		message.erase(boost::remove_if(message, boost::is_any_of("\r\n")), message.end());

		if (message.length() > 1024)
			message.substr(0, 1024);

		boost::asio::post(strand, boost::bind(&LocalWebUser::Parse, shared_from_this(), message));

		read();
	}
	else
		Close();
}
