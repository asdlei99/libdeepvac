/*
 * Copyright (c) 2020 Gemfield <gemfield@civilnet.cn>
 * This file is part of libdeepvac, licensed under the GPLv3 (the "License")
 * You may not use this file except in compliance with the License.
 */

#include <filesystem>
#include <chrono>
#include <ctime> 
#include <iostream>
#include "deepvac.h"

namespace deepvac {

Deepvac::Deepvac(const char* model_path, std::string device){
    GEMFIELD_SI;
    auto start = std::chrono::system_clock::now();
    try{
        device_ = device;
        module_ = std::make_unique<torch::jit::script::Module>(torch::jit::load(model_path, device_));
    }catch(const c10::Error& e){
        std::string msg = gemfield_org::format("%s: %s", "ERROR MODEL: ",e.what_without_backtrace() );
        GEMFIELD_E(msg.c_str());
        throw std::runtime_error(msg);
    }catch(...){
        std::string msg =  "Internal ERROR!";
        GEMFIELD_E(msg.c_str());
        throw std::runtime_error(msg);
    }
    std::chrono::duration<double> model_loading_duration = std::chrono::system_clock::now() - start;
    std::string msg = gemfield_org::format("Model loading time: %f", model_loading_duration.count());
    GEMFIELD_I(msg.c_str());
}

at::Tensor Deepvac::operator() (at::Tensor& t) {
    GEMFIELD_SI;
    return getEmbFromInputTensor(t);
}

at::Tensor Deepvac::getEmbFromInputTensor(at::Tensor& t){
    GEMFIELD_SI;

    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(t.to(device_));

    auto start = std::chrono::system_clock::now();
    at::Tensor output = module_->forward(inputs).toTensor();
    std::chrono::duration<double> forward_duration = std::chrono::system_clock::now() - start;
    std::string msg = gemfield_org::format("forward time: %f",  forward_duration.count() );
    GEMFIELD_DI(msg.c_str());

    return output;
}

} //namespace deepvac