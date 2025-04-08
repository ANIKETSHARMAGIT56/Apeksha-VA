#include <stdio.h>
#include <sio_client.h>
#include <string>
#include <unistd.h>
#include <functional>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>

#define APEKSHA_USE_IXWEBSOCKET
#define APEKSHA_USE_JSON

#include "../libapeksha/libapeksha.hpp"

#define CLAY_IMPLEMENTATION
#include "clay/clay.h"
#include "clay/renderers/raylib/clay_renderer_raylib.c"
std::string transcript = "";
bool connected = false;
enum transcript_message{
    start,
    partial,
    final,
    end,
    none,
};

class connectionHandler
{
public:
    sio::client &handler;
    connectionHandler(sio::client &h) : handler(h)
    {
    }
    void onfail()
    {
        std::cout << "Failed to connect" << std::endl;
        sleep(1);
        handler.connect("http://127.0.0.1:3000");
    }
    void onopen()
    {
        connected = true;
        std::string message = "ghost window";
        handler.socket()->emit("module_connect", message);
    }
};

class Animator {
private:
    float& variable;        // Reference to the variable to animate
    float finalValue;       // Target value
    float duration;         // Total animation time
    float elapsedTime;      // Time elapsed since the animation started
    float delay;            // Delay before the animation starts
    bool hasStarted;        // Flag to check if the animation has started

public:
    // Constructor
    Animator(float& var)
        : variable(var), finalValue(var), duration(0.0f), elapsedTime(0.0f), delay(0.0f), hasStarted(false) {}

    // Set the animation parameters with delay
    void setTarget(float finalVal, float time, float startDelay = 0.0f) {
        finalValue = finalVal;
        duration = time;
        delay = startDelay;
        elapsedTime = 0.0f;  // Reset elapsed time for new animation
        hasStarted = false; // Reset animation state
    }

    // Update function: animates the variable towards the final value
    void update(float delta_t) {
        if (!hasStarted) {
            elapsedTime += delta_t;

            // Check if the delay period has passed
            if (elapsedTime >= delay) {
                hasStarted = true;
                elapsedTime = 0.0f; // Reset elapsedTime for the actual animation
            }
            return; // Skip animation until the delay is over
        }

        if (elapsedTime < duration) {
            elapsedTime += delta_t;

            // Ensure elapsedTime doesn't exceed duration
            float t = std::min(elapsedTime / duration, 1.0f);

            // Linear interpolation
            variable = variable + t * (finalValue - variable);

            // If animation is complete, snap to the final value
            if (elapsedTime >= duration) {
                variable = finalValue;
            }
        }
    }

    // Check if the animation is complete
    bool isComplete() const {
        return hasStarted && elapsedTime >= duration;
    }
};



int padding = 12;
int font_size = 100;
const uint32_t FONT_ID_BODY_16 = 0;
float gammavalue;
bool reinitializeClay = false;

int width_from_length(int n){
    return int(n*(font_size)/2+padding*2);
}

void HandleClayErrors(Clay_ErrorData errorData) {
    printf("%s", errorData.errorText.chars);
    if (errorData.errorType == CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED) {
        reinitializeClay = true;
        Clay_SetMaxElementCount(200 * 2);
    } else if (errorData.errorType == CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED) {
        reinitializeClay = true;
        Clay_SetMaxMeasureTextCacheWordCount(500 * 2);
    }
}

