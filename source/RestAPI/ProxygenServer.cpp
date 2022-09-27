/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Memory.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/portability/GFlags.h>
#include <folly/portability/Unistd.h>

//#ifdef TEMPORARY_COMMENTED
#include <proxygen/httpserver/HTTPServer.h>
#include <proxygen/httpserver/RequestHandlerFactory.h>

#include "ProxygenHandler.h"
#include "ProxygenStats.h"

#include "EQF.H"
#include "OtmMemoryServiceWorker.h"
#include "../opentm2/core/utilities/LogWrapper.h"
#include "../opentm2/core/utilities/PropertyWrapper.H"
#include "../opentm2/core/utilities/FilesystemWrapper.h"
#include "config.h"

using namespace ProxygenService;
using namespace proxygen;

using folly::SocketAddress;

using Protocol = HTTPServer::Protocol;

/*
  // handler for resource URL w/o memory name
    auto resource = make_shared< Resource >();
    snprintf( szValue, 150, "/%s", szServiceName );
    resource->set_path( szValue );
    resource->set_method_handler( "GET", get_method_handler );
    resource->set_method_handler( "POST", post_method_handler );

    // handler for resource URL w memory name/import
    auto tagReplacement = make_shared< Resource >();
    snprintf( szValue, 150, "/%s_service/tagreplacement", szServiceName );
    tagReplacement->set_path( szValue );
    tagReplacement->set_method_handler( "POST", postTagReplacement_method_handler );
  

    // handler for resource URL w memory name/import
    auto memImport = make_shared< Resource >();
    snprintf( szValue, 150, "/%s/{id: .+}/import", szServiceName );
    memImport->set_path( szValue );
    memImport->set_method_handler( "POST", postImport_method_handler );

    // handler for resource URL w memory name/fuzzysearch
    auto fuzzysearch = make_shared< Resource >();
    snprintf( szValue, 150, "/%s/{id: .+}/fuzzysearch", szServiceName );
    fuzzysearch->set_path( szValue );
    fuzzysearch->set_method_handler( "POST", postFuzzySearch_method_handler );

    // handler for resource URL w memory name/concordancesearch
    auto concordancesearch = make_shared< Resource >();
    snprintf( szValue, 150, "/%s/{id: .+}/concordancesearch", szServiceName );
    concordancesearch->set_path( szValue );
    concordancesearch->set_method_handler( "POST", postConcordanceSearch_method_handler );

    // handler for resource URL w memory name/entry
    auto postEntry = make_shared< Resource >();
    snprintf( szValue, 150, "/%s/{id: .+}/entry", szServiceName );
    postEntry->set_path( szValue );
    postEntry->set_method_handler( "POST", postEntry_method_handler );

    // handler for resource URL w memory name/entry
    auto postEntryDelete = make_shared< Resource >();
    snprintf( szValue, 150, "/%s/{id: .+}/entrydelete", szServiceName );
    postEntryDelete->set_path( szValue );
    postEntryDelete->set_method_handler( "POST", postEntryDelete_method_handler );

    // handler for resource URL w memory name
    auto memname = make_shared< Resource >();
    snprintf( szValue, 150, "/%s/{id: .+}", szServiceName );
    memname->set_path( szValue );
    memname->set_method_handler( "DELETE", delete_method_handler );
    memname->set_method_handler( "GET", get_memory_method_handler );

    // handler for resource URL w memory name/status
    auto getStatus = make_shared< Resource >();
    snprintf( szValue, 150, "/%s/{id: .+}/status", szServiceName );
    getStatus->set_path( szValue );
    getStatus->set_method_handler( "GET", getStatus_method_handler );

    // handler for resource URL service/shutdown
    auto shutdownService = make_shared< Resource >();
    snprintf( szValue, 150, "/%s_service/shutdown", szServiceName );
    shutdownService->set_path( szValue );
    shutdownService->set_method_handler( "GET", shutdownService_method_handler );

    // handler for resource service/save all tms-    auto saveTms = make_shared< Resource >();
    snprintf( szValue, 150, "/%s_service/savetms", szServiceName );
    saveTms->set_path( szValue );
    saveTms->set_method_handler( "GET", saveAllOpenedTMService_method_handler );

    pSettings = make_shared< Settings >();
    pSettings->set_port( (uint16_t)uiPort );
    pSettings->set_worker_limit( uiWorkerThreads );
    pSettings->set_default_header( "Connection", "close" );
    pSettings->set_connection_timeout( std::chrono::seconds( uiTimeOut ) );
    //*/

