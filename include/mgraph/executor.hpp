// Copyright 2017 Tencent
// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MGRAPH_EXECUTOR_HPP
#define MGRAPH_EXECUTOR_HPP

#include <cstdint>

#include <vector>

#include <mgraph/graph.hpp>

namespace mgraph {

// Consumer policy for the generic execution loop. The concrete layer call
// (including in-place/packing/benchmark details) stays in the consumer.
template<typename LayerT, typename MatT, typename OptionT>
class ExecutionBackend {
public:
    virtual ~ExecutionBackend() = default;

    [[nodiscard]] virtual bool is_input_layer(const LayerT& layer) const = 0;
    [[nodiscard]] virtual OptionT mask_option(const OptionT& opt, std::int32_t featmask) const = 0;
    virtual std::int32_t invoke(const LayerT* layer, std::vector<MatT>& blob_mats, const OptionT& opt) const = 0;
};

// Recursive layer execution with lazy blob production. Blob lifetime and
// per-layer invocation are delegated to the backend.
template<typename LayerT, typename MatT, typename OptionT>
class Executor {
public:
    Executor(const std::vector<LayerT*>& layers, const std::vector<Blob>& blobs,
             const ExecutionBackend<LayerT, MatT, OptionT>& backend)
            : layers_(layers), blobs_(blobs), backend_(backend) {}

    std::int32_t forward(std::int32_t layer_index, std::vector<MatT>& blob_mats, const OptionT& opt) const {
        if (layer_index < 0 || static_cast<std::size_t>(layer_index) >= this->layers_.size() ||
            this->layers_[static_cast<std::size_t>(layer_index)] == nullptr) {
            return -1;
        }

        const LayerT* layer = this->layers_[static_cast<std::size_t>(layer_index)];
        if (this->backend_.is_input_layer(*layer)) {
            return 0;
        }

        for (std::int32_t bottom_blob_index : layer->bottoms) {
            if (bottom_blob_index < 0 || static_cast<std::size_t>(bottom_blob_index) >= blob_mats.size()) {
                return -1;
            }
            if (blob_mats[static_cast<std::size_t>(bottom_blob_index)].dims == 0) {
                if (static_cast<std::size_t>(bottom_blob_index) >= this->blobs_.size()) {
                    return -1;
                }
                const std::int32_t producer = this->blobs_[static_cast<std::size_t>(bottom_blob_index)].producer;
                if (producer < 0 || static_cast<std::size_t>(producer) >= this->layers_.size()) {
                    return -1;
                }
                const std::int32_t ret = this->forward(producer, blob_mats, opt);
                if (ret != 0) {
                    return ret;
                }
            }
        }

        const OptionT layer_opt = layer->featmask != 0 ? this->backend_.mask_option(opt, layer->featmask) : opt;
        return this->backend_.invoke(layer, blob_mats, layer_opt);
    }

private:
    const std::vector<LayerT*>& layers_;
    const std::vector<Blob>& blobs_;
    const ExecutionBackend<LayerT, MatT, OptionT>& backend_;
};

}  // namespace mgraph

#endif  // MGRAPH_EXECUTOR_HPP
