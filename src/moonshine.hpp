// moonshine.hpp
#pragma once
#include <onnxruntime_cxx_api.h>
#include <vector>
#include <string>
#include <memory>

class MoonshineModel
{
public:
    explicit MoonshineModel(const std::string &models_dir);

    // Generate tokens from audio samples
    // audio_samples should be normalized float32 values in [-1.0, 1.0]
    std::vector<int32_t> generate(const std::vector<float> &audio_samples, size_t max_len = 0);

    // Detoeknize tokens
    std::string detokenize(const std::vector<int32_t> &tokens);

private:
    std::unique_ptr<Ort::Session> preprocess_;
    std::unique_ptr<Ort::Session> encode_;
    std::unique_ptr<Ort::Session> uncached_decode_;
    std::unique_ptr<Ort::Session> cached_decode_;
    Ort::Env env_;
    Ort::MemoryInfo memory_info_;

    // Helper to create ONNX session
    std::unique_ptr<Ort::Session> createSession(const std::string &model_path);

    // Token ID to token map
    std::unordered_map<int32_t, std::string> token_id_to_token_;
};
