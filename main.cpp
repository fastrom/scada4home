#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdlib.h>
#include "ControlManager.h"

typedef void (*sighandler_t)(int);
static sighandler_t handle_signal (int sig_nr, sighandler_t signalhandler) 
{
   struct sigaction neu_sig, alt_sig;
   neu_sig.sa_handler = signalhandler;
   sigemptyset (&neu_sig.sa_mask);
   neu_sig.sa_flags = SA_RESTART;
   if (sigaction (sig_nr, &neu_sig, &alt_sig) < 0)
      return SIG_ERR;
   return alt_sig.sa_handler;
}

void daemonizeMe(const char *log_name, int facility)
{
  int i;
   pid_t pid;
   /* Elternprozess beenden, somit haben wir einen Waisen */
   /* dessen sich jetzt vorerst init annimmt              */
   if ((pid = fork ()) != 0)
      exit (EXIT_FAILURE);
   /* Kindprozess wird zum Sessionführer. Misslingt */
   /* dies, kann der Fehler daran liegen, dass der     */
   /* Prozess bereits Sessionführer ist               */
   if (setsid() < 0) {
      printf("%s kann nicht Sessionführer werden!\n",log_name);
      exit (EXIT_FAILURE);
   }
   /* Signal SIGHUP ignorieren */
   handle_signal (SIGHUP, SIG_IGN);
   /* Oder einfach: signal(SIGHUP, SIG_IGN) ... */
   /* Das Kind terminieren */
   if ((pid = fork ()) != 0)
      exit (EXIT_FAILURE);
   /* Gründe für das Arbeitsverzeichnis:
    * + core-Datei wird im aktuellen Arbeitsverzeichnis 
    *   hinterlegt.
    * + Damit bei Beendigung mit umount das Dateisystem 
    *   sicher abgehängt werden kann 
    */
   //chdir ("/");
   /* Damit wir nicht die Bitmaske vom Elternprozess     */
   /* erben bzw. diese bleibt, stellen wir diese auf 0  */
   umask (0);
   /* Wir schließen alle geöffneten Filedeskriptoren ... */
   for (i = sysconf (_SC_OPEN_MAX); i > 0; i--)
      close (i);
   /* Da unser Dämonprozess selbst kein Terminal für */
   /* die Ausgabe hat....                            */
   openlog ( log_name, LOG_PID | LOG_CONS| LOG_NDELAY, facility );
  
}

//BESI
int main(int argc, char **argv) {
    std::cout << "Starting scada4home..." << std::endl;
    
    bool runAsDaemon = false;
    if(argc > 1)
    {
      for(int i=0;i<argc;i++)
      {
	if(strcmp(argv[i],"daemon"))
	{
	  runAsDaemon = true;
	  break;
	}	  
      }
    }
    
    if(runAsDaemon)
    {
      std::cout << "Initializing daemon..." << std::endl;
      daemonizeMe("scada4home", LOG_LOCAL0);
    }
    
    //TODO: An Logtracer et.al. übergeben    
    ControlManager *cMan = new ControlManager();
    cMan->Start();
    
     while(true)
    {    
      usleep(1000000);
    }
    
    cMan->Stop();
    
    return 0;
}


