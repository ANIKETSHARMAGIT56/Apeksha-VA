#include <crow.h>
#include <mutex>
#include <nlohmann/json.hpp>
#include "include/rang/rang.hpp"
std::mutex mtx;

struct Module
{
  std::string name = "UNSPECIFIED";
  std::string type = "UNSPECIFIED";
};

typedef crow::websocket::connection client;

std::unordered_map<client *, Module> modules;

void log_event(client &module_client,
               nlohmann::json_abi_v3_11_3::basic_json<> &message)
{
  std::cout << "["
            << rang::fg::green
            << modules[&module_client].name
            << rang::fg::reset
            << "] "
            << rang::fg::blue
            << message["event"]["name"]
            << rang::fg::reset
            << " ---> "
            << rang::fg::cyan
            << message["event"]["data"]
            << rang::fg::reset
            << std::endl;
}

int main()
{
  crow::SimpleApp app;

  CROW_WEBSOCKET_ROUTE(app, "/")

      .onopen([&](client &client)
        {
        std::cout <<"new websocket connection"<<std::endl;
        std::lock_guard<std::mutex> _(mtx);
        modules[&client] = Module({.name = "UNSPECIFIED", .type = "UNSPECIFIED"});
        })

      .onclose(
          [&](client &emmiter_client, const std::string &reason, uint16_t code)
          {
            std::cout << "["
                      << rang::fg::green
                      << modules.at(&emmiter_client).name
                      << rang::fg::reset
                      << "] "
                      << rang::fg::red
                      << "disconnected"
                      << rang::fg::reset
                      << std::endl;
            std::lock_guard<std::mutex> _(mtx);
            modules.erase(&emmiter_client);
          })

      .onmessage(
          [&](client &emmiter_client, const std::string &data, bool is_binary)
          {
            std::lock_guard<std::mutex> _(mtx);
            try
            {
              auto message_json = nlohmann::json::parse(data);
              if (message_json.contains("module::connect"))
              {
                Module emmiter_module = Module({
                  .name = message_json["module::connect"]["name"],
                  .type = message_json["module::connect"]["type"]});
                modules[&emmiter_client] = emmiter_module;

                std::cout << "["
                          << rang::fg::green
                          << emmiter_module.name
                          << rang::fg::reset
                          << "] connected as "
                          << rang::fg::cyan
                          << emmiter_module.type
                          << rang::fg::reset
                          << std::endl;
              }
              else if (message_json.contains("event"))
              {
                log_event(emmiter_client, message_json);
                for (auto [iClient, iModule] : modules)
                {
                  if (&emmiter_client != iClient)
                  {
                    iClient->send_text(data);
                  }
                }
              }
              else
              {
                std::cout << data << std::endl;
              }
            }
            catch (std::exception e)
            {
              std::cout <<rang::fg::red<<data << rang::fg::reset<<std::endl;
              sleep(2);
            }
          });

  app.port(18315).multithreaded().run();
}