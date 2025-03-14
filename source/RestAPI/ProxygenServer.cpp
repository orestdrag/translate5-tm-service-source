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
#include "../opentm2/core/utilities/Property.h"
#include "../opentm2/core/utilities/FilesystemWrapper.h"
#include "../opentm2/core/utilities/LanguageFactory.H"
#include "ProxygenHTTPAcceptor.h"
#include "../cmake/git_version.h"
#include "config.h"
#include "OtmMemoryServiceWorker.h"
#include "ProxygenHTTPAcceptor.h"

DECLARE_string(servicename);
DECLARE_string(t5_ip);
DECLARE_bool(useconfigfile);
DECLARE_int32(port);
DECLARE_bool(localhostonly);
DECLARE_int32(triplesthreshold);
DECLARE_int32(timeout);
DECLARE_int32(servicethreads);
DECLARE_int64(allowedram);
DECLARE_int32(t5loglevel);
DECLARE_bool(log_every_request_start);
DECLARE_bool(log_every_request_end);
DECLARE_int64(tmRequestLockDefaultTimeout);
DECLARE_int64(tmListLockDefaultTimeout);
DECLARE_int64(tmLockDefaultTimeout);
DECLARE_bool(useTimedMutexesForReorganizeAndImport);

DECLARE_bool(limit_num_of_active_requests);
DECLARE_bool(add_premade_socket);
DECLARE_int32(servicethreads);

using namespace ProxygenService;
using namespace proxygen;

using folly::SocketAddress;

using Protocol = HTTPServer::Protocol;


#define USE_CONFIG_FILE 1

DECLARE_int64(socket_backlog);

atomic_int64_t LimitingConnectionEventCallback::activeConnections_ = 0;
atomic_int64_t LimitingConnectionEventCallback::allHandledConnections_ = 0;

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

DECLARE_int64(http_listen_backlog);

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
  options.listenBacklog = FLAGS_http_listen_backlog;
  
 
  // Use our custom acceptor factory
  //options.maxConnections = 16;  
  //options.acceptorFactory = std::make_unique<LimitedAcceptorFactory>(options);
  //options.acceptorFactory = std::make_unique<LimitedAcceptorFactory>(config);
  
  //options.h2cEnabled = true;
  options.h2cEnabled = false;
  options.supportsConnect = false;

  return options;
}

  std::string serviceName;
  std::string additionalServiceName;


  // Define a maximum limit for concurrent requests
  //constexpr int MAX_CONCURRENT_REQUESTS = 100;

  
class ProxygenHandlerFactory : public RequestHandlerFactory {
 public:
  void onServerStart(folly::EventBase* /*evb*/) noexcept override {
      
      T5Logger::GetInstance()->SetLogFilter(true);
    }

  void onServerStop() noexcept override {
  }

