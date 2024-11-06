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

std::vector<float> convertToFloat(const std::vector<int16_t>& pcm_data)
{
    std::vector<float> float_data;
    float_data.reserve(pcm_data.size());
    for (const auto& pcm_sample : pcm_data)
    {
        float_data.push_back(static_cast<float>(pcm_sample) / 32768.0f);
    }
    return float_data;
}

void audioCallback(void* userdata, Uint8* stream, int len)
{
    std::vector<int16_t>* buffer = static_cast<std::vector<int16_t>*>(userdata);
    int16_t* samples = reinterpret_cast<int16_t*>(stream);
    int sample_count = len / sizeof(int16_t);
    buffer->insert(buffer->end(), samples, samples + sample_count);
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

        // Set up audio capture
        SDL_AudioSpec desired_spec;
        SDL_AudioSpec obtained_spec;
        SDL_zero(desired_spec);
        desired_spec.freq = SAMPLE_RATE;
        desired_spec.format = AUDIO_S16LSB;
        desired_spec.channels = 1;
        desired_spec.samples = BUFFER_SIZE;
        desired_spec.callback = audioCallback;

        std::vector<int16_t> audio_buffer;
        desired_spec.userdata = &audio_buffer;

        if (SDL_OpenAudio(&desired_spec, &obtained_spec) < 0)
        {
            std::cerr << "Could not open audio: " << SDL_GetError() << "\n";
            SDL_Quit();
            return 1;
        }

        std::cout << "Audio opened\n";

        // Start audio capture
        SDL_PauseAudio(0);

        std::atomic<bool> running(true);
        std::thread transcription_thread(
            [&]()
            {
                std::cout << "Transcribing...\n";
                while (running)
                {
                    if (audio_buffer.size() >= SAMPLE_RATE)
                    {
                        // Process audio buffer
                        std::vector<int16_t> buffer(audio_buffer.begin(),
                                                    audio_buffer.begin() + SAMPLE_RATE);
                        audio_buffer.erase(audio_buffer.begin(),
                                           audio_buffer.begin() + SAMPLE_RATE);

                        // Convert audio buffer to float
                        std::vector<float> audio_samples = convertToFloat(buffer);

                        // Generate tokens
                        auto start = std::chrono::high_resolution_clock::now();
                        auto tokens = model.generate(audio_samples);
                        auto end = std::chrono::high_resolution_clock::now();
                        std::chrono::duration<double> duration = end - start;

                        // Detokenize tokens
                        std::string result = model.detokenize(tokens);

                        std::cout << "Transcription: " << result << "\n";
                        std::cout << "Token generation took " << duration.count() << " seconds\n";
                    }
                    else
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                }
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
        SDL_PauseAudio(1);

        // Wait for transcription thread to finish
        transcription_thread.join();

        // Clean up
        SDL_CloseAudio();
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