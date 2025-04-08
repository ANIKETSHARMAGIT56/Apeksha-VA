#pragma once
#include "IXWebSocket/ixwebsocket/IXWebSocket.h"
#include <string>
#include <string_view>
#include <iostream>


#include "IXWebSocket/ixwebsocket/IXNetSystem.h"
#include "IXWebSocket/ixwebsocket/IXWebSocket.h"
#include "IXWebSocket/ixwebsocket/IXUserAgent.h"
namespace Apeksha
{
    struct ClientOptions
    {
        u_int64_t ReconnectDelay = 1000;
        int64_t ReconnectAttempts = -1;
    };

    class Client
    {
    private:
        std::string mServerUrl;
        ix::WebSocket mSocket;
        // std::

    public:
        std::function<void()> OpenCallback{};
        std::function<void(std::string message)> MessageCallback = [](std::string message)
        { std::cout << message << std::endl; };
        Client(std::string ServerUrl, ClientOptions Options);
        void Open();
        void Send(std::string message);
        void Close();

        ~Client();
    };
}


#include <nlohmann/json.hpp>
namespace Apeksha
{
    struct Event
    {
        std::string name;
        std::string data;
    };
    struct ModuleData
    {
        std::string name;
        std::string type;
        std::string transport = "json";
    };
    typedef std::function<void(std::string data)> EventCallback;

    namespace json{
        class Module
        {
        private:
            Client &mClient;
            std::unordered_map<std::string, EventCallback> EventCallbackMap;

        public:
            Module(Client &client, ModuleData moduledata);
            const void Emit(std::string_view EventName, std::string_view data);
            const void Emit(std::string_view EventName);
            void On(std::string EventName, EventCallback callback);
            void MessageHandler(std::string message);
            Event EventParser(std::string message);
            ~Module();
        };
    }
    namespace serialized{

    }
}
