#ifndef SOOKEE_AI_GRAMMAR_H
#define SOOKEE_AI_GRAMMAR_H

/*
 * irc.h
 *
 *  Created on: 10 Jan 2012
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

#include <array>
#include <string>

namespace skivvy { namespace types {

using str = std::string;

}} // skivvy::types

namespace skivvy { namespace ai {

using namespace types;

enum class NounType
{
	PROPER
	, COMMON
	, COUNT
	, MASS
	, COLLECTIVE
	, CONCRETE
	, ABSTRACT
	, PRONOUN
};

struct noun
{
	str word;
};

class Person
{
protected:
	const std::string name;
};

class AI
: public Person
{

};


}} // sookee::ai

#endif // SOOKEE_AI_GRAMMAR_H
