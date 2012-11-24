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


#include "LogTracer.h"

LogTracer::LogTracer()
{
  _LogMutex = PTHREAD_MUTEX_INITIALIZER;
}

LogTracer::~LogTracer()
{

}

void LogTracer::Trace(string argText)
{
  if(!_LogVerbose)
    return;
   pthread_mutex_lock( &_LogMutex );
  cout <<  argText << endl;  
   pthread_mutex_unlock( &_LogMutex );
}
void LogTracer::Trace(string argText1, string argText2)
{
  if(!_LogVerbose)
    return;
   pthread_mutex_lock( &_LogMutex );
  cout <<  argText1 << argText2 << endl;  
   pthread_mutex_unlock( &_LogMutex );
}

void LogTracer::Trace(string argText1, int argText2)
{
  if(!_LogVerbose)
    return;
    
   pthread_mutex_lock( &_LogMutex );
  cout <<  argText1 << argText2 << endl;  
   pthread_mutex_unlock( &_LogMutex );
}

void LogTracer::Trace(string argText1, int argText2,bool argHighLevel)
{
  if(!(_LogVerbose || argHighLevel))
    return;
    
   pthread_mutex_lock( &_LogMutex );
  cout <<  argText1 << argText2 << endl;  
   pthread_mutex_unlock( &_LogMutex );
}

void LogTracer::Trace(string argText1, int argText2,int argText3)
{
    if(!_LogVerbose)
    return;
    
   pthread_mutex_lock( &_LogMutex );
  cout <<  argText1 << argText2 <<  " ; " << argText3 << endl;  
   pthread_mutex_unlock( &_LogMutex );
}

void LogTracer::SetLogLevel(bool argHigh)
{
  _LogVerbose = argHigh;
}