// replace plus signs in string with blanks
void restoreBlanks( std::string &strInOut )
{
  int iLen = strInOut.length();
  for ( int i = 0; i < iLen; i++ )
  {
    if ( strInOut[i] == '+' )
    {
      strInOut[i] = ' ';
    }
  }
}

class ProxygenHandlerFactory;

HTTPServerOptions setup_proxygen_servers_options(int threads, uint32_t timeout, uint32_t  initialReceiveWindow, uint32_t receiveStreamWindowSize, uint32_t receiveSesionWindowsSize ){
  
  if (threads <= 0) {
    threads = sysconf(_SC_NPROCESSORS_ONLN);
    //CHECK(threads > 0);
  }

  HTTPServerOptions options;
  options.threads = static_cast<size_t>(threads);
  options.idleTimeout = std::chrono::milliseconds(timeout);
  options.shutdownOn = {SIGINT, SIGTERM};
  options.enableContentCompression = false;
  options.handlerFactories =
      RequestHandlerChain().addThen<ProxygenHandlerFactory>().build();
  // Increase the default flow control to 1MB/10MB
  options.initialReceiveWindow = initialReceiveWindow ;
  options.receiveStreamWindowSize = receiveStreamWindowSize ;
  options.receiveSessionWindowSize = receiveSesionWindowsSize ;
  //options.h2cEnabled = true;
  options.h2cEnabled = false;
  options.supportsConnect = false;

  return options;
}

  std::string serviceName;
  std::string additionalServiceName;
class ProxygenHandlerFactory : public RequestHandlerFactory {
 public:
  void onServerStart(folly::EventBase* /*evb*/) noexcept override {
    stats_.reset(new ProxygenStats);

    }

  void onServerStop() noexcept override {
    stats_.reset();
  }


