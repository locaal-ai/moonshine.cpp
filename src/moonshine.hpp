// moonshine.hpp
#pragma once
#include <onnxruntime_cxx_api.h>
#include <vector>
#include <string>
#include <memory>
#include <map>

/**
 * @class MoonshineModel
 * @brief A class to handle the ONNX model inference for the Moonshine project.
 */
class MoonshineModel
{
   public:
    /**
     * @brief Constructor for the MoonshineModel class.
     * @param models_dir The directory containing the ONNX model files.
     */
    explicit MoonshineModel(const std::string &models_dir);

    /**
     * @brief Generate tokens from audio samples.
     * @param audio_samples A vector of normalized float32 audio samples in the range [-1.0, 1.0].
     * @param max_len The maximum length of the generated tokens. Default is 0 (no limit).
     * @return A vector of generated token IDs.
     */
    std::vector<int32_t> generate(const std::vector<float> &audio_samples, size_t max_len = 0);

    /**
     * @brief Detokenize the generated tokens into a string.
     * @param tokens A vector of token IDs.
     * @return A detokenized string.
     */
    std::string detokenize(const std::vector<int32_t> &tokens);

   private:
    std::unique_ptr<Ort::Session> preprocess_;  ///< ONNX session for the preprocessing model.
    std::unique_ptr<Ort::Session> encode_;      ///< ONNX session for the encoding model.
    std::unique_ptr<Ort::Session>
        uncached_decode_;  ///< ONNX session for the uncached decoding model.
    std::unique_ptr<Ort::Session> cached_decode_;  ///< ONNX session for the cached decoding model.
    Ort::Env env_;                                 ///< ONNX Runtime environment.
    Ort::MemoryInfo memory_info_;                  ///< Memory information for ONNX Runtime.

    /**
     * @brief Helper function to create an ONNX session.
     * @param model_path The path to the ONNX model file.
     * @return A unique pointer to the created ONNX session.
     */
    std::unique_ptr<Ort::Session> createSession(const std::string &model_path);

    std::map<int, std::string> token_id_to_token_;  ///< Map from token IDs to token strings.

    /**
     * @brief Load the tokenizer from a JSON string.
     * @param tokenizer_content The JSON string containing the tokenizer data.
     */
    void load_tokenizer(const std::string &tokenizer_content);
};
