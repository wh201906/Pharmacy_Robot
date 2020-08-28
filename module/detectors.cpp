// Copyright (C) 2018-2019 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <functional>
#include <iostream>
#include <fstream>
#include <random>
#include <memory>
#include <chrono>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <iterator>
#include <map>

#include <inference_engine.hpp>
#include <ngraph/ngraph.hpp>

#include <samples/ocv_common.hpp>
#include <samples/slog.hpp>

#include <ie_iextension.h>

#include "detectors.hpp"

using namespace InferenceEngine;

BaseDetection::BaseDetection(const std::string &topoName,
                             const std::string &pathToModel,
                             const std::string &deviceForInference,
                             int maxBatch, bool isBatchDynamic, bool isAsync,
                             bool doRawOutputMessages)
    : topoName(topoName), pathToModel(pathToModel), deviceForInference(deviceForInference),
      maxBatch(maxBatch), isBatchDynamic(isBatchDynamic), isAsync(isAsync),
      enablingChecked(false), _enabled(false), doRawOutputMessages(doRawOutputMessages)
{
    if(isAsync)
    {
        slog::info << "Use async mode for " << topoName << slog::endl;
    }
}

BaseDetection::~BaseDetection() {}

ExecutableNetwork* BaseDetection::operator ->()
{
    return &net;
}

void BaseDetection::submitRequest()
{
    if(!enabled() || request == nullptr) return;
    if(isAsync)
    {
        request->StartAsync();
    }
    else
    {
        request->Infer();
    }
}

void BaseDetection::wait()
{
    if(!enabled() || !request || !isAsync)
        return;
    request->Wait(IInferRequest::WaitMode::RESULT_READY);
}

bool BaseDetection::enabled() const
{
    if(!enablingChecked)
    {
        _enabled = !pathToModel.empty();
        if(!_enabled)
        {
            slog::info << topoName << " DISABLED" << slog::endl;
        }
        enablingChecked = true;
    }
    return _enabled;
}

void BaseDetection::printPerformanceCounts(std::string fullDeviceName)
{
    if(!enabled())
    {
        return;
    }
    slog::info << "Performance counts for " << topoName << slog::endl << slog::endl;
    ::printPerformanceCounts(*request, std::cout, fullDeviceName, false);
}


FaceDetection::FaceDetection(const std::string &pathToModel,
                             const std::string &deviceForInference,
                             int maxBatch, bool isBatchDynamic, bool isAsync,
                             double detectionThreshold, bool doRawOutputMessages,
                             float bb_enlarge_coefficient, float bb_dx_coefficient, float bb_dy_coefficient)
    : BaseDetection("Face Detection", pathToModel, deviceForInference, maxBatch, isBatchDynamic, isAsync, doRawOutputMessages),
      detectionThreshold(detectionThreshold),
      maxProposalCount(0), objectSize(0), enquedFrames(0), width(0), height(0),
      network_input_width(0), network_input_height(0),
      bb_enlarge_coefficient(bb_enlarge_coefficient), bb_dx_coefficient(bb_dx_coefficient),
      bb_dy_coefficient(bb_dy_coefficient), resultsFetched(false) {}

void FaceDetection::submitRequest()
{
    if(!enquedFrames) return;
    enquedFrames = 0;
    resultsFetched = false;
    results.clear();
    BaseDetection::submitRequest();
}

void FaceDetection::enqueue(const cv::Mat &frame)
{
    if(!enabled()) return;

    if(!request)
    {
        request = net.CreateInferRequestPtr();
    }

    width = static_cast<float>(frame.cols);
    height = static_cast<float>(frame.rows);

    Blob::Ptr  inputBlob = request->GetBlob(input);

    matU8ToBlob<uint8_t>(frame, inputBlob);

    enquedFrames = 1;
}

