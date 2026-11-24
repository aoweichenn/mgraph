// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

#include <cstdint>

#include <vector>

#include <gtest/gtest.h>
#include <mgraph/executor.hpp>
#include <mgraph/extractor.hpp>

namespace {

struct FakeMat {
    std::int32_t dims = 0;
    std::int32_t value = 0;
    bool released = false;

    bool empty() const { return dims == 0; }

    void release() {
        dims = 0;
        value = 0;
        released = true;
    }
};

struct FakeOption {
    std::int32_t num_threads = 1;
    bool masked = false;
};

struct FakeLayer {
    std::int32_t typeindex = -1;
    std::int32_t featmask = 0;
    std::vector<std::int32_t> bottoms;
    std::vector<std::int32_t> tops;
};

class RecordingBackend : public mgraph::ExecutionBackend<FakeLayer, FakeMat, FakeOption> {
public:
    bool is_input_layer(const FakeLayer& layer) const override { return layer.typeindex == 99; }

    FakeOption mask_option(const FakeOption& opt, std::int32_t featmask) const override {
        FakeOption masked = opt;
        masked.masked = (featmask & 1) != 0;
        masked.num_threads = 1;
        return masked;
    }

    bool should_release_bottom(const FakeMat&, const FakeOption&) const override { return true; }

    std::int32_t invoke(const FakeLayer* layer, std::vector<FakeMat>& blob_mats, const FakeOption& opt) const override {
        this->order.push_back(layer->typeindex);
        const std::int32_t top = layer->tops[0];
        blob_mats[static_cast<std::size_t>(top)].dims = 1;
        blob_mats[static_cast<std::size_t>(top)].value = opt.masked ? 100 + layer->typeindex : layer->typeindex;
        return 0;
    }

    mutable std::vector<std::int32_t> order;
};

}  // namespace

TEST(Executor, RunsLayersInDependencyOrder) {
    FakeLayer input;
    input.typeindex = 99;
    input.tops = {0};

    FakeLayer conv;
    conv.typeindex = 1;
    conv.bottoms = {0};
    conv.tops = {1};

    FakeLayer relu;
    relu.typeindex = 2;
    relu.bottoms = {1};
    relu.tops = {2};
    relu.featmask = 1;

    const std::vector<FakeLayer*> layers = {&input, &conv, &relu};
    const std::vector<mgraph::Blob> blobs = {{"data", 0, 1}, {"conv", 1, 2}, {"out", 2, -1}};

    RecordingBackend backend;
    mgraph::Executor<FakeLayer, FakeMat, FakeOption> executor(layers, blobs, backend);
    std::vector<FakeMat> blob_mats(3);
    FakeOption opt;

    ASSERT_EQ(executor.forward(2, blob_mats, opt), 0);
    ASSERT_EQ(backend.order.size(), 2U);
    EXPECT_EQ(backend.order[0], 1);
    EXPECT_EQ(backend.order[1], 2);
    EXPECT_EQ(blob_mats[2].value, 102);
    EXPECT_TRUE(blob_mats[0].released);
    EXPECT_TRUE(blob_mats[1].released);
}

TEST(Executor, RejectsInvalidLayerAndMissingProducer) {
    FakeLayer layer;
    layer.typeindex = 1;
    layer.bottoms = {0};
    layer.tops = {1};
    const std::vector<FakeLayer*> layers = {&layer};
    const std::vector<mgraph::Blob> blobs = {{"orphan", -1, 0}, {"out", 0, -1}};

    RecordingBackend backend;
    mgraph::Executor<FakeLayer, FakeMat, FakeOption> executor(layers, blobs, backend);
    std::vector<FakeMat> blob_mats(2);
    FakeOption opt;

    EXPECT_NE(executor.forward(-1, blob_mats, opt), 0);
    EXPECT_NE(executor.forward(1, blob_mats, opt), 0);
    EXPECT_NE(executor.forward(0, blob_mats, opt), 0);
    EXPECT_TRUE(backend.order.empty());
}

TEST(Executor, BackendMasksOptionWithoutFeatmask) {
    FakeLayer layer;
    layer.typeindex = 3;
    layer.bottoms = {};
    layer.tops = {0};
    const std::vector<FakeLayer*> layers = {&layer};
    const std::vector<mgraph::Blob> blobs = {{"out", 0, -1}};

    RecordingBackend backend;
    mgraph::Executor<FakeLayer, FakeMat, FakeOption> executor(layers, blobs, backend);
    std::vector<FakeMat> blob_mats(1);
    FakeOption opt;

    ASSERT_EQ(executor.forward(0, blob_mats, opt), 0);
    EXPECT_EQ(blob_mats[0].value, 3);
}

TEST(Extractor, InputsAndLazilyProducesOutput) {
    FakeLayer input;
    input.typeindex = 99;
    input.tops = {0};

    FakeLayer conv;
    conv.typeindex = 1;
    conv.bottoms = {0};
    conv.tops = {1};

    const std::vector<FakeLayer*> layers = {&input, &conv};
    const std::vector<mgraph::Blob> blobs = {{"data", 0, 1}, {"out", 1, -1}};

    RecordingBackend backend;
    std::vector<FakeMat> blob_mats(2);
    FakeOption opt;
    mgraph::Extractor<FakeLayer, FakeMat, FakeOption> extractor(layers, blobs, backend, blob_mats, opt);

    EXPECT_EQ(extractor.input(0, FakeMat{1, 5}), 0);
    EXPECT_EQ(extractor.ensure(1), 0);
    EXPECT_EQ(blob_mats[1].dims, 1);
    EXPECT_EQ(blob_mats[1].value, 1);
    ASSERT_EQ(backend.order.size(), 1U);
    EXPECT_EQ(extractor.blob_mats().size(), 2U);

    EXPECT_NE(extractor.input(-1, FakeMat()), 0);
    EXPECT_NE(extractor.ensure(2), 0);
}

TEST(Extractor, FinalizeStageRunsForNonEmptyOutput) {
    FakeLayer input;
    input.typeindex = 99;
    input.tops = {0};

    const std::vector<FakeLayer*> layers = {&input};
    const std::vector<mgraph::Blob> blobs = {{"data", 0, -1}};

    RecordingBackend backend;
    std::vector<FakeMat> blob_mats(1);
    blob_mats[0].dims = 1;
    blob_mats[0].value = 7;
    FakeOption opt;
    mgraph::Extractor<FakeLayer, FakeMat, FakeOption> extractor(layers, blobs, backend, blob_mats, opt);

    std::int32_t calls = 0;
    std::int32_t type = 0;
    FakeMat feat;
    EXPECT_EQ(extractor.finalize(0, feat, type,
                                 [&](std::int32_t, FakeMat& out, std::int32_t& out_type) {
                                     ++calls;
                                     out_type = 1;
                                     out.value += 1;
                                     return 0;
                                 }),
              0);
    EXPECT_EQ(calls, 1);
    EXPECT_EQ(feat.value, 8);
    EXPECT_EQ(type, 1);

    EXPECT_NE(extractor.finalize(1, feat, type, [](std::int32_t, FakeMat&, std::int32_t&) { return 0; }), 0);
}