#define CLAY_STRING(string) (CLAY__INIT(Clay_String) { .length = CLAY__STRING_LENGTH(string), .chars = (string) })
int main()
{
    transcript = "say now!";
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_WINDOW_TRANSPARENT);
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    SetConfigFlags(FLAG_WINDOW_MAXIMIZED);
    SetConfigFlags(FLAG_WINDOW_TOPMOST);
    // SetConfigFlags(FLAG_MSAA_4X_HINT);  
    // SetConfigFlags(FLAG_MSAA_4X_HINT);

    InitWindow(800, 50, "Clay - Raylib Renderer Example");
    SetWindowState(FLAG_WINDOW_UNDECORATED);
    SetWindowState(FLAG_WINDOW_MOUSE_PASSTHROUGH);
    SetWindowState(FLAG_WINDOW_TOPMOST);

    uint64_t clayRequiredMemory = Clay_MinMemorySize();
    Clay_Arena clayMemory = (Clay_Arena){
        .capacity = clayRequiredMemory,
        .memory = (char *)malloc(clayRequiredMemory)};
    Clay_Initialize(clayMemory, (Clay_Dimensions){
                                    .width = static_cast<float>(GetScreenWidth()),
                                    .height = static_cast<float>(GetScreenHeight())},(Clay_ErrorHandler) { HandleClayErrors });
    Clay_SetMeasureTextFunction(Raylib_MeasureText);
    Raylib_fonts[FONT_ID_BODY_16] = (Raylib_Font){
        .fontId = FONT_ID_BODY_16,
        .font = LoadFontEx("/home/aniket/code/Apeksha-VA/apeksha-gui/Doto-Bold.ttf",63,0, 0)};
    int smthsmth = 0;

    transcript_message TAM = none;
    // transcript_message WAM = none;
    int n = transcript.length();


    Apeksha::Client client("ws://localhost:18315",Apeksha::ClientOptions({
        .ReconnectDelay = 1000,
        .ReconnectAttempts = -1
    }));
    client.Open();
    Apeksha::Module guimod(client,{
        .name="apeksha-gui",
        .type="gui"
    });

    guimod.On("speech::start", [&](std::string data)
                   {
                    TAM = start;
                    transcript="Speak Now";
                    n = transcript.length();
                   });
    guimod.On("speech::transcript::partial", [&](std::string data)
                   { 
                    TAM= partial;
                    std::cout << "Partial transcript: " << data << std::endl;
                    transcript = data;
                    n = transcript.length();
                   });
    guimod.On("speech::transcript::final", [&](std::string data)
                   { 
                    TAM = final;
                    std::cout << "Final transcript: " << data << std::endl; 
                    transcript = data;
                    n = transcript.length();
                   });
    guimod.On("speech::end", [&](std::string data)
                   {
                    // TAM=end;
                    std::cout<<"end"<<std::endl;
                   });
    Animator windowtransparency(gammavalue);
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        if (TAM==start)
        {
            windowtransparency.setTarget(200.0f,0.5f);
            TAM=none;
        }
        else if(TAM ==final){
            windowtransparency.setTarget(255.0f,0.1f);
            TAM=end;
        }
        
        if(TAM == end && windowtransparency.isComplete()){
            windowtransparency.setTarget(0.0f,0.5f,1.0f);
            TAM = none;
        }

        // else if (TAM==end)
        // {
        //     windowtransparency.setTarget(0.0f,10.0f,1.0f);
        //     TAM=none;
        // }
        

        char arr[(int)n + 1];
        strcpy(arr, transcript.c_str());
        SetWindowSize(width_from_length(n),font_size+padding*2);
        SetWindowPosition(int((GetMonitorWidth(GetCurrentMonitor())/2)-(n*font_size/2)/2),GetMonitorHeight(GetCurrentMonitor())/2-font_size/2);

        Clay_SetLayoutDimensions(
            (Clay_Dimensions){
                .width = static_cast<float>(GetScreenWidth()),
                .height = static_cast<float>(GetScreenHeight())});
        Clay_BeginLayout();

        CLAY(
            CLAY_ID("root"),
            CLAY_RECTANGLE({.color = {12, 12, 12, gammavalue}, .cornerRadius = 10}),
            CLAY_LAYOUT({.sizing = {
                             .width = CLAY_SIZING_GROW(),
                             .height = CLAY_SIZING_GROW()},
                             .padding = {padding,padding},
                         .childAlignment = {CLAY_ALIGN_X_CENTER,CLAY_ALIGN_Y_CENTER}
                         })

        )
        {
            CLAY_TEXT(
                CLAY_STRING(transcript.c_str()),
                CLAY_TEXT_CONFIG({
                    .textColor = {255, 255, 255, gammavalue},
                    .fontId = FONT_ID_BODY_16,
                    .fontSize = font_size,
                }));
        }
        Clay_RenderCommandArray rendercommands = Clay_EndLayout();
        BeginDrawing();

        Color color = {0, 0, 255, 255};
        ClearBackground(BLANK);
        Clay_Raylib_Render(rendercommands);
        EndDrawing();
        if(!windowtransparency.isComplete()){
            windowtransparency.update(1.0f/60.0f);
        }
    }
}