CNNNetwork FaceDetection::read(const InferenceEngine::Core& ie)
{
    slog::info << "Loading network files for Face Detection" << slog::endl;
    /** Read network model **/
    auto network = ie.ReadNetwork(pathToModel);
    /** Set batch size to 1 **/
    slog::info << "Batch size is set to " << maxBatch << slog::endl;
    network.setBatchSize(maxBatch);
    // -----------------------------------------------------------------------------------------------------

    // ---------------------------Check inputs -------------------------------------------------------------
    slog::info << "Checking Face Detection network inputs" << slog::endl;
    InputsDataMap inputInfo(network.getInputsInfo());
    if(inputInfo.size() != 1)
    {
        throw std::logic_error("Face Detection network should have only one input");
    }
    InputInfo::Ptr inputInfoFirst = inputInfo.begin()->second;
    inputInfoFirst->setPrecision(Precision::U8);

    const SizeVector inputDims = inputInfoFirst->getTensorDesc().getDims();
    network_input_height = inputDims[2];
    network_input_width = inputDims[3];

    // -----------------------------------------------------------------------------------------------------

    // ---------------------------Check outputs ------------------------------------------------------------
    slog::info << "Checking Face Detection network outputs" << slog::endl;
    OutputsDataMap outputInfo(network.getOutputsInfo());
    if(outputInfo.size() == 1)
    {
        DataPtr& _output = outputInfo.begin()->second;
        output = outputInfo.begin()->first;
        const SizeVector outputDims = _output->getTensorDesc().getDims();
        maxProposalCount = outputDims[2];
        objectSize = outputDims[3];
        if(objectSize != 7)
        {
            throw std::logic_error("Face Detection network output layer should have 7 as a last dimension");
        }
        if(outputDims.size() != 4)
        {
            throw std::logic_error("Face Detection network output dimensions not compatible shoulld be 4, but was " +
                                   std::to_string(outputDims.size()));
        }
        _output->setPrecision(Precision::FP32);
    }
    else
    {
        for(const auto& outputLayer : outputInfo)
        {
            const SizeVector outputDims = outputLayer.second->getTensorDesc().getDims();
            if(outputDims.size() == 2 && outputDims.back() == 5)
            {
                output = outputLayer.first;
                maxProposalCount = outputDims[0];
                objectSize = outputDims.back();
                outputLayer.second->setPrecision(Precision::FP32);
            }
            else if(outputDims.size() == 1 && outputLayer.second->getPrecision() == Precision::I32)
            {
                labels_output = outputLayer.first;
            }
        }
        if(output.empty() || labels_output.empty())
        {
            throw std::logic_error("Face Detection network must contain ether single DetectionOutput or "
                                   "'boxes' [nx5] and 'labels' [n] at least, where 'n' is a number of detected objects.");
        }
    }

    slog::info << "Loading Face Detection model to the " << deviceForInference << " device" << slog::endl;
    input = inputInfo.begin()->first;
    return network;
}

