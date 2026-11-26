// Copyright 2017 Tencent
// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MGRAPH_EXECUTOR_HPP
#define MGRAPH_EXECUTOR_HPP

#include <cstdint>
#include <cstdio>
#include <cmath>
#include <cstdlib>

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

    // Whether a bottom blob may be released after its consumer succeeded.
    [[nodiscard]] virtual bool should_release_bottom(const MatT& blob, const OptionT& opt) const = 0;
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
        static const bool trace_enter_enabled = std::getenv("MGRAPH_TRACE") != nullptr;
        if (trace_enter_enabled) {
            std::fprintf(stderr, "[enter] layer %d\n", layer_index);
        }
        const std::int32_t ret = this->backend_.invoke(layer, blob_mats, layer_opt);
        if (ret != 0) {
            // Name the failing layer: without it a broken graph can only be
            // bisected by hand.
            std::fprintf(stderr, "[mgraph] layer %d failed (ret=%d) bottoms:", layer_index, ret);
            for (std::int32_t bottom_blob_index : layer->bottoms) {
                if (bottom_blob_index < 0 || static_cast<std::size_t>(bottom_blob_index) >= blob_mats.size()) {
                    std::fprintf(stderr, " %d:?", bottom_blob_index);
                    continue;
                }
                const MatT& m = blob_mats[static_cast<std::size_t>(bottom_blob_index)];
                std::fprintf(stderr, " %d:%dx%dx%d", bottom_blob_index, m.w, m.h, m.c);
            }
            std::fprintf(stderr, "\n");
            return ret;
        }

        // Opt-in per-layer shape trace: MGRAPH_TRACE=1 prints every produced
        // blob, which is the quickest way to find where a shape chain breaks.
        static const bool trace_enabled = std::getenv("MGRAPH_TRACE") != nullptr;
        if (trace_enabled) {
            std::fprintf(stderr, "[trace] layer %d tops:", layer_index);
            for (std::int32_t top_blob_index : layer->tops) {
                if (top_blob_index < 0 || static_cast<std::size_t>(top_blob_index) >= blob_mats.size()) {
                    std::fprintf(stderr, " %d:?", top_blob_index);
                    continue;
                }
                const MatT& m = blob_mats[static_cast<std::size_t>(top_blob_index)];
                std::fprintf(stderr, " %d:%dx%dx%d", top_blob_index, m.w, m.h, m.c);
                if (m.elemsize == 4u && m.data != nullptr) {
                    const auto* ptr = static_cast<const float*>(m.data);
                    const std::size_t count = m.cstep * static_cast<std::size_t>(m.c);
                    double absmax = 0.0;
                    std::size_t bad = 0;
                    for (std::size_t i = 0; i < count; ++i) {
                        const float v = ptr[i];
                        if (!std::isfinite(v)) {
                            ++bad;
                        } else if (static_cast<double>(std::fabs(v)) > absmax) {
                            absmax = static_cast<double>(std::fabs(v));
                        }
                    }
                    std::fprintf(stderr, " max=%.3e bad=%zu", absmax, bad);
                }
            }
            std::fprintf(stderr, "\n");
        }

        for (std::int32_t bottom_blob_index : layer->bottoms) {
            if (bottom_blob_index >= 0 && static_cast<std::size_t>(bottom_blob_index) < blob_mats.size() &&
                this->backend_.should_release_bottom(blob_mats[static_cast<std::size_t>(bottom_blob_index)], opt)) {
                blob_mats[static_cast<std::size_t>(bottom_blob_index)].release();
            }
        }

        return 0;
    }

private:
    const std::vector<LayerT*>& layers_;
    const std::vector<Blob>& blobs_;
    const ExecutionBackend<LayerT, MatT, OptionT>& backend_;
};

}  // namespace mgraph

#endif  // MGRAPH_EXECUTOR_HPP
