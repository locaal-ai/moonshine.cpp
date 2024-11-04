#include "moonshine.hpp"
#include <filesystem>
#include <stdexcept>
#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>

std::vector<const char *> cached_decode_input_names = {
    "args_0",
    "args_1",
    "args_2",
    "args_3",
    "args_4",
    "args_5",
    "args_6",
    "args_7",
    "args_8",
    "args_9",
    "args_10",
    "args_11",
    "args_12",
    "args_13",
    "args_14",
    "args_15",
    "args_16",
    "args_17",
    "args_18",
    "args_19",
    "args_20",
    "args_21",
    "args_22",
    "args_23",
    "args_24",
    "args_25",
    "args_26",
};

std::vector<const char *> cached_decode_output_names = {
    "reversible_embedding",
    "functional_23",
    "functional_23_1",
    "input_layer_102",
    "input_layer_103",
    "functional_26",
    "functional_26_1",
    "input_layer_106",
    "input_layer_107",
    "functional_29",
    "functional_29_1",
    "input_layer_110",
    "input_layer_111",
    "functional_32",
    "functional_32_1",
    "input_layer_114",
    "input_layer_115",
    "functional_35",
    "functional_35_1",
    "input_layer_118",
    "input_layer_119",
    "functional_38",
    "functional_38_1",
    "input_layer_122",
    "input_layer_123",
};

std::vector<const char *> decode_input_names = {"args_0", "args_1", "args_2"};
std::vector<const char *> decode_output_names = {
    "reversible_embedding",
    "functional_22",
    "functional_22_1",
    "functional_22_2",
    "functional_22_3",
    "functional_25",
    "functional_25_1",
    "functional_25_2",
    "functional_25_3",
    "functional_28",
    "functional_28_1",
    "functional_28_2",
    "functional_28_3",
    "functional_31",
    "functional_31_1",
    "functional_31_2",
    "functional_31_3",
    "functional_34",
    "functional_34_1",
    "functional_34_2",
    "functional_34_3",
    "functional_37",
    "functional_37_1",
    "functional_37_2",
    "functional_37_3",
};

std::vector<const char *> encode_input_names = {"args_0", "args_1"};
std::vector<const char *> encode_ouput_names = {"layer_normalization_12"};

// Function to read a UTF-8 encoded file into a string
std::string readFileAsUtf8(const std::string &file_path)
{
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("File not found: " + file_path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

MoonshineModel::MoonshineModel(const std::string &models_dir)
    : memory_info_(Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU))
{
    std::cout << "Loading models from: " << models_dir << std::endl;
    this->env_ = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "MoonshineModel");
    preprocess_ = createSession(models_dir + "/preprocess.onnx");
    encode_ = createSession(models_dir + "/encode.onnx");
    uncached_decode_ = createSession(models_dir + "/uncached_decode.onnx");
    cached_decode_ = createSession(models_dir + "/cached_decode.onnx");

    // Read tokenizer JSON as UTF-8
    std::string tokenizer_content = readFileAsUtf8(models_dir + "/tokenizer.json");
    nlohmann::json tokenizer = nlohmann::json::parse(tokenizer_content);

    // Create token ID to token map
    for (const auto &item : tokenizer["model"]["vocab"].items())
    {
        token_id_to_token_[item.value()] = item.key();
    }
}

std::unique_ptr<Ort::Session> MoonshineModel::createSession(const std::string &model_path)
{
    if (!std::filesystem::exists(model_path))
    {
        throw std::runtime_error("Model file not found: " + model_path);
    }

    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

#ifdef _WIN32
    // Convert wstring for Windows compatibility
    std::wstring real_path = std::filesystem::path(model_path).wstring();
    std::wcout << "Loading model: " << real_path << std::endl;
#else
    const std::string &real_path = model_path;
#endif

    // Use the constructor with wide string path
    return std::make_unique<Ort::Session>(env_, real_path.c_str(), session_options);
}