void FaceDetection::fetchResults()
{
    if(!enabled()) return;
    results.clear();
    if(resultsFetched) return;
    resultsFetched = true;
    LockedMemory<const void> outputMapped = as<MemoryBlob>(request->GetBlob(output))->rmap();
    const float *detections = outputMapped.as<float *>();

    if(!labels_output.empty())
    {
        LockedMemory<const void> labelsMapped = as<MemoryBlob>(request->GetBlob(labels_output))->rmap();
        const int32_t *labels = labelsMapped.as<int32_t *>();

        for(int i = 0; i < maxProposalCount && objectSize == 5; i++)
        {
            Result r;
            r.label = labels[i];
            r.confidence = detections[i * objectSize + 4];

            if(r.confidence <= detectionThreshold && !doRawOutputMessages)
            {
                continue;
            }

            r.location.x = static_cast<int>(detections[i * objectSize + 0] / network_input_width * width);
            r.location.y = static_cast<int>(detections[i * objectSize + 1] / network_input_height * height);
            r.location.width = static_cast<int>(detections[i * objectSize + 2] / network_input_width * width - r.location.x);
            r.location.height = static_cast<int>(detections[i * objectSize + 3] / network_input_height * height - r.location.y);

            // Make square and enlarge face bounding box for more robust operation of face analytics networks
            int bb_width = r.location.width;
            int bb_height = r.location.height;

            int bb_center_x = r.location.x + bb_width / 2;
            int bb_center_y = r.location.y + bb_height / 2;

            int max_of_sizes = std::max(bb_width, bb_height);

            int bb_new_width = static_cast<int>(bb_enlarge_coefficient * max_of_sizes);
            int bb_new_height = static_cast<int>(bb_enlarge_coefficient * max_of_sizes);

            r.location.x = bb_center_x - static_cast<int>(std::floor(bb_dx_coefficient * bb_new_width / 2));
            r.location.y = bb_center_y - static_cast<int>(std::floor(bb_dy_coefficient * bb_new_height / 2));

            r.location.width = bb_new_width;
            r.location.height = bb_new_height;

            if(doRawOutputMessages)
            {
                std::cout << "[" << i << "," << r.label << "] element, prob = " << r.confidence <<
                          "    (" << r.location.x << "," << r.location.y << ")-(" << r.location.width << ","
                          << r.location.height << ")"
                          << ((r.confidence > detectionThreshold) ? " WILL BE RENDERED!" : "") << std::endl;
            }
            if(r.confidence > detectionThreshold)
            {
                results.push_back(r);
            }
        }
    }

    for(int i = 0; i < maxProposalCount && objectSize == 7; i++)
    {
        float image_id = detections[i * objectSize + 0];
        if(image_id < 0)
        {
            break;
        }
        Result r;
        r.label = static_cast<int>(detections[i * objectSize + 1]);
        r.confidence = detections[i * objectSize + 2];

        if(r.confidence <= detectionThreshold && !doRawOutputMessages)
        {
            continue;
        }

        r.location.x = static_cast<int>(detections[i * objectSize + 3] * width);
        r.location.y = static_cast<int>(detections[i * objectSize + 4] * height);
        r.location.width = static_cast<int>(detections[i * objectSize + 5] * width - r.location.x);
        r.location.height = static_cast<int>(detections[i * objectSize + 6] * height - r.location.y);

        // Make square and enlarge face bounding box for more robust operation of face analytics networks
        int bb_width = r.location.width;
        int bb_height = r.location.height;

        int bb_center_x = r.location.x + bb_width / 2;
        int bb_center_y = r.location.y + bb_height / 2;

        int max_of_sizes = std::max(bb_width, bb_height);

        int bb_new_width = static_cast<int>(bb_enlarge_coefficient * max_of_sizes);
        int bb_new_height = static_cast<int>(bb_enlarge_coefficient * max_of_sizes);

        r.location.x = bb_center_x - static_cast<int>(std::floor(bb_dx_coefficient * bb_new_width / 2));
        r.location.y = bb_center_y - static_cast<int>(std::floor(bb_dy_coefficient * bb_new_height / 2));

        r.location.width = bb_new_width;
        r.location.height = bb_new_height;

        if(doRawOutputMessages)
        {
            std::cout << "[" << i << "," << r.label << "] element, prob = " << r.confidence <<
                      "    (" << r.location.x << "," << r.location.y << ")-(" << r.location.width << ","
                      << r.location.height << ")"
                      << ((r.confidence > detectionThreshold) ? " WILL BE RENDERED!" : "") << std::endl;
        }
        if(r.confidence > detectionThreshold)
        {
            results.push_back(r);
        }
    }
}

FacialLandmarksDetection::FacialLandmarksDetection(const std::string &pathToModel,
        const std::string &deviceForInference,
        int maxBatch, bool isBatchDynamic, bool isAsync, bool doRawOutputMessages)
    : BaseDetection("Facial Landmarks", pathToModel, deviceForInference, maxBatch, isBatchDynamic, isAsync, doRawOutputMessages),
      outputFacialLandmarksBlobName("align_fc3"), enquedFaces(0)
{
}

void FacialLandmarksDetection::submitRequest()
{
    if(!enquedFaces) return;
    if(isBatchDynamic)
    {
        request->SetBatch(enquedFaces);
    }
    BaseDetection::submitRequest();
    enquedFaces = 0;
}

void FacialLandmarksDetection::enqueue(const cv::Mat &face)
{
    if(!enabled())
    {
        return;
    }
    if(enquedFaces == maxBatch)
    {
        slog::warn << "Number of detected faces more than maximum(" << maxBatch << ") processed by Facial Landmarks estimator" << slog::endl;
        return;
    }
    if(!request)
    {
        request = net.CreateInferRequestPtr();
    }

    Blob::Ptr inputBlob = request->GetBlob(input);

    matU8ToBlob<uint8_t>(face, inputBlob, enquedFaces);

    enquedFaces++;
}