  RequestHandler* onRequest(RequestHandler* handler, HTTPMessage* message) noexcept override {
    auto methodStr = message->getMethodString ();
    auto method = message->getMethod ();
    auto url = message->getURL () ;
    auto path = message->getPath () ;
    auto queryString   = message->getQueryString () ;
    auto headers = message->getHeaders();
    auto queryParams = message->getQueryParams();
    //message->get
    //ParseURL parseUrl(url);
    //auto query = parseUrl.query();
    //auto host = parseUrl.host(); 
    // No need to strictly verify the URL when reparsing it
    //auto u = ParseURL::parseURL(url, /*strict=*/false);
    if(url.size() <= 1){
      return  new ProxygenHandler(stats_.get());;
    }

    std::transform(url.begin(), url.end(), url.begin(),
    [](unsigned char c){ return std::tolower(c); });

    if(url[0] == '/'){
      url = url.substr(1, url.size() - 1);
    }

    auto urlSeparator = url.find("/");

    if( urlSeparator == -1 ){
      return  new ProxygenHandler(stats_.get());
    }

    std::string urlService = url.substr(0, urlSeparator);
   
    if(urlService != serviceName && urlService != additionalServiceName ){
      return new ProxygenHandler(stats_.get());
    }

    auto requestHandler = new ProxygenHandler(stats_.get());

    requestHandler->command = ProxygenHandler::COMMAND::UNKNOWN_COMMAND;

    url = url.substr(urlSeparator + 1);
    std::string urlMemName, urlCommand;
    if( urlService == additionalServiceName ){
      if( url.size() ){
        urlSeparator = url.find("/");
        if( urlSeparator == -1 ){
          urlCommand = url;
        }else{
          urlCommand = url.substr(0, urlSeparator);
        }
      }

    }else{
      if( url.size() ){
        urlSeparator = url.find("/");
        if( urlSeparator == -1 ){
          urlMemName = url;
        }else{
          urlMemName = url.substr(0, urlSeparator);
          restoreBlanks(urlMemName);
          url = url.substr(urlSeparator + 1);

          if(url.size()){
            urlSeparator = url.find("/");
            if( urlSeparator == -1 ){
              urlSeparator = url.size();
            }

            urlCommand = url.substr(0, urlSeparator);
          }
        }
      }
    }
    requestHandler->memName = urlMemName; 

    if( urlCommand.empty() ){
      if( urlMemName.empty()){
        if( methodStr == "GET"){
          requestHandler->command = ProxygenHandler::COMMAND::LIST_OF_MEMORIES;
        }else if( methodStr == "POST" ){
          requestHandler->command = ProxygenHandler::COMMAND::CREATE_MEM;
        }
      }else{// mem name is not empty, but command is empty 
        if(methodStr == "POST"){
          requestHandler->command = ProxygenHandler::COMMAND::IMPORT_MEM;
          //check if it's in internal format; 
          if(false /*internalFormatHeader*/ ){
            requestHandler->command = ProxygenHandler::COMMAND::IMPORT_MEM_INTERNAL_FORMAT;
          }     
        }else if(methodStr == "GET"){
          requestHandler->command = ProxygenHandler::COMMAND::EXPORT_MEM;
          //check if it's in internal format; 
          if(false /*internalFormatHeader*/ ){
            requestHandler->command = ProxygenHandler::COMMAND::EXPORT_MEM_INTERNAL_FORMAT;
          }
        }else if(methodStr == "DELETE"){
          requestHandler->command = ProxygenHandler::COMMAND::DELETE_MEM;
        }
      }
    }else{
      if ( urlService == additionalServiceName && urlCommand ==  "shutdown" ){//for calls %serviceName%_service
        if( methodStr == "GET"){
          requestHandler->command = ProxygenHandler::COMMAND::SHUTDOWN;
        }else if( methodStr == "POST" && urlCommand ==  "tagreplacement"){
          requestHandler->command = ProxygenHandler::COMMAND::TAGREPLACEMENTTEST;
        }

      }
      if(!urlMemName.empty()){//for command memName should always exists
        if(methodStr == "POST"){
          if(urlCommand == "fuzzysearch"){
            requestHandler->command = ProxygenHandler::COMMAND::FUZZY;
          }else if(urlCommand == "concordancesearch"){
            requestHandler->command = ProxygenHandler::COMMAND::CONCORDANCE;
          }else if(urlCommand ==  "entry"){ // update 
            requestHandler->command = ProxygenHandler::COMMAND::UPDATE_ENTRY;
          }else if(urlCommand ==  "entrydelete"){  
            requestHandler->command = ProxygenHandler::COMMAND::DELETE_ENTRY;
          }else if(urlCommand ==  "import"){ // update 
            requestHandler->command = ProxygenHandler::COMMAND::IMPORT_MEM;
          }
        }else if(methodStr == "GET"){
          if(urlCommand ==  "status"){ // update 
            requestHandler->command = ProxygenHandler::COMMAND::STATUS_MEM;
          }
        }
      }
    }
    //if( urlMemName.empty() )
    //if(         urlMemName.empty() && urlCommand.emtpy() && method == "GET"){// LIST_OF_MEMORIES
    //  requestHandler->command = ProxygenHandler::COMMAND::LIST_OF_MEMORIES;
    //}else if(   urlMemName.empty()   && urlCommand.emty() && method == "POST"){// CREATE_MEM 
   // }else if( (!urlMemName.empty()) && urlCommand.empty() && method == "POST"){// CREATE_MEM 
    //}


    //std::string urlService = url.substr(urlSeparator);
    


   //if (u) {
      // Recreate the URL by just changing the query string
    //}


    //if( strcmp(message, "t5memory_proxygen") == 0 ){
    // if( message->fields_.data_.request.url_ == "t5memory_proxygen"){
      
      //auto statusMessage = message->getStatusMessage();

      return requestHandler;
    //}
  }

