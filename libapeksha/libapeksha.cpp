#include "libapeksha.hpp" 

namespace Apeksha{
    Client::Client(std::string ServerUrl, ClientOptions Options) : mServerUrl(ServerUrl)
    {
        ix::initNetSystem();
        mSocket.setUrl(mServerUrl);
        mSocket.setMaxWaitBetweenReconnectionRetries(Options.ReconnectDelay);
        mSocket.setMinWaitBetweenReconnectionRetries(Options.ReconnectDelay);
        mSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg)
                                     {
            if (msg->type == ix::WebSocketMessageType::Message)
            {
                MessageCallback(msg->str);
            }
            else if (msg->type == ix::WebSocketMessageType::Open)
            {
                // OpenCallback();
            }
            else if (msg->type == ix::WebSocketMessageType::Error)
            {
                // Maybe SSL is not configured properly
                std::cout << "Connection error: " << msg->errorInfo.reason << std::endl;
                std::cout << "> " << std::flush;
            } });
    }

    void Client::Open()
    {

        mSocket.start();
        while (mSocket.getReadyState() != ix::ReadyState::Open)
        {
        }
    }
    void Client::Send(std::string message)
    {
        mSocket.send(message);
    }
    void Client::Close()
    {
        mSocket.close();
    }

    Client::~Client()
    {
    }
}
namespace Apeksha{
    namespace json{


    Module::Module(Client &client, ModuleData moduledata) : mClient(client)
    {
        mClient.MessageCallback = [this](std::string message)
        { MessageHandler(message); };
        nlohmann::json ModuleConnectRequest;
        ModuleConnectRequest["module::connect"]["name"] = moduledata.name;
        ModuleConnectRequest["module::connect"]["type"] = moduledata.type;
        mClient.Send(ModuleConnectRequest.dump());
    }
    const void Module::Emit(std::string_view EventName, std::string_view data)
    {
        nlohmann::json event;
        event["event"]["name"] = EventName;
        event["event"]["data"] = data;
        mClient.Send(event.dump());
    }
    const void Module::Emit(std::string_view EventName)
    {
        nlohmann::json event;
        event["event"]["name"] = EventName;
        event["event"]["data"] = nullptr;
        mClient.Send(event.dump());
    }
    Event Module::EventParser(std::string message)
    {
        try
        {
            nlohmann::json EventJson = nlohmann::json::parse(message)["event"];
            if (EventJson["data"].is_null())
            {
                return {
                    .name = EventJson["name"],
                    .data = "null"
                };
            }
            else
            {
                return {
                    .name = EventJson["name"],
                    .data = EventJson["data"]};
            }
        }
        catch (const std::exception &e)
        {
            return {.name = "null", .data = "null"};
        }
    }
    void Module::MessageHandler(std::string message)
    {
        Event RecievedEvent = EventParser(message);
        try
        {
            EventCallbackMap.at(RecievedEvent.name)(RecievedEvent.data);
        }
        catch (const std::exception &e)
        {
        std::cout <<RecievedEvent.data<<std::endl;
        }
    }
    void Module::On(std::string EventName, EventCallback callback)
    {
        EventCallbackMap[EventName] = callback;
    }

    Module::~Module()
    {
    }
    }
}