std::vector<float> FacialLandmarksDetection::operator[](int idx) const
{
    std::vector<float> normedLandmarks;

    auto landmarksBlob = request->GetBlob(outputFacialLandmarksBlobName);
    auto n_lm = getTensorChannels(landmarksBlob->getTensorDesc());
    LockedMemory<const void> facialLandmarksBlobMapped =
        as<MemoryBlob>(request->GetBlob(outputFacialLandmarksBlobName))->rmap();
    const float *normed_coordinates = facialLandmarksBlobMapped.as<float *>();

    if(doRawOutputMessages)
    {
        std::cout << "[" << idx << "] element, normed facial landmarks coordinates (x, y):" << std::endl;
    }

    auto begin = n_lm * idx;
    auto end = begin + n_lm / 2;
    for(auto i_lm = begin; i_lm < end; ++i_lm)
    {
        float normed_x = normed_coordinates[2 * i_lm];
        float normed_y = normed_coordinates[2 * i_lm + 1];

        if(doRawOutputMessages)
        {
            std::cout << normed_x << ", " << normed_y << std::endl;
        }

        normedLandmarks.push_back(normed_x);
        normedLandmarks.push_back(normed_y);
    }

    return normedLandmarks;
}

CNNNetwork FacialLandmarksDetection::read(const InferenceEngine::Core& ie)
{
    slog::info << "Loading network files for Facial Landmarks Estimation" << slog::endl;
    // Read network model
    auto network = ie.ReadNetwork(pathToModel);
    // Set maximum batch size
    network.setBatchSize(maxBatch);
    slog::info << "Batch size is set to  " << network.getBatchSize() << " for Facial Landmarks Estimation network" << slog::endl;

    // ---------------------------Check inputs -------------------------------------------------------------
    slog::info << "Checking Facial Landmarks Estimation network inputs" << slog::endl;
    InputsDataMap inputInfo(network.getInputsInfo());
    if(inputInfo.size() != 1)
    {
        throw std::logic_error("Facial Landmarks Estimation network should have only one input");
    }
    InputInfo::Ptr& inputInfoFirst = inputInfo.begin()->second;
    inputInfoFirst->setPrecision(Precision::U8);
    input = inputInfo.begin()->first;
    // -----------------------------------------------------------------------------------------------------

    // ---------------------------Check outputs ------------------------------------------------------------
    slog::info << "Checking Facial Landmarks Estimation network outputs" << slog::endl;
    OutputsDataMap outputInfo(network.getOutputsInfo());
    const std::string outName = outputInfo.begin()->first;
    if(outName != outputFacialLandmarksBlobName)
    {
        throw std::logic_error("Facial Landmarks Estimation network output layer unknown: " + outName
                               + ", should be " + outputFacialLandmarksBlobName);
    }
    Data& data = *outputInfo.begin()->second;
    data.setPrecision(Precision::FP32);
    const SizeVector& outSizeVector = data.getTensorDesc().getDims();
    if(outSizeVector.size() != 2 && outSizeVector.back() != 70)
    {
        throw std::logic_error("Facial Landmarks Estimation network output layer should have 2 dimensions and 70 as"
                               " the last dimension");
    }

    slog::info << "Loading Facial Landmarks Estimation model to the " << deviceForInference << " plugin"
               << slog::endl;

    _enabled = true;
    return network;
}


Load::Load(BaseDetection& detector) : detector(detector)
{
}

void Load::into(InferenceEngine::Core & ie, const std::string & deviceName, bool enable_dynamic_batch) const
{
    if(detector.enabled())
    {
        std::map<std::string, std::string> config = { };
        bool isPossibleDynBatch = deviceName.find("CPU") != std::string::npos ||
                                  deviceName.find("GPU") != std::string::npos;

        if(enable_dynamic_batch && isPossibleDynBatch)
        {
            config[PluginConfigParams::KEY_DYN_BATCH_ENABLED] = PluginConfigParams::YES;
        }

        detector.net = ie.LoadNetwork(detector.read(ie), deviceName, config);
    }
}

