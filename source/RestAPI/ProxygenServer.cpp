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
#include <sstream>
#include <sys/types.h>
#include <ifaddrs.h>

#include "ProxygenHandler.h"
#include "ProxygenStats.h"

#include "EQF.H"
#include "OtmMemoryServiceWorker.h"
#include "../opentm2/core/utilities/LogWrapper.h"
#include "../opentm2/core/utilities/PropertyWrapper.H"
#include "../opentm2/core/utilities/FilesystemWrapper.h"
#include "../cmake/git_version.h"
#include "config.h"
#include "OtmMemoryServiceWorker.h"

using namespace ProxygenService;
using namespace proxygen;

using folly::SocketAddress;

using Protocol = HTTPServer::Protocol;



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
      
      SetLogFilter(true);
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
    if(url.size() <= 1){
      return  new ProxygenHandler(stats_.get());;
    }

    //std::transform(url.begin(), url.end(), url.begin(),
    //[](unsigned char c){ return std::tolower(c); });

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
    return requestHandler;
  }

  static int startService(){
    
    // Increase the default flow control to 10MB/100MB
    uint32_t initialReceiveWindow = uint32_t(1 << 20) * 10;
    uint32_t receiveStreamWindowSize = uint32_t(1 << 20) * 10;
    uint32_t receiveSessionWindowSize = 10 * (1 << 20) * 10;


    LogMessage(TRANSACTION, "Trying to prepare t5memory");
    auto pMemService = OtmMemoryServiceWorker::getInstance();

    char szServiceName[100] = "t5memory";
    char szOtmDirPath[255] ="";
    unsigned int uiPort = 4040;
    int iWorkerThreads = 0;
    unsigned int uiTimeOut = 60000;
    unsigned int uiLogLevel = 2;
    unsigned int uiAllowedRAM = 1500; // MB
    unsigned int uiThreshold = 33;
    bool fLocalHostOnly = false;

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
            uiPort = std::stoi(conf.get_value("port", "4040"));
            iWorkerThreads = std::stoi(conf.get_value("threads", "0"));
            uiTimeOut = std::stoi(conf.get_value("timeout", "3600"));
            uiLogLevel = std::stoi(conf.get_value("logLevel","2"));
            uiAllowedRAM = std::stoi(conf.get_value(KEY_ALLOWED_RAM,"500"));
            uiThreshold = std::stoi(conf.get_value(KEY_TRIPLES_THRESHOLD, "33"));
            fLocalHostOnly = std::stoi(conf.get_value(KEY_LOCALHOST_ONLY, "0")) == 1;
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
    SetLogLevel(uiLogLevel);

    char szValue[150];

    LogMessage7(TRANSACTION,"PrepareOtmMemoryService:: done, port/path = :", toStr(uiPort).c_str(),"/", 
        szServiceName,"; Allowed ram = ", toStr(uiAllowedRAM).c_str()," MB\n Setting up proxygen http options...");

    auto options = setup_proxygen_servers_options( iWorkerThreads, uiTimeOut, initialReceiveWindow, receiveStreamWindowSize, receiveSessionWindowSize );
    
    LogMessage(TRANSACTION, ":: creating address data structure");
    folly::SocketAddress addr;
    ifaddrs* addresses = nullptr, *pAddr = nullptr; 
    char host[NI_MAXHOST];

    if(fLocalHostOnly == false){
      getifaddrs(&addresses);
      int s, family = pAddr->ifa_addr->sa_family;
      
      while(pAddr != nullptr 
      && !( family == AF_INET && !(strcmp(pAddr->ifa_name, "eth0") && strcmp(pAddr->ifa_name, "enp0s3") ) )
      ){
        pAddr = pAddr->ifa_next;
        family = pAddr->ifa_addr->sa_family;
      }
      
    
      if (pAddr != nullptr && pAddr->ifa_addr != nullptr && pAddr->ifa_addr->sa_data != nullptr && pAddr->ifa_addr->sa_family == AF_INET ) {
        s = getnameinfo(pAddr->ifa_addr,
                sizeof(struct sockaddr_in) ,
                host, NI_MAXHOST,
                NULL, 0, NI_NUMERICHOST);
        if (s != 0) {
          pAddr = nullptr;
        }
      }else{
        pAddr = nullptr;
      }
    }

    if(fLocalHostOnly || pAddr == nullptr){
      strcpy(host, "127.0.0.1");
    }
    addr.setFromIpPort(host, uiPort);
    if(addresses != nullptr){
      freeifaddrs(addresses);
    }
    
    std::vector<HTTPServer::IPConfig> IPs = {
      {addr,  Protocol::HTTP}
    };
     

    HTTPServer server(std::move(options));
    server.bind(IPs);

    // Start HTTPServer mainloop in a separate thread
    std::thread t([&]() { server.start(); });
    OtmMemoryServiceWorker::getInstance()->init();
    
    std::stringstream initMsg;
    initMsg << "Service details:\n  Service name = " << szServiceName;
    initMsg << "\n  Address :" << host << ":" << toStr(uiPort); 
    initMsg << "\n  Build date: " << buildDate;
    initMsg << "\n  Git commit info: " << gitHash;
    initMsg << "\n  Version: " << appVersion;
    initMsg << "\n  Workdir: " << szOtmDirPath;
    initMsg << "\n  Worker threads: " << toStr(iWorkerThreads);
    initMsg << "\n  Timeout: " << toStr(uiTimeOut);
    initMsg << "\n  Log level: " << toStr(uiLogLevel);
    initMsg << "\n  Triples threshold: " << toStr(uiThreshold);
    initMsg << "\n  Localhost only: " << toStr(fLocalHostOnly);   
    initMsg << "\n\n\n\
                |==================================================================|\n\
                |-------------Setup is done -> waiting for requests...-------------|\n\
                |==================================================================|\n";
    LogMessage3(TRANSACTION, __func__, ":: ", initMsg.str().c_str());
    t.join();
    return 0;
  }
 private:
  folly::ThreadLocalPtr<ProxygenStats> stats_;
};

int proxygen_server_init(){
  return ProxygenHandlerFactory::startService();
}

