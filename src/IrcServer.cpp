/*
 * ircbot.cpp
 *
 *  Created on: 29 Jul 2011
 *      Author: oaskivvy@gmail.com
 */

/*-----------------------------------------------------------------.
| Copyright (C) 2011 SooKee oaskivvy@gmail.com               |
'------------------------------------------------------------------'

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.

http://www.gnu.org/licenses/gpl-2.0.html

'-----------------------------------------------------------------*/

#include <hol/small_types.h>
#include <hol/bug.h>
#include <hol/simple_logger.h>
#include <hol/string_utils.h>

#include <skivvy/IrcServer.h>
#include <skivvy/irc-constants.h>

namespace skivvy { namespace ircbot {

using namespace skivvy;
using namespace skivvy::irc;

namespace hol {
	using namespace header_only_library::string_utils;
}

using namespace header_only_library::small_types::basic;
using namespace header_only_library::small_types::string_containers;
using namespace header_only_library::simple_logger;

// BASE

str ctcp_escape(str s)
{
	return s;
}

bool BaseIrcServer::send(const str& cmd)
{
	LOG::I << "send: " << cmd;
	return send_unlogged(cmd);
}

bool BaseIrcServer::pass(const str& pwd)
{
	if(pwd.empty())
		return send(PASS + " none");
	return send(PASS + " " + pwd);
}

bool BaseIrcServer::nick(const str& n)
{
	return send(NICK + " " + n);
}

bool BaseIrcServer::user(const str& u, const str m, const str r)
{
	return send(USER + " " + u + " " + m + " * :" + r);
}

bool BaseIrcServer::join(const str& channel, const str& key)
{
	return send(JOIN + " " + channel + " " + key);
}

bool BaseIrcServer::part(const str& channel, const str& message)
{
	return send(PART + " " + channel + " " + message);
}

bool BaseIrcServer::ping(const str& info)
{
	return send_unlogged(PING + " " + info);
}

bool BaseIrcServer::pong(const str& info)
{
	return send_unlogged(PONG + " " + info);
}

bool BaseIrcServer::say(const str& to, const str& text)
{
	return send(PRIVMSG + " " + to + " :" + text);
}

bool BaseIrcServer::ctcp(const str& to, const str& text)
{
	return send(PRIVMSG + " " + to + " :\001" + ctcp_escape(text) + '\001');
}

bool BaseIrcServer::notice(const str& to, const str& text)
{
	return send(NOTICE + " " + to + " :" + text);
}

bool BaseIrcServer::whois(const str_set& masks)
{
	return send(WHOIS + " " + hol::join(std::begin(masks), std::end(masks), ","));
}

bool BaseIrcServer::quit(const str& reason)
{
	return send(QUIT + " :" + reason);
}

// :SooKee!~SooKee@SooKee.users.quakenet.org KICK #skivvy Skivvy :Skivvy

// Parameters: <channel> *( "," <channel> ) <user> *( "," <user> ) [<comment>]
// KICK KICK #skivvy Skivvy :
bool BaseIrcServer::kick(const str_set& chans, const str_set& users, const str& comment)
{
	str cmd = KICK;

	cmd += " " + hol::join(std::begin(chans), std::end(chans), ",");
	cmd += " " + hol::join(std::begin(users), std::end(users), ",");

	return send(cmd + " :" + comment);
}

// non standard??
bool BaseIrcServer::auth(const str& user, const str& pass)
{
	return send(AUTH + " " + user + " " + pass);
}

bool BaseIrcServer::me(const str& to, const str& text)
{
	return say(to, "\001ACTION " + text + "\001");
}

bool BaseIrcServer::query(const str& nick)
{
	return send(QUERY + " " + nick);
}

// MODE #skivvy-admin +o Zim-blackberry

bool BaseIrcServer::mode(const str& chan, const str& mode, const str& nick)
{
	return send(MODE + " " + chan + " " + mode + " " + nick);
}

// correct???
bool BaseIrcServer::mode(const str& nick, const str& mode)
{
	return send(MODE + " " + nick + " " + mode);
}

bool BaseIrcServer::reply(const message& msg, const str& text)
{
	str from;
	if(!msg.prefix.empty())
		from = msg.prefix.substr(0, msg.prefix.find("!"));

	if(!msg.get_to().empty() && msg.get_to()[0] == '#')
		return say(msg.get_to(), text);
	else
		return say(from, text);

	return false;
}

bool BaseIrcServer::reply_pm(const message& msg, const str& text)
{
	return say(msg.get_nick(), text);
}

bool BaseIrcServer::reply_notice(const message& msg, const str& text)
{
	str from;
	if(!msg.prefix.empty())
		from = msg.prefix.substr(0, msg.prefix.find("!"));

	if(!msg.get_to().empty() && msg.get_to()[0] == '#')
		return notice(msg.get_to(), text);
	else
		return notice(from, text);

	return false;
}

bool BaseIrcServer::reply_pm_notice(const message& msg, const str& text)
{
	return notice(msg.get_nick(), text);
}


// TEST

bool TestIrcServer::send_unlogged(const str& cmd)
{
	if(!(ofs << cmd.substr(0, 510) << "\r\n" << std::flush))
		LOG::E << "send failed.";
	return bool(ofs);
}

bool TestIrcServer::connect(const str& host, long port)
{
	str ifile = host + "-" + (port<10?"0":"") + std::to_string(port) + "-in.txt";
	str ofile = host + "-" + (port<10?"0":"") + std::to_string(port) + "-out.txt";

	bug_var(ifile);
	bug_var(ofile);

	ifs.clear();
	ifs.open(ifile);
	bug_var((bool)ifs);
	ofs.clear();
	ofs.open(ofile);
	bug_var((bool)ofs);
	return ifs && ofs;
}

void TestIrcServer::close()
{
	ifs.close();
	ofs.close();
}

bool TestIrcServer::receive(str& line)
{
	std::getline(ifs, line);
	return ifs && true;
}

// REMOTE

bool RemoteIrcServer::connect(const str& host, long port)
{
	bug_fun();
	close();
//	if(ss.is_open())
//		ss.close();

	io_service.reset();

	boost::system::error_code ec;
	asio::ip::tcp::resolver resolver(io_service);
	asio::connect(ss, resolver.resolve({host, std::to_string(port)}));

	if(ec)
	{
		LOG::E << ec.message();
		return false;
	}

	return true;
}

void RemoteIrcServer::close()
{
	if(ss.is_open())
		ss.close();
}

bool RemoteIrcServer::send_unlogged(const str& cmd)
{
	lock_guard lock(mtx_ss);
//		ss << cmd.substr(0, 510) << "\r\n" << std::flush;
	boost::system::error_code ec;

	auto const buf = cmd + "\r\n";
	auto pos = buf.data();
	auto end = pos + buf.size();
	while((pos += ss.write_some(asio::buffer(pos, end - pos), ec)) < end)
		if(ec)
			return false;

	return true;
}

bool RemoteIrcServer::receive(str& line)
{
	line.clear();

	std::array<char, 1024> buf;
	boost::system::error_code ec;

	while(auto len = ss.read_some(asio::buffer(buf), ec))
	{
		line.append(buf.data(), len);
		if(len < buf.size())
			break;
	}

	if(ec)
	{
		LOG::E << ec.message();
		return false;
	}

	return true;
}

}} // skivvy::ircbot
