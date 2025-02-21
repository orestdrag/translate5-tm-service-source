#pragma once

#include <proxygen/httpserver/HTTPServer.h>
#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/RequestHandlerFactory.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include <proxygen/httpserver/HTTPServerAcceptor.h>
#include <folly/io/async/AsyncServerSocket.h>
#include <folly/SocketAddress.h>
#include <iostream>

using namespace proxygen;
using namespace folly;
/*
class LimitedHTTPServerAcceptor : public HTTPServerAcceptor {
public:
    static constexpr size_t MAX_CONNECTIONS = 16;  // Limit total open connections
    static constexpr size_t MAX_QUEUED_REQUESTS = 8; // Limit waiting requests

    explicit LimitedHTTPServerAcceptor(const HTTPServerOptions& options)
        : HTTPServerAcceptor(options) {}

    void onNewConnection(
        folly::AsyncTransportWrapper::UniquePtr socket,
        const folly::SocketAddress* addr,
        const std::string& nextProtocolName,
        wangle::SecureTransportType secureTransportType,
        const wangle::TransportInfo& tinfo) override {

        size_t currentConnections = getNumConnections();
        
        if (currentConnections >= MAX_CONNECTIONS) {
            T5LOG(T5ERROR) << "Too many connections: Rejecting new connection\n";
            socket.reset();  // Immediately close connection
            return;
        }

        HTTPServerAcceptor::onNewConnection(
            std::move(socket), addr, nextProtocolName, secureTransportType, tinfo);
    }
};

class LimitedAcceptorFactory : public HTTPServer::AcceptorFactory {
public:
    HTTPServerAcceptor* newAcceptor(folly::EventBase* evb) override {
        return new LimitedHTTPServerAcceptor(options_);
    }

    explicit LimitedAcceptorFactory(const HTTPServerOptions& options)
        : options_(options) {}

private:
    HTTPServerOptions options_;
};/

class LimitedAcceptorFactory : public wangle::AcceptorFactory {
public:
    //explicit LimitedAcceptorFactory(const proxygen::AcceptorConfiguration& config)
    //    : config_(config) {}

    virtual std::shared_ptr<wangle::Acceptor> newAcceptor(folly::EventBase* evb) override {
        //auto codecFactory = std::make_shared<proxygen::HTTPDefaultCodecFactory>();
        //auto handlerFactory = std::vector<proxygen::RequestHandlerFactory*>{};
        
        //auto acceptor = new proxygen::HTTPServerAcceptor(config_, codecFactory, handlerFactory, options_);

        auto config =  proxygen::HTTPServerAcceptor::makeConfig(ipConfig_, options_);
        //std::shared_ptr<HTTPCodecFactory>& codecFactory = nullptr;
        auto acceptor =  proxygen::HTTPServerAcceptor::make(config, options_);
    config.maxNumPendingConnectionsPerWorker = 0;
/*
 static AcceptorConfiguration makeConfig(const HTTPServer::IPConfig& ipConfig,
                                          const HTTPServerOptions& opts);

  static std::unique_ptr<HTTPServerAcceptor> make(
      const AcceptorConfiguration& conf,
      const HTTPServerOptions& opts,
      const std::shared_ptr<HTTPCodecFactory>& codecFactory = nullptr);
        
        //acceptor->setMaxConnections(16);  // Enforce max concurrent connections
        return acceptor;
    }

    virtual ~LimitedAcceptorFactory() = default;

private:
    proxygen::AcceptorConfiguration config_;
    proxygen::HTTPServerOptions options_;
    proxygen::HTTPServer::IPConfig ipConfig_;
};
//*/

DECLARE_bool(log_tcp_backog_events);

class LimitingConnectionEventCallback : public folly::AsyncServerSocket::ConnectionEventCallback {
public:
    const int64_t maxConnections_{1};
    static atomic_int64_t activeConnections_;
    static atomic_int64_t allHandledConnections_;

    void onConnectionEnqueuedForAcceptorCallback(

        const NetworkSocket socket,
        const SocketAddress& addr) noexcept override {
        //if (activeConnections_ >= maxConnections_) {
            // Close the socket to reject the connection
            //::close(socket.toFd());
            
            /*
            proxygen::ResponseBuilder(nullptr)
            .status(500, "Internal Server Error")
            .body(folly::IOBuf::copyBuffer("An error occurred"))
            .sendWithEOM();//*/
        //} else {
            // Increment the active connection couny           
             ++activeConnections_;
            if(FLAGS_log_tcp_backog_events)
                T5LOG(T5TRANSACTION)  << "onConnectionEnqueuedForAcceptorCallback from: " << addr.describe() << "; activeConnections = " << activeConnections_;
            // Proceed with normal processing
        //}
        // Custom logic to handle the enqueued connection
    }
    
    void onConnectionAccepted(
        const folly::NetworkSocket socket,
        const folly::SocketAddress& addr) noexcept override {
            
        if(FLAGS_log_tcp_backog_events)
            T5LOG(T5TRANSACTION)  << "Connection accepted from: " << addr.describe() << "; activeConnections = " << activeConnections_;
    }
    void onConnectionDropped(
        const folly::NetworkSocket socket,
        const folly::SocketAddress& addr) noexcept override {
        // Decrement the active connection count
        --activeConnections_;
        if(FLAGS_log_tcp_backog_events)
            T5LOG(T5TRANSACTION)  << "Connection dropped from: " << addr.describe()
                    << " Error: " << "; activeConnections = " << activeConnections_;
    }

    void onConnectionAcceptError(const int err) noexcept override {
        T5LOG(T5ERROR) << "Error accepting connection: " << folly::errnoStr(err);
    }

    void onConnectionDequeuedByAcceptorCallback(
        const folly::NetworkSocket socket,
        const folly::SocketAddress& addr) noexcept override {
           activeConnections_--;
           allHandledConnections_++;

        if(FLAGS_log_tcp_backog_events)
            T5LOG(T5TRANSACTION)  << "Connection dequeued by acceptor from: " << addr.describe() << "; activeConnections = " << activeConnections_ << "; allHandledConnections_ = " << allHandledConnections_;
    }

    void onBackoffStarted() noexcept override {
        if(FLAGS_log_tcp_backog_events)
            T5LOG(T5TRANSACTION)  << "Backoff started." ;
    }

    void onBackoffEnded() noexcept override {
        if(FLAGS_log_tcp_backog_events)
            T5LOG(T5TRANSACTION)  << "Backoff ended." ;
    }

    void onBackoffError() noexcept override {
        if(FLAGS_log_tcp_backog_events)
            T5LOG(T5ERROR) << "Backoff error occurred." ;
    }

};

