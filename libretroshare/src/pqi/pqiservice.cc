/*
 * libretroshare/src/pqi pqiservice.cc
 *
 * 3P/PQI network interface for RetroShare.
 *
 * Copyright 2004-2008 by Robert Fernie.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License Version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems to "retroshare@lunamutt.com".
 *
 */

#include "pqi/pqiservice.h"
#include "pqi/pqidebug.h"
#include <sstream>

const int pqiservicezone = 60478;

p3ServiceServer::p3ServiceServer()
{
	pqioutput(PQL_DEBUG_BASIC, pqiservicezone, 
		"p3ServiceServer::p3ServiceServer()");

        rrit = services.begin();
	return;
}

int	p3ServiceServer::addService(pqiService *ts)
{
	pqioutput(PQL_DEBUG_BASIC, pqiservicezone, 
		"p3ServiceServer::addService()");

	std::map<int, pqiService *>::iterator it;
	it = services.find(ts -> getType());
	if (it != services.end())
	{
		// it exists already!
		return -1;
	}

	services[ts -> getType()] = ts;
        rrit = services.begin();
	return 1;
}

int	p3ServiceServer::incoming(RsRawItem *item)
{
	pqioutput(PQL_DEBUG_BASIC, pqiservicezone, 
		"p3ServiceServer::incoming()");

	{
		std::ostringstream out;
		out << "p3ServiceServer::incoming() PacketId: ";
		out << std::hex << item -> PacketId() << std::endl;
		out << "Looking for Service: ";
		out << (item -> PacketId() & 0xffffff00) << std::dec << std::endl;

		out << "Item:" << std::endl;
		item -> print(out);
		out << std::endl;
		pqioutput(PQL_DEBUG_BASIC, pqiservicezone, out.str());
	}

	std::map<int, pqiService *>::iterator it;
	it = services.find(item -> PacketId() & 0xffffff00);
	if (it == services.end())
	{
		pqioutput(PQL_DEBUG_BASIC, pqiservicezone, 
		"p3ServiceServer::incoming() Service: No Service - deleting");

		// delete it.
		delete item;

		// it exists already!
		return -1;
	}

	{
		std::ostringstream out;
		out << "p3ServiceServer::incoming() Sending to";
		out << it -> second << std::endl;
		pqioutput(PQL_DEBUG_BASIC, pqiservicezone, out.str());

		return (it->second) -> receive(item);
	}

	delete item;
	return -1;
}



RsRawItem *p3ServiceServer::outgoing()
{
	pqioutput(PQL_DEBUG_ALL, pqiservicezone, 
		"p3ServiceServer::outgoing()");

	if (rrit != services.end())
	{
		rrit++;
	}
	else
	{
		rrit = services.begin();
	}

	std::map<int, pqiService *>::iterator sit = rrit;
	// run to the end.
	RsRawItem *item;

	// run through to the end,
	for(;rrit != services.end();rrit++)
	{
		if (NULL != (item = (rrit -> second) -> send()))
		{
			std::ostringstream out;
			out << "p3ServiceServer::outgoing() Got Item From:";
			out << rrit -> second << std::endl;

			item -> print(out);
			out << std::endl;
			pqioutput(PQL_DEBUG_BASIC, pqiservicezone, out.str());
			return item;
		}
	}

	// from the beginning to where we started.
	for(rrit = services.begin();rrit != sit; rrit++)
	{
		if (NULL != (item = (rrit -> second) -> send()))
		{
			std::ostringstream out;
			out << "p3ServiceServer::outgoing() Got Item From:";
			out << rrit -> second << std::endl;

			item -> print(out);
			out << std::endl;
			pqioutput(PQL_DEBUG_BASIC, pqiservicezone, out.str());
			return item;
		}
	}
	return NULL;
}



int	p3ServiceServer::tick()
{
	pqioutput(PQL_DEBUG_ALL, pqiservicezone, 
		"p3ServiceServer::tick()");

	std::map<int, pqiService *>::iterator it;

	// from the beginning to where we started.
	for(it = services.begin();it != services.end(); it++)
	{
		std::ostringstream out;
		out << "p3ServiceServer::service id:" << it -> first;
		out << " -> Service: " << it -> second;
		out << std::endl;
		pqioutput(PQL_DEBUG_ALL, pqiservicezone, out.str());

		// now we should actually tick the service.
		(it -> second) -> tick();
	}
	return 1;
}