  static int startService(int argc, char* argv[]){
    // gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    
    // Increase the default flow control to 10MB/100MB
    uint32_t initialReceiveWindow = uint32_t(1 << 20) * 10;
    uint32_t receiveStreamWindowSize = uint32_t(1 << 20) * 10;
    uint32_t receiveSessionWindowSize = 10 * (1 << 20) * 10;


    LogMessage(TRANSACTION, "Trying to prepare t5memory");
    auto pMemService = OtmMemoryServiceWorker::getInstance();

    char szServiceName[100] = "t5memory";
    char szOtmDirPath[255] ="";
    unsigned int uiPort = 8080;
    int iWorkerThreads = 0;
    unsigned int uiTimeOut = 60000;
    unsigned int uiLogLevel = 2;
    unsigned int uiAllowedRAM = 1500; // MB
    unsigned int uiThreshold = 33;

    /* get configuration settings */
    {
        std::string defOtmDirPath, path = filesystem_get_home_dir();
        defOtmDirPath = path + "/.t5memory/";
        path = defOtmDirPath + "t5memory.conf";
        strncpy(szOtmDirPath, defOtmDirPath.c_str(), 254);

        config conf(path);
        int res = conf.parse();

        if (!res) {
            strncpy(szServiceName,
                conf.get_value("name", "t5memory").c_str(), 100);
            serviceName = szServiceName;
            additionalServiceName = serviceName+"_service";

            strncpy(szOtmDirPath,
                conf.get_value("otmdir", defOtmDirPath.c_str()).c_str(), 254);    
            uiPort = std::stoi(conf.get_value("port", "8080"));
            iWorkerThreads = std::stoi(conf.get_value("threads", "0"));
            uiTimeOut = std::stoi(conf.get_value("timeout", "3600"));
            uiLogLevel = std::stoi(conf.get_value("logLevel","2"));
            uiAllowedRAM = std::stoi(conf.get_value(KEY_ALLOWED_RAM,"500"));
            uiThreshold = std::stoi(conf.get_value(KEY_TRIPLES_THRESHOLD, "33"));
        }else{
          LogMessage3(ERROR, __func__, ":: can't open t5memory.conf, path = ", path.c_str());
        }
    }

    properties_set_str_anyway(KEY_SERVICE_URL, szServiceName);
    properties_set_str_anyway(KEY_OTM_DIR, szOtmDirPath);
    properties_set_int_anyway(KEY_ALLOWED_RAM, uiAllowedRAM);// saving in megabytes to avoid int overflow
    properties_set_int_anyway(KEY_TRIPLES_THRESHOLD, uiThreshold);
    std::string memDir = szOtmDirPath;
    memDir += "/MEM/";
    properties_add_str(KEY_MEM_DIR, memDir.c_str());

    //From here we have logging in file turned on
    DesuppressLoggingInFile();

    LogMessage8(TRANSACTION, "PrepareOtmMemoryService::parsed service name = ", szServiceName, "; port = ", 
        toStr(uiPort).c_str(), "; Worker threads = ", toStr(iWorkerThreads).c_str(),
            "; timeout = ", toStr(uiTimeOut).c_str());
    LogMessage6(TRANSACTION,"PrepareOtmMemoryService:: otm dir = ", szOtmDirPath, "; logLevel = ", toStr(uiLogLevel).c_str(), 
                          "; triples_threshold = ", toStr(uiThreshold));
    SetLogLevel(uiLogLevel);
    // set caller's service name and port fields
    //strcpy( pszService, szServiceName );
    //*puiPort = uiPort;

    char szValue[150];

    LogMessage7(TRANSACTION,"PrepareOtmMemoryService:: done, port/path = :", toStr(uiPort).c_str(),"/", 
        szServiceName,"; Allowed ram = ", toStr(uiAllowedRAM).c_str()," MB\n Setting up proxygen http options...");

    auto options = setup_proxygen_servers_options( iWorkerThreads, uiTimeOut, initialReceiveWindow, receiveStreamWindowSize, receiveSessionWindowSize );

    std::vector<HTTPServer::IPConfig> IPs = {
      {SocketAddress("localhost", uiPort, true), Protocol::HTTP}
    };

    LogMessage2(DEBUG, __func__,":: creating  server...");
    HTTPServer server(std::move(options));
    LogMessage2(DEBUG, __func__,":: server created, binding IPs...");
    server.bind(IPs);

    // Start HTTPServer mainloop in a separate thread
    LogMessage2(DEBUG, __func__,":: IPs binded, creating server thread...");
    std::thread t([&]() { server.start(); });
    LogMessage2(INFO, __func__,":: joining thread... setup is done -> waiting for requests...");
    t.join();
    return 0;
  }
 private:
  folly::ThreadLocalPtr<ProxygenStats> stats_;
};



int main_test() {
  int argc = 1;
  char *str1 = "main_test";
  char *str2 = "1990";

  char* argv[2];
  argv[0] = str1;
  argv[1] = str2;
  ProxygenHandlerFactory::startService(argc, argv);
  return 0;
}


//#endif
