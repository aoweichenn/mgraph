// Copyright 2017 Tencent
// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MGRAPH_GRAPH_HPP
#define MGRAPH_GRAPH_HPP

#include <cstdint>

#include <string>
#include <utility>
#include <vector>

#include <mformat/data_reader.hpp>
#include <mformat/param_dict.hpp>
#include <mgraph/config.hpp>

namespace mgraph {

struct Blob {
    Blob() : producer(-1), consumer(-1) {}

    Blob(std::string blob_name, std::int32_t blob_producer, std::int32_t blob_consumer)
            : name(std::move(blob_name)), producer(blob_producer), consumer(blob_consumer) {}

    std::string name;
    std::int32_t producer;
    std::int32_t consumer;
};

struct LayerSpec {
    LayerSpec() : type_index(-1) {}

    // Text param leaves type_index at -1 and fills type; binary param is the
    // other way around. The consumer resolves text names through its registry.
    std::int32_t type_index;
    std::string type;
    std::string name;
    std::vector<std::int32_t> bottoms;
    std::vector<std::int32_t> tops;
    mformat::ParamDict params;
};

struct ExecutionPlan {
    // Layer indices in execution order (currently the stored order).
    std::vector<std::int32_t> layer_order;
    // For each blob: the last layer that consumes it, or -1 for network output.
    std::vector<std::int32_t> blob_last_use;
    // Blobs without a producer are network inputs; blobs without a consumer are outputs.
    std::vector<std::int32_t> input_blobs;
    std::vector<std::int32_t> output_blobs;
};

class MGRAPH_EXPORT Graph {
public:
    Graph();

    void clear();

    // Parse ncnn text/bin param into topology and per-layer parameter dicts.
    // Returns 0 on success.
    std::int32_t load_param(const mformat::DataReader& reader);
    std::int32_t load_param_bin(const mformat::DataReader& reader);

    [[nodiscard]] ExecutionPlan build_execution_plan() const;

    std::vector<Blob> blobs;
    std::vector<LayerSpec> layers;
};

}  // namespace mgraph

#endif  // MGRAPH_GRAPH_HPP
