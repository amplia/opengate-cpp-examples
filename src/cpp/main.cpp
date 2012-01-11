//
// main.cpp
//

#include <WipReactor.h>
#ifndef WIN32
  #include <PWatchDogApi.h>
  #include <PWatchDogServer.h>
#endif
#include "Agent.h"

/**
 * <p>
 * <b>OpenGate M2M/Mobility Communications Platform</b>
 * </p>
 * <p>
 * <i>Wireless your bussiness</i>
 * </p>
 * <p>
 * <b>Description:</b><br/>
 * Connectivity agent main class.
 *
 * </p>
 * <p>
 * If you want more info about this example please send an email to
 * <a href="mailto:support@amplia.es">Amplia Support</a>
 * <p>
 * Copyright 2007 Amplia Soluciones S.L. All rights reserved.
 *
 * @author Amplia Soluciones S.L
 */

WipReactor *reactor = NULL;

int main(int argc, char *argv[])
{
	
    if(argc < 3) {
      printf("Uso: %s %s",WipString(argv[0]).ptr(),"<config_path> <temp_path>");
	    exit(-1);
    }
	
	WipLogger::GetInstance().SetEnableSystemOut(true);
	WipLogger::GetInstance().SetEnabledLevel(WipLogger::LOGLEVEL_DEBUG);
	WipLogger::GetInstance().SetEnableAppendLog(true);

	// Creates a new instance of the WipReactor class. Used inside
	// the OpenGate library. Controls the application lifecycle.
	reactor = new WipReactor;

	WipString config_path(argv[1]);
  WipString temp_path(argv[2]);

	// OJO si lo pongo aqui se corrmpe la memoria en el constructor del Agente
	// WipReactor reactor;

    WipLogger::GetInstance().SetEnableSystemOut(true);
	
	// Inicio monitorización con PWatchDog
	#ifndef WIN32
	if((argc == 4) && (WipString(argv[3]) == "pwdog")) {
	  // Arranco el Servidor ProcessWatchDog
	  try{
	    WipLogger::GetInstance().SetEnableSystemOut(true);
	    PWatchDogServer pwdServer(reactor);
	    pwdServer.Initialize(31000,config_path);
	    while(true) reactor->HandleEvents(NULL);
	  } catch (const OGException& ex) {
	    WIP_LOG_ERROR(ex.GetMessage());
	  }
	  exit(0);
	} else {
	  WIP_LOG_INFO("Arranco Cliente ProcessWatchDog")
	  // Arranco el Cliente ProcessWatchDog
	  PWatchDogApi::Initialize(reactor,"127.0.0.1",31000,"MCMain",2000,config_path);
	}
	#endif
	
	
	WIP_LOG_INFO("");
	WIP_LOG_INFO(" -= Domocell Concentrator Application =-");
	WIP_LOG_INFO("");

	// Instantiates the mainstay class. 
	// \see Agent.cpp.
	Agent *agent = new Agent(reactor,config_path,temp_path);

	try
	{
		// Performs basic configuration.
		// \param selectedCarriers Enabled connectors.
		// \param "ogm2m.cfg" Configuration file. Provided with this
		// example. See directory ./../conf/.
		agent->LoadConfiguration();

		// Initializes agent action.
		agent->Initialize();

		// Application main loop. While the agent is running (until a
		// CTRL+C - SIGINT signal) the reactor iterates over any generated event,
		// processing and executing it.
		while (agent->IsRunning())
		{
			WipTimeValue tv(1,0);
			reactor->HandleEvents(&tv);
		}

		// When CTRL+C - SIGINT signal is received the agent finishes.
		agent->Finalize();
	}
	catch(OGException e)
	{
		WIP_LOG_ERROR(OGErrorConstants::ErrorCodeToStr(e.GetErrorCode())+": "+e.GetMessage());
	}

	if (agent)
		delete agent;

	if (reactor)
	{
		// Finish and releases the reactor instance.
		reactor->Terminate();
		delete reactor;
	}
	return 0;
}
