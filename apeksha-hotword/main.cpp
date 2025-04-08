#define MINIAUDIO_IMPLEMENTATION

#include "miniaudio.h"
#include <pv_porcupine.h>
#include <stdio.h>
#include <stdbool.h>
#include <iostream>
#include <nlohmann/json.hpp>

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXUserAgent.h>

// Available at lib/common/porcupine_params.pv
const char *model_path = "resources/hotword-model/porcupine_params_hi.pv";
// AccessKey obtained from Picovoice Console (https://console.picovoice.ai/)
const char *access_key = "XwRPU6GqP3TkX0Wv44X7xXaxNa3EOoKotAmULEURRK7ZC8Hy8WibDA==";
const char *keyword_path = "resources/hotword-model/apeksha-linux.ppn";
const float sensitivity = 1.0f;

pv_porcupine_t *handle = NULL;

// Audio buffer and related variables
static int16_t audio_buffer[512]; // Frame length must match Porcupine's frame length
static size_t audio_buffer_index = 0;
static bool is_capture_active = false;
static ma_device capture_device;

// Callback function to capture audio
static void data_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount) {
    (void)pDevice;
    (void)pOutput;

    const int16_t *input_pcm = (const int16_t *)pInput;

    for (ma_uint32 i = 0; i < frameCount; i++) {
        if (audio_buffer_index < sizeof(audio_buffer) / sizeof(audio_buffer[0])) {
            audio_buffer[audio_buffer_index++] = input_pcm[i];
        }
    }
}

// Initialize the audio capture device
int init_audio_capture() {
    ma_result result;
    ma_device_config capture_config;

    capture_config = ma_device_config_init(ma_device_type_capture);
    capture_config.capture.format = ma_format_s16;
    capture_config.capture.channels = 1;
    capture_config.sampleRate = 16000; // Must match Porcupine's expected rate
    capture_config.dataCallback = data_callback;

    result = ma_device_init(NULL, &capture_config, &capture_device);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize capture device: %d\n", result);
        return -1;
    }

    result = ma_device_start(&capture_device);
    if (result != MA_SUCCESS) {
        printf("Failed to start capture device: %d\n", result);
        ma_device_uninit(&capture_device);
        return -1;
    }

    is_capture_active = true;
    return 0;
}

// Get the next audio frame from the capture buffer
const int16_t *get_next_audio_frame() {
    if (audio_buffer_index >= sizeof(audio_buffer) / sizeof(audio_buffer[0])) {
        audio_buffer_index = 0;
        return audio_buffer;
    }
    return NULL;
}

// Cleanup audio capture resources
void cleanup_audio_capture() {
    if (is_capture_active) {
        ma_device_uninit(&capture_device);
    }
}

int main() {

    ix::initNetSystem();

    ix::WebSocket webSocket;
    std::string url("ws://localhost:18315");
    webSocket.setUrl(url);

    webSocket.setOnMessageCallback([&webSocket](const ix::WebSocketMessagePtr& msg)
        {
            if (msg->type == ix::WebSocketMessageType::Open)
            {
                std::cout << "Connection established" << std::endl;
                std::cout << "> " << std::flush;
                webSocket.send(
                    R"(
                    {
                        "module::connect":{
                            "name":"hotword-cpp",
                            "type":"hotword"
                        }
                    }
                    )"
                );
            }
            else if (msg->type == ix::WebSocketMessageType::Error)
            {
                // Maybe SSL is not configured properly
                std::cout << "Connection error: " << msg->errorInfo.reason << std::endl;
                std::cout << "> " << std::flush;
            }
        }
    );
    webSocket.start();
    
    
    const char *keyword_paths[] = {keyword_path};
    const float sensitivities[] = {sensitivity};

    const pv_status_t status = pv_porcupine_init(
        access_key,
        model_path,
        1,
        keyword_paths,
        sensitivities,
        &handle
    );

    if (status != PV_STATUS_SUCCESS) {
        printf("Failed to initialize Porcupine: %d\n", status);
        return -1;
    }

    if (init_audio_capture() != 0) {
        pv_porcupine_delete(handle);
        return -1;
    }

    printf("Listening for keyword...\n");
    while (true) {
        const int16_t *pcm = get_next_audio_frame();
        if (pcm == NULL) {
            // ma_sleep(32);
            continue; // No audio data available yet, skip this loop
        }

        int32_t keyword_index = -1;
        const pv_status_t process_status = pv_porcupine_process(handle, pcm, &keyword_index);
        if (process_status != PV_STATUS_SUCCESS) {
            printf("Error processing audio: %d\n", process_status);
            break;
        }

        if (keyword_index != -1) {
            std::cout<< "mekichut" <<std::endl;
            webSocket.send(
                R"(
                {
                    "event":{
                        "name":"speech::start",
                        "data":null
                    }
                }
                
                )"
            );
        }
    }
    std::cout<<"Exiting!"<<std::endl;
    pv_porcupine_delete(handle);
    cleanup_audio_capture();

    return 0;
}
