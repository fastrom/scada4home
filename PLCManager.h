/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef PLCMANAGER_H
#define PLCMANAGER_H

#include "/usr/local/include/modbus/modbus.h"
#include <pthread.h>
#include <errno.h>
#include "LogTracer.h"
#include "SharedTypes.h"
#include <queue>

 


class PLCManager
{
  private:
     modbus_t *_ModbusProxy;
     string _DeviceIpAddress;
     LogTracer *_Logger;
     pthread_mutex_t _ModBusMutex;
     pthread_mutex_t _SendQueueMutex;
     uint16_t _MsgWriteIndex;
     int16_t _MsgReadindex;
     timeval _LastSendTime;
     timeval _LastAlivePing;
     pthread_t _ProcessingThread; 
     IPLCEventSubscriber *_EventSubscriber;
     queue<ScadaItemMessage> _SendQueue;
     bool ReadPLCMessages();
     bool WritePLCMessages();
     bool OpenModBus();
     bool InitPLC();
     bool CloseModBus();
     void IncrementWritePos();   
     bool WriteMessage(ItemMessageTypes::T argMsgType, ItemTypes::T argSrc,uint8_t argSrcIdx,ItemProperties::T,uint16_t argValue);
     static void *LaunchMemberFunction(void *obj)
     {
	PLCManager *targetObj = reinterpret_cast<PLCManager *>(obj);
	return targetObj->ProcessingLoop();
     }

     
     
     
  public:
     PLCManager(IPLCEventSubscriber *argEventSubsciber,string argIpAddress,LogTracer *argLogger);
     ~PLCManager();
     void Send(ScadaItemMessage argMsg);     
     bool Start();
     void Stop();
     void * ProcessingLoop();
};

#endif // PLCMANAGER_H
