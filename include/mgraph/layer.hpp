// Copyright 2017 Tencent
// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MGRAPH_LAYER_HPP
#define MGRAPH_LAYER_HPP

#include <cstdint>

#include <string>
#include <vector>

#include <mformat/model_bin.hpp>
#include <mformat/param_dict.hpp>

namespace mgraph {

// Type-parameterised layer base. MatT and OptionT stay consumer-defined so this
// header does not depend on ncnn or Vulkan types.
template<typename MatT, typename OptionT>
class Layer {
public:
    Layer() {
        this->one_blob_only = false;
        this->support_inplace = false;
        this->support_vulkan = false;
        this->support_packing = false;
        this->support_bf16_storage = false;
        this->support_fp16_storage = false;
        this->support_int8_storage = false;
        this->support_tensor_storage = false;
        this->support_vulkan_packing = false;
        this->support_any_packing = false;
        this->support_vulkan_any_packing = false;
        this->support_batch = false;
        this->support_reserved_2 = false;
        this->support_reserved_3 = false;
        this->support_reserved_4 = false;
        this->support_reserved_5 = false;
        this->support_reserved_6 = false;
        this->support_reserved_7 = false;
        this->support_reserved_8 = false;
        this->support_reserved_9 = false;
        this->featmask = 0;
        this->userdata = nullptr;
        this->typeindex = -1;
    }

    virtual ~Layer() = default;

    virtual std::int32_t load_param(const mformat::ParamDict&) { return 0; }

    virtual std::int32_t load_model(const mformat::ModelBin&) { return 0; }

    virtual std::int32_t create_pipeline(const OptionT&) { return 0; }

    virtual std::int32_t destroy_pipeline(const OptionT&) { return 0; }

    virtual std::int32_t forward(const std::vector<MatT>& bottom_blobs, std::vector<MatT>& top_blobs,
                                 const OptionT& opt) const {
        if (!this->support_inplace) {
            return -1;
        }

        top_blobs = bottom_blobs;
        for (std::int32_t i = 0; i < static_cast<std::int32_t>(top_blobs.size()); ++i) {
            top_blobs[static_cast<std::size_t>(i)] =
                bottom_blobs[static_cast<std::size_t>(i)].clone(opt.blob_allocator);
            if (top_blobs[static_cast<std::size_t>(i)].empty()) {
                return -100;
            }
        }

        return this->forward_inplace(top_blobs, opt);
    }

    virtual std::int32_t forward(const MatT& bottom_blob, MatT& top_blob, const OptionT& opt) const {
        if (!this->support_inplace) {
            return -1;
        }

        top_blob = bottom_blob.clone(opt.blob_allocator);
        if (top_blob.empty()) {
            return -100;
        }

        return this->forward_inplace(top_blob, opt);
    }

    virtual std::int32_t forward_inplace(std::vector<MatT>&, const OptionT&) const { return -1; }

    virtual std::int32_t forward_inplace(MatT&, const OptionT&) const { return -1; }

    // Graph-level activation fusion: an activation layer reports the fused
    // activation type it is equivalent to, a convolution accepts it. Both
    // default to "not supported" so unused layers stay untouched.
    virtual std::int32_t get_fused_activation(std::int32_t& /*type*/, MatT& /*params*/) const { return -1; }

    virtual bool can_fuse_activation(std::int32_t /*type*/) const { return false; }

    virtual void set_fused_activation(std::int32_t /*type*/, const MatT& /*params*/) {}

public:
    bool one_blob_only;
    bool support_inplace;
    bool support_vulkan;
    bool support_packing;
    bool support_bf16_storage;
    bool support_fp16_storage;
    bool support_int8_storage;
    bool support_tensor_storage;
    bool support_vulkan_packing;
    bool support_any_packing;
    bool support_vulkan_any_packing;
    bool support_batch;
    bool support_reserved_2;
    bool support_reserved_3;
    bool support_reserved_4;
    bool support_reserved_5;
    bool support_reserved_6;
    bool support_reserved_7;
    bool support_reserved_8;
    bool support_reserved_9;

    std::int32_t featmask;

    void* userdata;
    std::int32_t typeindex;

    std::string type;
    std::string name;

    std::vector<std::int32_t> bottoms;
    std::vector<std::int32_t> tops;
    std::vector<MatT> bottom_shapes;
    std::vector<MatT> top_shapes;
};

}  // namespace mgraph

#endif  // MGRAPH_LAYER_HPP
