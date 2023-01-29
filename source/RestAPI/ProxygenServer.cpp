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
#include "../opentm2/core/utilities/LanguageFactory.H"
#include "../cmake/git_version.h"
#include "config.h"
#include "OtmMemoryServiceWorker.h"

DECLARE_string(servicename);
DECLARE_bool(useconfigfile);
DECLARE_int32(port);
DECLARE_bool(localhostonly);
DECLARE_int32(triplesthreshold);
DECLARE_int32(timeout);
DECLARE_int32(servicethreads);
DECLARE_int32(allowedram);
DECLARE_int32(t5loglevel);

using namespace ProxygenService;
using namespace proxygen;

using folly::SocketAddress;

using Protocol = HTTPServer::Protocol;


#define USE_CONFIG_FILE 1


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

    if(VLOG_IS_ON(1)){
      std::string msg = "SERVER RECEIVED REQUEST:";
      msg += "\n\t URL: " + url;

      LogMessage1(TRANSACTION, msg.c_str());
    }
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
      urlSeparator = url.size();
      //return  new ProxygenHandler(stats_.get());
    }

    std::string urlService = url.substr(0, urlSeparator);
   
    if(urlService != serviceName && urlService != additionalServiceName ){
      LogMessage8(ERROR, __func__, ":: Wrong url \'", urlService.c_str(), "\', should be \'", 
          serviceName.c_str(),"\' or \'", additionalServiceName.c_str(),"\'");
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
      std::transform(urlCommand.begin(), urlCommand.end(), urlCommand.begin(),
            [](unsigned char c){ return std::tolower(c); });
      if ( urlService == additionalServiceName ){//for calls %serviceName%_service
        if( methodStr == "GET"){
          if(urlCommand ==  "shutdown"){
            requestHandler->command = ProxygenHandler::COMMAND::SHUTDOWN;
          }else if(urlCommand == "resources"){
            requestHandler->command = ProxygenHandler::COMMAND::RESOURCE_INFO;
          }
        }else if( methodStr == "POST"){
          if(urlCommand == "tagreplacement"){
           requestHandler->command = ProxygenHandler::COMMAND::TAGREPLACEMENTTEST;
          }
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


    LogMessage1(TRANSACTION, "Trying to prepare t5memory");
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

    std::string defOtmDirPath, path = filesystem_get_home_dir();

    defOtmDirPath = path + "/.t5memory/";    
    strncpy(szOtmDirPath, defOtmDirPath.c_str(), 254);

    //#ifdef GFLAGS_ENABLED
    if(FLAGS_servicename.empty() == false){
      strncpy(szServiceName, FLAGS_servicename.c_str(), 100);
    }

    if(FLAGS_port){
      uiPort = FLAGS_port;
    }

    if(FLAGS_timeout >= 0){
      uiTimeOut = FLAGS_timeout;
    }
    if(FLAGS_triplesthreshold >= 0){
      uiThreshold = FLAGS_triplesthreshold;
    }
    if(FLAGS_localhostonly >= 0){
      fLocalHostOnly = FLAGS_localhostonly;
    }

    if(FLAGS_servicethreads > 0){
      iWorkerThreads = FLAGS_servicethreads;
    }

    if(FLAGS_allowedram >= 0){
      uiAllowedRAM = FLAGS_allowedram;
    }
    if(FLAGS_t5loglevel >= 0){
      uiLogLevel = FLAGS_t5loglevel;
    }
    //#else 
    //  #ifndef USE_CONFIG_FILE
    //    #define USE_CONFIG_FILE
    //  #endif 
    //  #define FLAGS_useconfigfile true
    //#endif //GFLAGS_ENABLED

    #ifdef USE_CONFIG_FILE
    /* get configuration settings */
    if(FLAGS_useconfigfile){
        path = defOtmDirPath + "t5memory.conf";
        config conf(path);
        int res = conf.parse();

        if (!res) {
            strncpy(szServiceName,
                conf.get_value("name", szServiceName).c_str(), 100);            

            strncpy(szOtmDirPath,
                conf.get_value("otmdir", defOtmDirPath.c_str()).c_str(), 254);    
            uiPort = std::stoi(conf.get_value("port", toStr(uiPort)));
            iWorkerThreads = std::stoi(conf.get_value("threads", toStr(iWorkerThreads)));
            uiTimeOut = std::stoi(conf.get_value("timeout", toStr(uiTimeOut)));
            uiLogLevel = std::stoi(conf.get_value("logLevel",toStr(uiLogLevel)));
            uiAllowedRAM = std::stoi(conf.get_value(KEY_ALLOWED_RAM,toStr(uiAllowedRAM)));
            uiThreshold = std::stoi(conf.get_value(KEY_TRIPLES_THRESHOLD, toStr(uiAllowedRAM)));
            fLocalHostOnly = std::stoi(conf.get_value(KEY_LOCALHOST_ONLY, toStr(fLocalHostOnly)));
        }else{
          LogMessage3(ERROR, __func__, ":: can't open t5memory.conf, path = ", path.c_str());
        }
    }
    #endif

    properties_set_str_anyway(KEY_SERVICE_URL, szServiceName);
    properties_set_str_anyway(KEY_OTM_DIR, szOtmDirPath);
    properties_set_int_anyway(KEY_ALLOWED_RAM, uiAllowedRAM);// saving in megabytes to avoid int overflow
    properties_set_int_anyway(KEY_TRIPLES_THRESHOLD, uiThreshold);
    properties_set_int_anyway(KEY_NUM_OF_THREADS, iWorkerThreads);
    properties_set_int_anyway(KEY_TIMEOUT_SETTINGS, uiTimeOut);

    std::string memDir = szOtmDirPath;
    memDir += "/MEM/";
    properties_add_str(KEY_MEM_DIR, memDir.c_str());

    //From here we have logging in file turned on
    DesuppressLoggingInFile();
    SetLogLevel(uiLogLevel);

    char szValue[150];


    LogMessage7(DEBUG,"PrepareOtmMemoryService:: done, port/path = :", toStr(uiPort).c_str(),"/", 
        szServiceName,"; Allowed ram = ", toStr(uiAllowedRAM).c_str()," MB\n Setting up proxygen http options...");

    auto options = setup_proxygen_servers_options( iWorkerThreads, uiTimeOut, initialReceiveWindow, receiveStreamWindowSize, receiveSessionWindowSize );
    
    LogMessage1(DEBUG, ":: creating address data structure");
    folly::SocketAddress addr;
    ifaddrs* addresses = nullptr, *pAddr = nullptr; 
    char host[NI_MAXHOST];

    if(fLocalHostOnly == false){
      getifaddrs(&addresses);
      int s;//, family = pAddr->ifa_addr->sa_family;
      pAddr = addresses;
      
      while(pAddr != nullptr 
          && !( pAddr->ifa_addr->sa_family == AF_INET 
          && !(strcmp(pAddr->ifa_name, "eth0") 
            && strcmp(pAddr->ifa_name, "enp0s3") ) )
      ){
        pAddr = pAddr->ifa_next;
        if(V_IS_ON(2)){
          LogMessage3(DEBUG, __func__,":: checking addr name: ", pAddr->ifa_name );
        }
        //family = pAddr->ifa_addr->sa_family;
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
     
    LogMessage5(INFO, __func__, ":: binding to socket, ",addr.getAddressStr(), "; port = ", toStr(addr.getPort()).c_str());
    HTTPServer server(std::move(options));
    server.bind(IPs);

    // Start HTTPServer mainloop in a separate thread
    std::thread t([&]() { server.start(); });
    OtmMemoryServiceWorker::getInstance()->init();
    
    std::string runDate = getTimeStr();
    properties_set_str_anyway(KEY_RUN_DATE, runDate.c_str());

    //read languages.xml
    LanguageFactory *pLangFactory = LanguageFactory::getInstance();
    std::stringstream initMsg;
    initMsg << "Service details:\n  Service name = " << szServiceName;
    initMsg << "\n  Address :" << host << ":" << uiPort; 
    initMsg << "\n  Build date: " << buildDate;
    initMsg << "\n  Run date: " << runDate;
    initMsg << "\n  Git commit info: " << gitHash;
    initMsg << "\n  Version: " << appVersion;
    initMsg << "\n  Workdir: " << szOtmDirPath;
    initMsg << "\n  Worker threads: " << iWorkerThreads;
    initMsg << "\n  Timeout: " << uiTimeOut;
    initMsg << "\n  Log level: " << uiLogLevel;
    initMsg << "\n  Triples threshold: " << uiThreshold;
    initMsg << "\n  Localhost only: " << fLocalHostOnly; 
    initMsg << "\n  Allowed ram = " << uiAllowedRAM << " MB";  
    initMsg << "\n\n\n\
                |==================================================================|\n\
                |-------------Setup is done -> waiting for requests...-------------|\n\
                |==================================================================|\n";
    LogMessage3(TRANSACTION, __func__, ":: ", initMsg.str().c_str());
    serviceName = szServiceName;
    additionalServiceName = serviceName+"_service";
    
    t.join();
    return 0;
  }
 private:
  folly::ThreadLocalPtr<ProxygenStats> stats_;
};

int proxygen_server_init(){
  return ProxygenHandlerFactory::startService();
}

