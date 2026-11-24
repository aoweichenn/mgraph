// Copyright 2017 Tencent
// Copyright 2026 mcnn authors
// check-magic-numbers: allow-file ported ncnn param token limits
// SPDX-License-Identifier: BSD-3-Clause

#include <mgraph/graph.hpp>

#include <climits>
#include <cstring>

#include <mgraph/log.hpp>

namespace mgraph {
namespace {

constexpr std::int32_t kParamMagic = 7767517;
constexpr std::int32_t kMaxNetCount = 1000000;

std::int32_t scan_net_value(const mformat::DataReader& reader, std::int32_t& value) {
    char token[32];
    if (reader.scan("%31s", token) != 1) {
        return 0;
    }

    if (std::strlen(token) == 31) {
        char suffix[2];
        if (reader.scan("%1[^ \t\r\n\v\f]", suffix) == 1) {
            return 0;
        }
    }

    const char* p = token;
    const bool negative = *p == '-';
    if (*p == '-' || *p == '+') {
        ++p;
    }
    if (*p == '\0') {
        return 0;
    }

    std::uint32_t magnitude = 0;
    const std::uint32_t limit =
        negative ? static_cast<std::uint32_t>(INT_MAX) + 1U : static_cast<std::uint32_t>(INT_MAX);
    for (; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return 0;
        }
        const std::uint32_t digit = static_cast<std::uint32_t>(*p - '0');
        if (magnitude > (limit - digit) / 10U) {
            return 0;
        }
        magnitude = magnitude * 10U + digit;
    }

    value = negative ? (magnitude == static_cast<std::uint32_t>(INT_MAX) + 1U ? INT_MIN
                                                                              : -static_cast<std::int32_t>(magnitude))
                     : static_cast<std::int32_t>(magnitude);
    return 1;
}

std::int32_t scan_net_value(const mformat::DataReader& reader, char (&value)[256]) {
    if (reader.scan("%255s", value) != 1) {
        return 0;
    }

    if (std::strlen(value) == 255) {
        char suffix[2];
        if (reader.scan("%1[^ \t\r\n\v\f]", suffix) == 1) {
            return 0;
        }
    }

    return 1;
}

std::int32_t find_blob_index_by_name(const std::vector<Blob>& blobs, std::int32_t blob_index, const char* name) {
    for (std::int32_t index = 0; index < blob_index; ++index) {
        if (blobs[static_cast<std::size_t>(index)].name == name) {
            return index;
        }
    }
    return -1;
}

}  // namespace

Graph::Graph() {}

void Graph::clear() {
    this->blobs.clear();
    this->layers.clear();
}

std::int32_t Graph::load_param(const mformat::DataReader& reader) {
    this->clear();

    std::int32_t magic = 0;
    if (scan_net_value(reader, magic) != 1) {
        MGRAPH_LOGE("parse magic failed");
        return -1;
    }
    if (magic != kParamMagic) {
        MGRAPH_LOGE("param is too old, please regenerate");
        return -1;
    }

    std::int32_t layer_count = 0;
    std::int32_t blob_count = 0;
    if (scan_net_value(reader, layer_count) != 1 || scan_net_value(reader, blob_count) != 1) {
        MGRAPH_LOGE("parse layer_count or blob_count failed");
        return -1;
    }
    if (layer_count <= 0 || blob_count <= 0 || layer_count > kMaxNetCount || blob_count > kMaxNetCount) {
        MGRAPH_LOGE("invalid layer_count or blob_count");
        return -1;
    }

    this->blobs.resize(static_cast<std::size_t>(blob_count));
    this->layers.resize(static_cast<std::size_t>(layer_count));

    std::int32_t blob_index = 0;
    for (std::int32_t i = 0; i < layer_count; ++i) {
        char layer_type[256];
        char layer_name[256];
        std::int32_t bottom_count = 0;
        std::int32_t top_count = 0;
        if (scan_net_value(reader, layer_type) != 1 || scan_net_value(reader, layer_name) != 1 ||
            scan_net_value(reader, bottom_count) != 1 || scan_net_value(reader, top_count) != 1) {
            MGRAPH_LOGE("parse layer %d header failed", i);
            return -1;
        }
        if (bottom_count < 0 || top_count < 0 || bottom_count > kMaxNetCount || top_count > kMaxNetCount) {
            MGRAPH_LOGE("invalid bottom_count or top_count %d %d", bottom_count, top_count);
            return -1;
        }

        LayerSpec& spec = this->layers[static_cast<std::size_t>(i)];
        spec.type = layer_type;
        spec.name = layer_name;
        spec.bottoms.resize(static_cast<std::size_t>(bottom_count));
        spec.tops.resize(static_cast<std::size_t>(top_count));

        for (std::int32_t j = 0; j < bottom_count; ++j) {
            char bottom_name[256];
            if (scan_net_value(reader, bottom_name) != 1) {
                MGRAPH_LOGE("parse layer %d bottom %d failed", i, j);
                return -1;
            }

            std::int32_t bottom_blob_index = find_blob_index_by_name(this->blobs, blob_index, bottom_name);
            if (bottom_blob_index == -1) {
                if (blob_index >= blob_count) {
                    MGRAPH_LOGE("too many blobs at layer %d bottom %d", i, j);
                    return -1;
                }
                bottom_blob_index = blob_index;
                this->blobs[static_cast<std::size_t>(bottom_blob_index)].name = bottom_name;
                ++blob_index;
            }

            this->blobs[static_cast<std::size_t>(bottom_blob_index)].consumer = i;
            spec.bottoms[static_cast<std::size_t>(j)] = bottom_blob_index;
        }

        for (std::int32_t j = 0; j < top_count; ++j) {
            if (blob_index >= blob_count) {
                MGRAPH_LOGE("too many blobs at layer %d top %d", i, j);
                return -1;
            }

            char top_name[256];
            if (scan_net_value(reader, top_name) != 1) {
                MGRAPH_LOGE("parse layer %d top %d failed", i, j);
                return -1;
            }

            Blob& blob = this->blobs[static_cast<std::size_t>(blob_index)];
            blob.name = top_name;
            blob.producer = i;
            spec.tops[static_cast<std::size_t>(j)] = blob_index;
            ++blob_index;
        }

        if (spec.params.load_param(reader) != 0) {
            MGRAPH_LOGE("ParamDict load_param %d %s failed", i, layer_name);
            return -1;
        }
    }

    return 0;
}

