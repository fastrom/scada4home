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


#include "RASManager.h"
#include "Pop3Client.h"
#include "SharedUtils.h"
#include "ItemRepository.h"
#include <sys/time.h> 
#include <sstream>


RASManager::RASManager(ItemRepository *argItemRepo,IRASEventSubscriber *argEventSubsciber,LogTracer *argLogger)
{
  _EventSubscriber = argEventSubsciber;
  _Logger = argLogger;
  _ItemRepo = argItemRepo;
  //_POP3Client = new Pop3Client("mail.bereuter.com",110);
}

RASManager::~RASManager()
{

}



bool RASManager::Start()
{
  _Logger->Trace("Starting RASManager...");  
      
  pthread_create( &_ProcessingThread, NULL, LaunchMemberFunction,this); // create a thread running function1
    
  return true;

}

void* RASManager::ProcessingLoop()
{  
    timeval nowTime;
    while(1)
    { 		
	gettimeofday(&nowTime,NULL);
	long diffSec = nowTime.tv_sec - _LastPop3Fetch.tv_sec;
        long diffUsec = nowTime.tv_usec - _LastPop3Fetch.tv_usec;
	if(diffSec > 10 )
	{	 
	  _Logger->Trace("Fetching POP3 Server...");	  
	  _LastPop3Fetch = nowTime;
	  FetchPop3Mails();
	}
					
	usleep(1000000);
	
    }
    return 0;

}

void RASManager::FetchPop3Mails()
{
  try
  {    
    Pop3Client client = Pop3Client("mail.bereuter.com",_Logger, 110);
    client.login("home@bereuter.com","Quattro1");
    client.setShortMessage(true);   
    vector<int> msgIdList;
    client.listMails(msgIdList);
   
    for(vector<int>::iterator iter = msgIdList.begin(); iter!=msgIdList.end(); ++iter)
    {
      int curId = (*iter);
      Email curMail;
      bool rcvOK = client.FetchMail(curId,curMail);
      if(rcvOK)
      {
	AnalyzeMail(curMail);		  
      }
    }
    
    client.quit();
    
   
  }
  catch (const char *e) 
  {
    cerr << "POP3 client failed: " << e << endl;
    return ;
  }
  catch (...)
  {
    cerr << "An unknown error occured, quitting..." << endl;
    return ;
  }  
}

void RASManager::AnalyzeMail(Email argMail)
{
    
  if(argMail.Subject.find("scada4home") < 0 )
  {
    HandleAnalyzeError(argMail,"Unknown Mail Subject received " + argMail.Subject);
    return;
  }    
  
  vector<string> msgLines;
  SharedUtils::Tokenize(argMail.BodyText,msgLines,"\r\n");	
  for(vector<string>::iterator iter = msgLines.begin();iter != msgLines.end();++iter)
  {      
    string line = (*iter);
    string marker = "##";
    int endPos = line.find(marker,marker.length());
    int startPos = line.find(marker);
    if(startPos < 0 || endPos <= startPos)
      continue;      
    
    string strCommand = line.substr(startPos+marker.length(),endPos - startPos - marker.length());
    vector<string> cmdTokens;
    SharedUtils::Tokenize(strCommand,cmdTokens,"|");
    if(cmdTokens.size() == 0)
    {
      HandleAnalyzeError(argMail,"Received empty command " + line);	
      continue;
    }
    
    if(cmdTokens.size() == 1)
    {
      string strOperator = cmdTokens[0];
      if(strOperator == "HELP")
      {
	SendHelp(argMail);
      }
      else
	HandleAnalyzeError(argMail,"Operators not yet implemented " + line);
    }
    else 
    {
      string actionType = cmdTokens[0]; 
      string strItem = cmdTokens[1];
      
      ScadaItem *item = _ItemRepo->GetItem(strItem);
      if(item != NULL)
      {
	if(actionType == "GET")
	{
	  ItemUpdateMessage msg;
	  msg.MsgType = ItemMessageTypes::StatusRequest;
	  msg.ItemType = item->ItemType;
	  msg.ItemIndex = item->Index;
	  msg.Property = ItemProperties::Position;
	  
	  _EventSubscriber->RASMessageReceived(msg);
	  
	  //Somebody updates and the Repo and waits meanwhile
	  
	  stringstream sstream;
	  sstream << " Value=" << item->Value;
	  SendMail(argMail.FromAddr,"Re:" + argMail.Subject + "(GET Result)","Item " + item->Name + sstream.str());
	}
	else if(actionType == "SET")
	{
	  if(cmdTokens.size() < 3)
	  {
	    HandleAnalyzeError(argMail,"Received command not valid for a SET Action, no Value parameter provided " + strCommand);	
	    continue;
	  }
	  
	  string strValue = cmdTokens[2];	    
	  
	  ItemUpdateMessage msg;
	  msg.MsgType = ItemMessageTypes::Command;
	  msg.ItemType = item->ItemType;
	  msg.ItemIndex = item->Index;
	  msg.Property = ItemProperties::Value;
	  msg.Value = SharedUtils::ConvertToItemValue(strValue,item->ItemType);
      
	  _EventSubscriber->RASMessageReceived(msg);
	  SendMail(argMail.FromAddr,"Re:" + argMail.Subject + "(SET Result)","Command " + strCommand +" processed successfully");
	}
	else
	  HandleAnalyzeError(argMail,"Received command not valid for a SET Action, no Value parameter provided " + strCommand);	
	  
      }
      else
	HandleAnalyzeError(argMail,"Received command for unknown Item " + strCommand);	
    }  
  }
  
}

void RASManager::SendHelp(Email argMail)
{
  string helpText = "Command synopsis:\r\n";
  helpText.append("##SET|itemname|value##\r\n");
  helpText.append("##GET|itemname##\r\n");
  helpText.append("##HELP##\r\n");
  helpText.append("\r\n");
  helpText.append("Available Items:\r\n");
  
  vector<ScadaItem*> itemList = _ItemRepo->GetItems();
  for(vector<ScadaItem*>::iterator iter=itemList.begin();iter != itemList.end();++iter)
  {
    
    helpText.append((*iter)->Name + "\r\n");      
  }    
  
  SendMail(argMail.FromAddr,"Re:" + argMail.Subject + " (HELP)",helpText);  
  
}


void RASManager::HandleAnalyzeError(Email argMail, string argText)
{
  _Logger->Trace(argText);
  SendMail(argMail.FromAddr,"Re:" + argMail.Subject + " (ERR)",argText);  
  
}



void RASManager::SendMail(string argReceiverEmail,string argSubject,string argBodyText)
{
  SMTPClient client;
  client.SendMail(argReceiverEmail,argSubject,argBodyText); 
  
}

void RASManager::Stop()
{
  
  
}
