#ifndef _SKIVVY_SOCKETSTREAM_H_
#define _SKIVVY_SOCKETSTREAM_H_

/*-----------------------------------------------------------------.
| Copyright (C) 2011 SooKee oaskivvy@gmail.com               |
'------------------------------------------------------------------'

This code is was created from code (C) Copyright Nicolai M. Josuttis 2001
with the following Copyright Notice:
*/

/* The following code declares classes to read from and write to
 * file descriptore or file handles.
 *
 * See
 *      http://www.josuttis.com/cppcode
 * for details and the latest version.
 *
 * - open:
 *      - integrating BUFSIZ on some systems?
 *      - optimized reading of multiple characters
 *      - stream for reading AND writing
 *      - i18n
 *
 * (C) Copyright Nicolai M. Josuttis 2001.
 * Permission to copy, use, modify, sell and distribute this software
 * is granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied
 * warranty, and with no claim as to its suitability for any purpose.
 *
 * Version: Jul 28, 2002
 * History:
 *  Jul 28, 2002: bugfix memcpy() => memmove()
 *                fdinbuf::underflow(): cast for return statements
 *  Aug 05, 2001: first public version
 */

 /*

Permission to copy, use, modify, sell and distribute this software
is granted under the same conditions.

'----------------------------------------------------------------*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <streambuf>
#include <istream>
#include <ostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring> // strerror()
#include <cerrno>

#include <sookee/log.h>
#include <sookee/bug.h>

//#include <skivvy/logrep.h>

namespace skivvy { namespace net {

//using namespace skivvy::utils;
using namespace sookee::bug;
using namespace sookee::log;

template<typename Char>
class basic_socketbuf
: public std::basic_streambuf<Char>
{
public:
	typedef Char char_type;
	typedef std::basic_streambuf<char_type> buf_type;
	typedef std::basic_ostream<char_type> stream_type;
	typedef typename buf_type::int_type int_type;
	typedef typename std::basic_streambuf<Char>::traits_type traits_type;

protected:

	static const int char_size = sizeof(char_type);
	static const int SIZE = 128;

	int sock;
	char_type* ibuf;//[SIZE] ;
	char_type* obuf;//[SIZE] ;


public:
	basic_socketbuf()
	: sock(0)
	, ibuf(0)
	, obuf(0)
	{
		ibuf = new char_type[SIZE];
		obuf = new char_type[SIZE];
		buf_type::setp(obuf, obuf + (SIZE - 1));
		buf_type::setg(ibuf, ibuf, ibuf);
	}

	virtual ~basic_socketbuf()
	{
		sync();
		delete[] ibuf;
		delete[] obuf;
	}

	void set_socket(int sock) { this->sock = sock; }
	int get_socket() { return this->sock; }

protected:

	int output_buffer()
	{
		int num = buf_type::pptr() - buf_type::pbase();
		if(send(sock, reinterpret_cast<char*>(obuf), num * char_size, 0) != num)
			return traits_type::eof();
		buf_type::pbump(-num);
		return num;
	}

	virtual int_type overflow(int_type c)
	{
		if(c != traits_type::eof())
		{
			*buf_type::pptr() = c;
			buf_type::pbump(1);
		}

		if(output_buffer() == traits_type::eof())
			return traits_type::eof();
		return c;
	}

	virtual int sync()
	{
		if(output_buffer() == traits_type::eof())
			return traits_type::eof();
		return 0;
	}

	virtual int_type underflow()
	{
		if(buf_type::gptr() < buf_type::egptr())
			return *buf_type::gptr();

		int num;
		if((num = recv(sock, reinterpret_cast<char*>(ibuf), SIZE * char_size, 0)) <= 0)
			return traits_type::eof();

		buf_type::setg(ibuf, ibuf, ibuf + num);
		return *buf_type::gptr();
	}
};

typedef basic_socketbuf<char> socketbuf;
typedef basic_socketbuf<wchar_t> wsocketbuf;

template<typename Char>
class basic_socketstream
: public std::basic_iostream<Char>
{
public:
	typedef Char char_type;
	typedef std::basic_iostream<char_type> stream_type;
	typedef basic_socketbuf<char_type> buf_type;

protected:
	buf_type buf;

public:
	basic_socketstream(): stream_type(&buf) {}
	basic_socketstream(int s): stream_type(&buf) { buf.set_socket(s); }

	virtual ~basic_socketstream()
	{
		close();
	}

	void close()
	{
//		bug_func();
//		bug_var(buf.get_socket());
		if(buf.get_socket() != 0) ::close(buf.get_socket());
		stream_type::clear();
	}

	bool open(const std::string& host, uint16_t port, int type = SOCK_STREAM, bool nb = false)
	{
//		bug_func();
		close();

		int sd;
		hostent* he;

		if((sd = socket(PF_INET, type, 0)) == -1)
		{
			log(std::strerror(errno));
			stream_type::setstate(std::ios::failbit);
			return false;
		}
		bug_var(sd);

		if(!(he = gethostbyname(host.c_str())))
		{
			::close(sd);
			stream_type::setstate(std::ios::failbit);
			return false;
		}

		sockaddr_in sin;
		std::copy(reinterpret_cast<char*>(he->h_addr)
			, reinterpret_cast<char*>(he->h_addr) + he->h_length
			, reinterpret_cast<char*>(&sin.sin_addr.s_addr));

		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sin.sin_addr = *((in_addr*) he->h_addr);

		if(connect(sd, reinterpret_cast<sockaddr*>(&sin), sizeof(sin)) < 0)
		{
			::close(sd);
			stream_type::setstate(std::ios::failbit);
			return false;
		}

		if(nb)
			fcntl(sd, F_SETFL, O_NONBLOCK);
		buf.set_socket(sd);

		return *this;
	}
};

typedef basic_socketstream<char> socketstream;
typedef basic_socketstream<wchar_t> wsocketstream;

}} // skivvy::net

#endif // _SKIVVY_SOCKETSTREAM_H_
