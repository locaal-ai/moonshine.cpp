// live.cpp
#define SDL_MAIN_HANDLED
#include <moonshine.hpp>
#include <SDL.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <conio.h>  // For _kbhit() and _getch() on Windows

const int SAMPLE_RATE = 16000;
const int BUFFER_SIZE = 4096;

void audioCallback(void* userdata, Uint8* stream, int len)
{
    std::vector<float>* buffer = static_cast<std::vector<float>*>(userdata);
    float* samples = reinterpret_cast<float*>(stream);
    int sample_count = len / sizeof(float);
    buffer->insert(buffer->end(), samples, samples + sample_count);
}

void listAudioDevices()
{
    int count = SDL_GetNumAudioDevices(SDL_TRUE);  // SDL_TRUE for recording devices
    std::cout << "Available recording devices:\n";
    for (int i = 0; i < count; ++i)
    {
        const char* name = SDL_GetAudioDeviceName(i, SDL_TRUE);
        std::cout << i << ": " << (name ? name : "Unknown Device") << "\n";
    }
}

int main(int argc, char* argv[])
{
    std::cout << "Moonshine Live Transcription\n";

    SDL_SetMainReady();  // Tell SDL we'll handle the main entry point
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <models_dir>\n";
        return 1;
    }

    try
    {
        std::cout << "Initializing...\n";

        // Initialize model
        MoonshineModel model(argv[1]);

        std::cout << "Model initialized\n";

        // Initialize SDL
        if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
            std::cerr << "Could not initialize SDL: " << SDL_GetError() << "\n";
            return 1;
        }

        std::cout << "SDL initialized\n";

        // List available devices
        listAudioDevices();

        // Set up audio capture
        SDL_AudioSpec desired_spec;
        SDL_AudioSpec obtained_spec;
        SDL_zero(desired_spec);
        desired_spec.freq = SAMPLE_RATE;
        desired_spec.format = AUDIO_F32;
        desired_spec.channels = 1;
        desired_spec.samples = BUFFER_SIZE;
        desired_spec.callback = audioCallback;

        std::vector<float> audio_buffer;
        desired_spec.userdata = &audio_buffer;

        // Open the default recording device
        SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL,      // device name (NULL for default)
                                                    SDL_TRUE,  // is_capture (recording)
                                                    &desired_spec,   // desired spec
                                                    &obtained_spec,  // obtained spec
                                                    SDL_AUDIO_ALLOW_FORMAT_CHANGE);

        if (dev == 0)
        {
            std::cerr << "Could not open audio device: " << SDL_GetError() << "\n";
            SDL_Quit();
            return 1;
        }

        std::cout << "Audio device opened: " << SDL_GetAudioDeviceName(0, SDL_TRUE) << "\n";
        // print the obtained spec
        std::cout << "Obtained spec: " << obtained_spec.freq << " Hz, "
                  << SDL_AUDIO_BITSIZE(obtained_spec.format) << " bits, "
                  << (obtained_spec.channels == 1 ? "mono" : "stereo") << "\n";

        // Start audio capture
        SDL_PauseAudioDevice(dev, 0);

        std::atomic<bool> running(true);
        std::thread transcription_thread(
            [&]()
            {
                std::cout << "Transcribing...\n";
                size_t last_buffer_size = 0;
                while (running)
                {
                    if (audio_buffer.size() >= SAMPLE_RATE)
                    {
                        if (audio_buffer.size() == last_buffer_size)
                        {
                            // No new audio data
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            continue;
                        }
                        last_buffer_size = audio_buffer.size();

                        // Process audio buffer
                        std::vector<float> buffer(audio_buffer.begin(), audio_buffer.end());

                        // Limit the buffer size to 10 seconds
                        if (audio_buffer.size() > 10 * SAMPLE_RATE)
                        {
                            audio_buffer.erase(audio_buffer.begin(), audio_buffer.end());
                        }

                        // Generate tokens
                        auto start = std::chrono::high_resolution_clock::now();
                        auto tokens = model.generate(buffer);
                        auto end = std::chrono::high_resolution_clock::now();
                        std::chrono::duration<double> duration = end - start;

                        // Detokenize tokens
                        std::string result = model.detokenize(tokens);

                        // erase the last console line
                        std::cout << "\x1b[A";
                        // clear the line
                        std::cout << "\r\033[K";

                        std::cout << "Transcription: " << result << "\n";
                    }
                    else
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                }
                std::cout << "Transcription thread finished\n";
            });

        std::cout << "Recording... Press 'q' or 'ESC' to stop.\n";
        while (running)
        {
            if (_kbhit())
            {
                int ch = _getch();
                if (ch == 'q' || ch == 27)
                {  // 27 is the ASCII code for ESC
                    running = false;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Stop audio capture
        SDL_PauseAudioDevice(dev, 1);

        // Wait for transcription thread to finish
        transcription_thread.join();

        // Clean up
        SDL_CloseAudioDevice(dev);
        SDL_Quit();
    }
    catch (const Ort::Exception& e)
    {
        std::cerr << "ONNX Runtime error: " << e.what() << "\n";
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}