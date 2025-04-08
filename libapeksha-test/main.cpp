#include<iostream>

#define APEKSHA_USE_IXWEBSOCKET
#define APEKSHA_USE_JSON

#include"../libapeksha/libapeksha.hpp"

int main(){
    Apeksha::Client client("ws://localhost:18315",Apeksha::ClientOptions({
        .ReconnectDelay = 1000,
        .ReconnectAttempts = -1
    }));
    client.Open();
    
    
    Apeksha::json::Module tester(client,{
        .name="libapeksha-dev",
        .type="Testing"
    });
    
    tester.Emit("speech::start");
    
    tester.On("speech::transcript::partial",[](std::string data){
        std::cout<<data<<std::endl;
    });


    while (1)
    {
    }    
    client.Close();
    return 0;
}