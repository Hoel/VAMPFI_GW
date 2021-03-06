/* Copyright (C) 2018  GridRF Radio Team(tech@gridrf.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef __PACKETFORWARDER_H__
#define __PACKETFORWARDER_H__

#include <stdint.h>
#include <pthread.h>
#include "Base64.h"
#include "board.h"

#ifdef _WIN32
#include <WinSock2.h>
#include <Windows.h>
#else
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include "TimerHandler.h"
#include "TimerEvent.h"
#include "IMessager.h"
#include "IMessageSender.h"


#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

class PacketForwarder:public TimerHandler, public IMessager
{
private:
    uint16_t _port;
	struct sockaddr_in ser_sin;
	bool isRunning;
	int socket_fd;
	pthread_t _sockThread;
	unsigned long long mac;
	IMessageSender *msgSender;
	LoRa_Config *_conf;

	TimerEvent_t heatTimeoutTimer;

private:
	void OnSockRead(uint8_t *buffer, int size, struct sockaddr_in *from_addr);

public:
	PacketForwarder(LoRa_Config *conf);
	~PacketForwarder();

	int Init(TimerEvent *timer, IMessageSender *sender);
	void OnPacket(uint16_t token, uint8_t *buffer, int size);
	int Send(uint8_t *buffer, int size);
	void Stop();
	void Run();

	bool OnTimeoutIrq(void *user);
};

#endif

