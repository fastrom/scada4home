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


#ifndef LOGTRACER_H
#define LOGTRACER_H

#include <iostream>

using namespace std;

class LogTracer
{
private:
    bool _LogVerbose;
    pthread_mutex_t _LogMutex;

  public:
    LogTracer();
    virtual ~LogTracer();
    void Trace(string argText);
    void Trace(string argText1, int argText2);
    void Trace(string argText1, string argText2);
    void Trace(string argText1, int argText2,bool argHighLevel);
    void Trace(string argText1, int argText2,int argText3);
    void SetLogLevel(bool argHigh);
  
};

#endif // LOGTRACER_H