  RequestHandler* onRequest(RequestHandler* handler, HTTPMessage* message) noexcept override {

    ProxygenHandler* requestHandler = new ProxygenHandler();

    if (FLAGS_limit_num_of_active_requests 
      && (++ProxygenStats::getInstance()->activeRequestsCounter >= FLAGS_servicethreads)) 
    {
      requestHandler->pRequest = new ErrorRequestData;
      requestHandler->pRequest->buildErrorReturn(503, "Server overloaded. Try again later.", 503);
      //ProxygenStats::getInstance()->activeRequestsCounter.fetch_sub(1);
      return requestHandler;

      // Decrement the counter immediately since we're not handling this request
      //if(FLAGS_limit_num_of_active_requests) 
      //  activeRequestsCounter.fetch_sub(1);
        
    }
    
    auto methodStr = message->getMethodString ();
    auto method = message->getMethod ();
    auto url = message->getURL () ;
    auto path = message->getPath () ;
    //auto queryString   = message->getQueryString () ;
    auto headers = message->getHeaders();
    //auto queryParams = message->getQueryParams();

    //if(0 || VLOG_IS_ON(1)){
    if(FLAGS_log_every_request_start){
      std::string msg = "SERVER RECEIVED REQUEST:";
      msg += "\t URL: " + url;
      msg += "\t method: " + methodStr;
      //msg += "\t body";
      
      msg += "\t reqCount(id): " + std::to_string(ProxygenStats::getInstance()->getRequestCount());
      

      T5LOG(T5TRANSACTION)  << msg;
    }
    if(url.size() <= 1){
      //ProxygenStats::getInstance()->activeRequestsCounter.fetch_sub(1);
      return requestHandler;
    }

    //std::transform(url.begin(), url.end(), url.begin(),
    //[](unsigned char c){ return std::tolower(c); });

    std::string urlMemName;
    std::string urlCommand;

    try{
    if(url[0] == '/'){
      url = url.substr(1, url.size() - 1);
    }

    auto urlSeparator = url.find("/");

    if( urlSeparator == -1 ){
      urlSeparator = url.size();
    }

    std::string urlService = url.substr(0, urlSeparator);

    if(urlService != serviceName && urlService != additionalServiceName ){
      T5LOG(T5ERROR) <<":: Wrong url \'" << urlService << "\', should be \'"<<
          serviceName << "\' or \'" << additionalServiceName <<"\'";
      requestHandler->pRequest = new UnknownRequestData;
      //ProxygenStats::getInstance()->activeRequestsCounter.fetch_sub(1) 
      return requestHandler;
    }

    requestHandler->pRequest =  nullptr;
    
    url = url.size() > urlSeparator ? url.substr(urlSeparator + 1) : "";
    
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
  
          url = url.size() > urlSeparator ? url.substr(urlSeparator + 1) : "";

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

    if( urlCommand.empty() ){
      if( urlMemName.empty()){
        if( methodStr == "GET"){
          requestHandler->pRequest = new ListTMRequestData();
        }else if( methodStr == "POST" ){
          std::string contentTypeHeader = headers.getSingleOrEmpty("Content-Type");
          bool isMultipart = contentTypeHeader.find("multipart/form-data") != std::string::npos;
          requestHandler->pRequest = new CreateMemRequestData(!isMultipart);

        }
      }else{// mem name is not empty, but command is empty 
        if(methodStr == "POST"){
          //requestHandler->pRequest = new ImportRequestData();
          //check if it's in internal format;            
        }else if(methodStr == "GET"){
          requestHandler->pRequest = new ExportRequestData();
        }else if(methodStr == "DELETE"){
          requestHandler->pRequest = new DeleteMemRequestData();
        }
      }
    }else{
      std::transform(urlCommand.begin(), urlCommand.end(), urlCommand.begin(),
            [](unsigned char c){ return std::tolower(c); });
      if ( urlService == additionalServiceName ){//for calls %serviceName%_service
        if( methodStr == "GET"){
          if(urlCommand ==  "shutdown"){
            requestHandler->pRequest = new ShutdownRequestData();
          }else if(urlCommand == "resources"){
            auto request = new ResourceInfoRequestData();
            requestHandler->pRequest = request;
          }else if(urlCommand == "flags"){
            requestHandler->pRequest =  new FlagsRequestData();
          }else if(urlCommand == "savetms"){
            requestHandler->pRequest = new SaveAllTMsToDiskRequestData();
          }
        }else if( methodStr == "POST"){
          if(urlCommand == "tagreplacement"){
            requestHandler->pRequest = new TagRemplacementRequestData();
          }
        }
      }
      if(!urlMemName.empty()){//for command memName should always exists
        if(methodStr == "POST"){ if(urlCommand == "multifuzzysearch"){
          requestHandler->pRequest = new MultiFuzzySearchRequestData();
        }else if(urlCommand == "fuzzysearch"){
            requestHandler->pRequest = new FuzzySearchRequestData();
          }else if(urlCommand == "concordancesearch"){
            requestHandler->pRequest = new ConcordanceSearchRequestData();
          }else if(urlCommand == "search"){
            requestHandler->pRequest = new ConcordanceExtendedSearchRequestData();
          }else if(urlCommand ==  "entry"){ // update 
            requestHandler->pRequest = new UpdateEntryRequestData();
          }else if(urlCommand ==  "entrydelete"){  
            requestHandler->pRequest = new DeleteEntryRequestData();
          }else if(urlCommand == "entriesdelete"){
            requestHandler->pRequest = new DeleteEntriesReorganizeRequestData();
          }else if(urlCommand == "getentry"){
            requestHandler->pRequest = new GetEntryRequestData();
          }else if(urlCommand ==  "import"){ // update 
            requestHandler->pRequest = new ImportRequestData();
          }else if(urlCommand == "importtmx"){
            //requestHandler->pRequest = new ImportStreamRequestData();
            requestHandler->pRequest = new ImportRequestData(false); 
          }else if(urlCommand == "clone"){
            requestHandler->pRequest = new CloneTMRequestData();
          }else if(urlCommand == "importlocal"){
            requestHandler->pRequest = new ImportLocalRequestData();
          }
        }else if(methodStr == "GET"){
          if(urlCommand ==  "status"){ // update 
            requestHandler->pRequest = new StatusMemRequestData();
          }else if(urlCommand == "reorganize"){
            requestHandler->pRequest = new ReorganizeRequestData();
          }else if(urlCommand == "download.tm"){
            requestHandler->pRequest = new ExportRequestData(EXPORT_MEM_INTERNAL_FORMAT_STREAM);
          }else if(urlCommand == "download.tmx"){
            requestHandler->pRequest = new ExportRequestData(EXPORT_MEM_TMX_STREAM);
          }else if(urlCommand == "flush"){
            requestHandler->pRequest = new FlushMemRequestData();
          }
        }
      }
    }
    }
    catch(...){
      T5LOG(T5ERROR) << "url wasn't parsed correctly, url: " << url;
    }

    if(!requestHandler->pRequest){
      requestHandler->pRequest = new UnknownRequestData;
      requestHandler->pRequest->strUrl = url;
      
    }
    requestHandler->pRequest->strMemName = urlMemName;
    restoreBlanks(requestHandler->pRequest->strMemName);;

    return requestHandler;
  }

