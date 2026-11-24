// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MGRAPH_OUTPUT_BACKEND_HPP
#define MGRAPH_OUTPUT_BACKEND_HPP

#include <cstdint>

#include <concepts>

namespace mgraph {

// Consumer policy applied when an extracted blob leaves the graph, for
// example unpacking, float16/bfloat16/int8 conversion or allocator detach.
// The graph owns the extraction stage; the backend owns the layout/cast rules.
template<typename MatT>
class OutputBackend {
public:
    virtual ~OutputBackend() = default;

    // `blob_index` identifies the graph node that produced the already-copied
    // `feat`; implementations may adjust `feat` and `type` in place.
    virtual std::int32_t finalize(std::int32_t blob_index, MatT& feat, std::int32_t& type) = 0;
};

// A callable with the same contract as OutputBackend::finalize.
template<typename Fn, typename MatT>
concept FinalizeCallable = requires(Fn&& fn, std::int32_t blob_index, MatT& mat, std::int32_t& type) {
    { fn(blob_index, mat, type) } -> std::convertible_to<std::int32_t>;
};

}  // namespace mgraph

#endif  // MGRAPH_OUTPUT_BACKEND_HPP
