/**
		Copyright (C) 2012 Nils Weiss, Patrick Bruenn.

    This file is part of Wifly_Light.

    Wifly_Light is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Wifly_Light is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Wifly_Light.  If not, see <http://www.gnu.org/licenses/>. */

#include "ClientSocket.h"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>

#include <stdio.h>

ClientSocket::ClientSocket(const char* pAddr, short port, int style)
	: mSock(socket(AF_INET, style, 0))
{
	memset(&mSockAddr, 0, sizeof(mSockAddr));
	mSockAddr.sin_family = AF_INET;
	mSockAddr.sin_port = htons(port);
	inet_pton(AF_INET, pAddr, &(mSockAddr.sin_addr));
}

ClientSocket::~ClientSocket()
{
#ifndef ANDROID
	close(mSock);
#endif
}


TcpSocket::TcpSocket(const char* pAddr, short port)
	: ClientSocket(pAddr, port, SOCK_STREAM)
{
	if(connect(mSock, reinterpret_cast<sockaddr*>(&mSockAddr), sizeof(mSockAddr)) < 0)
	{
		std::cout << "Connection to " << pAddr << ":" << port << " failed" << std::endl;
	}
}

int TcpSocket::Recv(unsigned char* pBuffer, size_t length) const
{
	return recv(mSock, pBuffer, length, 0);
}

int TcpSocket::Send(unsigned char* frame, size_t length) const
{
	return send(mSock, frame, length, 0);
}

UdpSocket::UdpSocket(const char* pAddr, short port)
	: ClientSocket(pAddr, port, SOCK_DGRAM)
{
}

int UdpSocket::Recv(unsigned char* pBuffer, size_t length) const
{
	std::cout << __FILE__ << ":" << __LINE__ << " Not implemented" << std::endl;
	return -1;
}

int UdpSocket::Send(unsigned char* frame, size_t length) const
{
	return sendto(mSock, frame, length, 0, (struct sockaddr*)&mSockAddr, sizeof(mSockAddr));
}

