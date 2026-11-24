// Copyright 2017 Tencent
// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MGRAPH_EXTRACTOR_HPP
#define MGRAPH_EXTRACTOR_HPP

#include <cstdint>

#include <vector>

#include <mgraph/executor.hpp>
#include <mgraph/graph.hpp>
#include <mgraph/output_backend.hpp>

namespace mgraph {

// Blob store + lazy extraction orchestration. Layer invocation is handled by
// Executor/ExecutionBackend; output post-processing stays in the consumer.
template<typename LayerT, typename MatT, typename OptionT>
class Extractor {
public:
    Extractor(const std::vector<LayerT*>& layers, const std::vector<Blob>& blobs,
              const ExecutionBackend<LayerT, MatT, OptionT>& backend, std::vector<MatT>& blob_mats, OptionT& opt)
            : executor_(layers, blobs, backend), blobs_(blobs), blob_mats_(blob_mats), opt_(opt) {}

    std::int32_t input(std::int32_t blob_index, const MatT& in) {
        if (blob_index < 0 || static_cast<std::size_t>(blob_index) >= this->blob_mats_.size()) {
            return -1;
        }
        this->blob_mats_[static_cast<std::size_t>(blob_index)] = in;
        return 0;
    }

    std::int32_t ensure(std::int32_t blob_index) {
        if (blob_index < 0 || static_cast<std::size_t>(blob_index) >= this->blob_mats_.size() ||
            static_cast<std::size_t>(blob_index) >= this->blobs_.size()) {
            return -1;
        }

        if (this->blob_mats_[static_cast<std::size_t>(blob_index)].dims == 0) {
            const std::int32_t producer = this->blobs_[static_cast<std::size_t>(blob_index)].producer;
            if (producer < 0) {
                return -1;
            }
            return this->executor_.forward(producer, this->blob_mats_, this->opt_);
        }

        return 0;
    }

    template<FinalizeCallable<MatT> FinalizeFn>
    std::int32_t finalize(std::int32_t blob_index, MatT& feat, std::int32_t& type, FinalizeFn&& finalize_fn) {
        if (blob_index < 0 || static_cast<std::size_t>(blob_index) >= this->blob_mats_.size()) {
            return -1;
        }
        feat = this->blob_mats_[static_cast<std::size_t>(blob_index)];
        if (feat.empty()) {
            return 0;
        }
        return finalize_fn(blob_index, feat, type);
    }

    std::int32_t finalize(std::int32_t blob_index, MatT& feat, std::int32_t& type, OutputBackend<MatT>& backend) {
        if (blob_index < 0 || static_cast<std::size_t>(blob_index) >= this->blob_mats_.size()) {
            return -1;
        }
        feat = this->blob_mats_[static_cast<std::size_t>(blob_index)];
        if (feat.empty()) {
            return 0;
        }
        return backend.finalize(blob_index, feat, type);
    }

    [[nodiscard]] std::vector<MatT>& blob_mats() { return this->blob_mats_; }

    [[nodiscard]] const std::vector<MatT>& blob_mats() const { return this->blob_mats_; }

private:
    Executor<LayerT, MatT, OptionT> executor_;
    const std::vector<Blob>& blobs_;
    std::vector<MatT>& blob_mats_;
    OptionT& opt_;
};

}  // namespace mgraph

#endif  // MGRAPH_EXTRACTOR_HPP