std::int32_t Graph::load_param_bin(const mformat::DataReader& reader) {
    this->clear();

    auto read_value = [&reader](void* value, std::size_t size) -> bool { return reader.read(value, size) == size; };

    std::int32_t magic = 0;
    if (!read_value(&magic, sizeof(magic))) {
        MGRAPH_LOGE("read magic failed");
        return -1;
    }
    if (magic != kParamMagic) {
        MGRAPH_LOGE("param is too old, please regenerate");
        return -1;
    }

    std::int32_t layer_count = 0;
    std::int32_t blob_count = 0;
    if (!read_value(&layer_count, sizeof(layer_count)) || !read_value(&blob_count, sizeof(blob_count))) {
        MGRAPH_LOGE("read layer_count or blob_count failed");
        return -1;
    }
    if (layer_count <= 0 || blob_count <= 0 || layer_count > kMaxNetCount || blob_count > kMaxNetCount) {
        MGRAPH_LOGE("invalid layer_count or blob_count");
        return -1;
    }

    this->blobs.resize(static_cast<std::size_t>(blob_count));
    this->layers.resize(static_cast<std::size_t>(layer_count));

    for (std::int32_t i = 0; i < layer_count; ++i) {
        std::int32_t type_index = 0;
        std::int32_t bottom_count = 0;
        std::int32_t top_count = 0;
        if (!read_value(&type_index, sizeof(type_index)) || !read_value(&bottom_count, sizeof(bottom_count)) ||
            !read_value(&top_count, sizeof(top_count))) {
            MGRAPH_LOGE("read layer %d header failed", i);
            return -1;
        }
        if (type_index < 0) {
            MGRAPH_LOGE("invalid layer type %d", type_index);
            return -1;
        }
        if (bottom_count < 0 || top_count < 0 || bottom_count > kMaxNetCount || top_count > kMaxNetCount) {
            MGRAPH_LOGE("invalid bottom_count or top_count %d %d", bottom_count, top_count);
            return -1;
        }

        LayerSpec& spec = this->layers[static_cast<std::size_t>(i)];
        spec.type_index = type_index;
        spec.bottoms.resize(static_cast<std::size_t>(bottom_count));
        spec.tops.resize(static_cast<std::size_t>(top_count));

        for (std::int32_t j = 0; j < bottom_count; ++j) {
            std::int32_t bottom_blob_index = 0;
            if (!read_value(&bottom_blob_index, sizeof(bottom_blob_index))) {
                MGRAPH_LOGE("read layer %d bottom %d failed", i, j);
                return -1;
            }
            if (bottom_blob_index < 0 || bottom_blob_index >= blob_count) {
                MGRAPH_LOGE("layer %d invalid bottom_blob_index %d", i, bottom_blob_index);
                return -1;
            }
            this->blobs[static_cast<std::size_t>(bottom_blob_index)].consumer = i;
            spec.bottoms[static_cast<std::size_t>(j)] = bottom_blob_index;
        }

        for (std::int32_t j = 0; j < top_count; ++j) {
            std::int32_t top_blob_index = 0;
            if (!read_value(&top_blob_index, sizeof(top_blob_index))) {
                MGRAPH_LOGE("read layer %d top %d failed", i, j);
                return -1;
            }
            if (top_blob_index < 0 || top_blob_index >= blob_count) {
                MGRAPH_LOGE("layer %d invalid top_blob_index %d", i, top_blob_index);
                return -1;
            }
            this->blobs[static_cast<std::size_t>(top_blob_index)].producer = i;
            spec.tops[static_cast<std::size_t>(j)] = top_blob_index;
        }

        if (spec.params.load_param_bin(reader) != 0) {
            MGRAPH_LOGE("ParamDict load_param_bin %d failed", i);
            return -1;
        }
    }

    return 0;
}

ExecutionPlan Graph::build_execution_plan() const {
    ExecutionPlan plan;
    plan.layer_order.resize(this->layers.size());
    for (std::size_t i = 0; i < this->layers.size(); ++i) {
        plan.layer_order[i] = static_cast<std::int32_t>(i);
    }

    plan.blob_last_use.resize(this->blobs.size(), -1);
    plan.input_blobs.reserve(this->blobs.size());
    plan.output_blobs.reserve(this->blobs.size());
    for (std::size_t i = 0; i < this->blobs.size(); ++i) {
        const Blob& blob = this->blobs[i];
        if (blob.producer == -1) {
            plan.input_blobs.push_back(static_cast<std::int32_t>(i));
        }
        if (blob.consumer == -1) {
            plan.output_blobs.push_back(static_cast<std::int32_t>(i));
        } else {
            plan.blob_last_use[i] = blob.consumer;
        }
    }

    return plan;
}

}  // namespace mgraph