std::vector<int32_t> MoonshineModel::generate(const std::vector<float> &audio_samples, size_t max_len)
{
    // Prepare input audio tensor
    std::vector<int64_t> audio_shape = {1, static_cast<int64_t>(audio_samples.size())};
    Ort::Value audio_tensor = Ort::Value::CreateTensor<float>(
        memory_info_,
        const_cast<float *>(audio_samples.data()),
        audio_samples.size(),
        audio_shape.data(),
        audio_shape.size());

    std::vector<Ort::AllocatedStringPtr> outputNames;
    Ort::AllocatorWithDefaultOptions allocator;

    std::vector<const char *> rawInputNames = {"args_0"};
    std::vector<const char *> rawOutputNames = {"sequential"};

    std::cout << "Preprocess. Input size: " << audio_samples.size() << std::endl;

    // Preprocess
    std::vector<Ort::Value> preprocess_inputs;
    preprocess_inputs.push_back(std::move(audio_tensor));
    auto preprocessed = preprocess_->Run(
        Ort::RunOptions{nullptr},
        rawInputNames.data(),
        preprocess_inputs.data(),
        preprocess_inputs.size(),
        rawOutputNames.data(),
        1);

    // print the shape of the output tensor
    auto shape = preprocessed[0].GetTensorTypeAndShapeInfo().GetShape();

    // Calculate sequence length
    int32_t seq_len = (int32_t)shape[1];
    const std::vector<int64_t> seq_len_shape = {1};
    Ort::Value seq_len_tensor = Ort::Value::CreateTensor<int32_t>(
        memory_info_,
        &seq_len,
        1,
        seq_len_shape.data(),
        seq_len_shape.size());

    std::vector<Ort::Value> encode_inputs;
    encode_inputs.reserve(2); // Reserve space for the inputs
    encode_inputs.push_back(std::move(preprocessed[0]));
    encode_inputs.push_back(std::move(seq_len_tensor));
    // Encode
    auto context = encode_->Run(
        Ort::RunOptions{nullptr},
        encode_input_names.data(),
        encode_inputs.data(),
        encode_inputs.size(),
        encode_ouput_names.data(),
        encode_ouput_names.size());

    // copy context to avoid modifying the original context
    auto context_shape = context[0].GetTensorTypeAndShapeInfo().GetShape();
    Ort::Value context_copy = Ort::Value::CreateTensor<float>(
        memory_info_,
        context[0].GetTensorMutableData<float>(),
        context[0].GetTensorTypeAndShapeInfo().GetElementCount(),
        context_shape.data(),
        context_shape.size());

    // Initial token
    std::vector<int32_t> tokens = {1}; // Start token
    std::vector<int64_t> input_shape = {1, 1};
    Ort::Value inputs_tensor = Ort::Value::CreateTensor<int32_t>(
        memory_info_,
        tokens.data(),
        tokens.size(),
        input_shape.data(),
        input_shape.size());

    // Calculate max_len if not provided
    if (max_len == 0)
    {
        max_len = static_cast<size_t>((audio_samples.size() / 16000.0) * 6);
    }

    seq_len_tensor = Ort::Value::CreateTensor<int32_t>(
        memory_info_,
        &seq_len,
        1,
        seq_len_shape.data(),
        seq_len_shape.size());

    std::vector<Ort::Value> uncached_decode_inputs;
    uncached_decode_inputs.reserve(3); // Reserve space for the inputs
    uncached_decode_inputs.push_back(std::move(inputs_tensor));
    uncached_decode_inputs.push_back(std::move(context_copy));
    uncached_decode_inputs.push_back(std::move(seq_len_tensor));

    // Initial uncached decode
    auto cache = uncached_decode_->Run(
        Ort::RunOptions{nullptr},
        decode_input_names.data(),
        uncached_decode_inputs.data(),
        uncached_decode_inputs.size(),
        decode_output_names.data(),
        uncached_decode_->GetOutputCount());

    // Generate tokens
    for (size_t i = 0; i < max_len; ++i)
    {
        float *logits_data = cache[0].GetTensorMutableData<float>();
        size_t logits_size = cache[0].GetTensorTypeAndShapeInfo().GetElementCount();

        // Find argmax
        int32_t next_token = 0;
        float max_val = logits_data[0];
        for (size_t j = 1; j < logits_size; ++j)
        {
            if (logits_data[j] > max_val)
            {
                max_val = logits_data[j];
                next_token = static_cast<int32_t>(j);
            }
        }

        tokens.push_back(next_token);
        if (next_token == 2)
            break; // End token

        // Update sequence length
        seq_len++;
        seq_len_tensor = Ort::Value::CreateTensor<int32_t>(
            memory_info_,
            &seq_len,
            1,
            seq_len_shape.data(),
            seq_len_shape.size());

        // Prepare next input
        std::vector<int32_t> next_input = {next_token};
        inputs_tensor = Ort::Value::CreateTensor<int32_t>(
            memory_info_,
            next_input.data(),
            1,
            input_shape.data(),
            input_shape.size());

        context_copy = Ort::Value::CreateTensor<float>(
            memory_info_,
            context[0].GetTensorMutableData<float>(),
            context[0].GetTensorTypeAndShapeInfo().GetElementCount(),
            context_shape.data(),
            context_shape.size());

        // Run cached decode
        std::vector<Ort::Value> cached_inputs;
        cached_inputs.push_back(std::move(inputs_tensor));
        cached_inputs.push_back(std::move(context_copy));
        cached_inputs.push_back(std::move(seq_len_tensor));
        for (size_t j = 1; j < cache.size(); ++j)
        {
            cached_inputs.push_back(std::move(cache[j]));
        }

        cache = cached_decode_->Run(
            Ort::RunOptions{nullptr},
            cached_decode_input_names.data(),
            cached_inputs.data(),
            cached_inputs.size(),
            cached_decode_output_names.data(),
            cached_decode_->GetOutputCount());
    }

    return tokens;
}

std::string MoonshineModel::detokenize(const std::vector<int32_t> &tokens)
{
    std::string result;
    for (const auto &token : tokens)
    {
        if (token_id_to_token_.find(token) != token_id_to_token_.end())
        {
            result += token_id_to_token_.at(token) + " ";
        }
    }

    return result;
}
