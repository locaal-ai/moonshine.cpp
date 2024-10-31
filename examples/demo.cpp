// main.cpp
#include <moonshine.hpp>

#include <fstream>
#include <iostream>
#include <vector>

std::vector<float> readWavFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open WAV file");
    }

    // Skip WAV header (44 bytes)
    file.seekg(44);

    // Read PCM data
    std::vector<int16_t> pcm_data;
    int16_t sample;
    while (file.read(reinterpret_cast<char *>(&sample), sizeof(int16_t)))
    {
        pcm_data.push_back(sample);
        if (pcm_data.size() >= 160000)
        {
            break;
        }
    }

    // Convert to float32 normalized [-1.0, 1.0]
    std::vector<float> float_data;
    float_data.reserve(pcm_data.size());
    for (const auto &sample : pcm_data)
    {
        float_data.push_back(static_cast<float>(sample) / 32768.0f);
    }

    return float_data;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <models_dir> <wav_file>\n";
        return 1;
    }

    try
    {
        // Read audio file
        auto audio_samples = readWavFile(argv[2]);

        std::cout << "Read " << audio_samples.size() << " samples\n";

        // Initialize model
        MoonshineModel model(argv[1]);

        // Generate tokens
        auto tokens = model.generate(audio_samples);

        // Print tokens (you'll need to decode these using your tokenizer)
        std::cout << "Generated tokens: ";
        for (const auto &token : tokens)
        {
            std::cout << token << " ";
        }
        std::cout << "\n";
    }
    catch (const Ort::Exception &e)
    {
        std::cerr << "ONNX Runtime error: " << e.what() << "\n";
        return 1;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