  static int startService(){
    FilesystemHelper::init();
    
    // Increase the default flow control to 10MB/100MB
    uint32_t initialReceiveWindow = uint32_t(1 << 20) * 10;
    uint32_t receiveStreamWindowSize = uint32_t(1 << 20) * 10;
    uint32_t receiveSessionWindowSize = 10 * (1 << 20) * 10;


    T5LOG( T5TRANSACTION)  << "Trying to prepare t5memory";
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
    if(FLAGS_servicethreads > 0){
      iWorkerThreads = FLAGS_servicethreads;
    }

    if(FLAGS_allowedram >= 0){
      uiAllowedRAM = FLAGS_allowedram;
    }
    if(FLAGS_t5loglevel >= 0){
      uiLogLevel = FLAGS_t5loglevel;
    }

    Properties::GetInstance()->set_anyway(KEY_SERVICE_URL, szServiceName);
    Properties::GetInstance()->set_anyway(KEY_OTM_DIR, FilesystemHelper::GetOtmDir().c_str());
    Properties::GetInstance()->set_anyway(KEY_ALLOWED_RAM, uiAllowedRAM);// saving in megabytes to avoid int overflow
   // Properties::GetInstance()->set_anyway(KEY_TRIPLES_THRESHOLD, uiThreshold);
    Properties::GetInstance()->set_anyway(KEY_NUM_OF_THREADS, iWorkerThreads);
    Properties::GetInstance()->set_anyway(KEY_TIMEOUT_SETTINGS, uiTimeOut);

    Properties::GetInstance()->add_key(KEY_MEM_DIR, FilesystemHelper::GetMemDir().c_str());

    //From here we have logging in file turned on
    T5Logger::GetInstance()->DesuppressLoggingInFile();
    T5Logger::GetInstance()->SetLogLevel(uiLogLevel);

    char szValue[150];
    std::string host;


    T5LOG(T5DEBUG)<<"PrepareOtmMemoryService:: done, port/path = :" << uiPort <<"/" << szServiceName <<
      "; Allowed ram = " << uiAllowedRAM <<" MB\n Setting up proxygen http options...";

    auto options = setup_proxygen_servers_options( iWorkerThreads, uiTimeOut, initialReceiveWindow, receiveStreamWindowSize, receiveSessionWindowSize );
    
    T5LOG( T5DEBUG) << ":: creating address data structure";

    folly::SocketAddress addr;
    LimitingConnectionEventCallback connectionEventCallback;
    if(FLAGS_t5_ip.empty()){
      addr.setFromLocalPort(uiPort);
      host = "any";
    }else{
      host = FLAGS_t5_ip;
      addr.setFromIpPort(host, uiPort);
    }

     
    T5LOG(T5TRANSACTION) <<  ":: binding to socket, " << addr.getAddressStr() << "; port = " <<addr.getPort();
    folly::AsyncServerSocket::UniquePtr serverSocket(new folly::AsyncServerSocket(folly::EventBaseManager::get()->getEventBase()));

    if(FLAGS_add_premade_socket){
      // Create and configure the AsyncServerSocket
      serverSocket->setReusePortEnabled(true); // Example option
      serverSocket->bind(addr);
      serverSocket->listen(FLAGS_socket_backlog);
      serverSocket->setConnectionEventCallback(&connectionEventCallback);
      serverSocket->startAccepting();
      options.useExistingSocket(std::move(serverSocket)); 
    }
    
    HTTPServer server(std::move(options));
    //auto acceptorFactory = std::make_shared<LimitedAcceptorFactory>;
    server.bind({{addr, proxygen::HTTPServer::Protocol::HTTP}});

    //auto connectionEventCallback = std::make_shared<MyConnectionEventCallback>();
    /*
    for (auto &sock : server.getSockets()) {
      // Note: setMaxConnections() is provided by folly::AsyncServerSocket.
      //sock->onConnectionEnqueuedForAcceptorCallback(connectionEventCallback);
      if (auto* serverSocket = dynamic_cast<folly::AsyncServerSocket*>(sock)) {
        // basePtr is actually an instance of AsyncServerSocket
        serverSocket->setConnectionEventCallback(&connectionEventCallback);
      } else {
        // basePtr is not an instance of AsyncServerSocket
        // Handle accordingly
      }
    }//*/

    // Start HTTPServer mainloop in a separate thread
    std::thread t([&]() { server.start(); });
    OtmMemoryServiceWorker::getInstance()->init();
    
    std::string runDate = getTimeStr();
    Properties::GetInstance()->set_anyway(KEY_RUN_DATE, runDate);

    //read languages.xml
    LanguageFactory *pLangFactory = LanguageFactory::getInstance();

    serviceName = szServiceName;
    additionalServiceName = serviceName+"_service";

    if( true /*printIninMsg*/){
      T5LOG(T5TRANSACTION) << "Service details:  Service name = " << szServiceName <<
      "\n  Additional service name: "<< additionalServiceName <<
      "\n  Address :" << host << ":" << uiPort << 
      "\n  Build date: " << buildDate <<
      "\n  Run date: " << runDate <<
      "\n  Git commit info: " << gitHash <<
      "\n  Version: " << appVersion <<
      "\n  Asan:" << asanIsOn <<
      "\n  Workdir: " << szOtmDirPath <<
      "\n  Worker threads: " << iWorkerThreads <<
      "\n  Timeout: " << uiTimeOut <<
      "\n  Log level: " << uiLogLevel <<
      "\n  Triples threshold: " << uiThreshold <<
      "\n  Localhost only: " << fLocalHostOnly << 
      "\n  Allowed ram = " << uiAllowedRAM << " MB" <<
      "\n  tmRequestLockDefaultTimeout = " << FLAGS_tmRequestLockDefaultTimeout << "ms" <<
      "\n  tmListLockDefaultTimeout = " << FLAGS_tmListLockDefaultTimeout << "ms" <<
      "\n  tmLockDefaultTimeout = " << FLAGS_tmLockDefaultTimeout<< "ms" << 
      "\n  useTimedMutexesForReorganizeAndImport = " << FLAGS_useTimedMutexesForReorganizeAndImport <<
      "\n\n\n\
                  |==================================================================|\n\
                  |-------------Setup is done -> waiting for requests...-------------|\n\
                  |==================================================================|\n";
      
    }
    
    t.join();
    return 0;
  }
 private:
};

int proxygen_server_init(){
  return ProxygenHandlerFactory::startService();
}

