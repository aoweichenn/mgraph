// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

#include <cstdint>

#include <gtest/gtest.h>
#include <mgraph/layer.hpp>
#include <mgraph/layer_factory.hpp>

namespace {

struct FakeMat {
    FakeMat clone(void*) const {
        FakeMat copy;
        copy.value = value;
        return copy;
    }

    bool empty() const { return false; }

    std::int32_t value = 0;
};

struct FakeOption {
    void* blob_allocator = nullptr;
};

using FakeLayerBase = mgraph::Layer<FakeMat, FakeOption>;

class InplaceLayer : public FakeLayerBase {
public:
    std::int32_t forward_inplace(FakeMat& mat, const FakeOption&) const override {
        mat.value += 1;
        return 0;
    }
};

struct FactoryState {
    std::int32_t created = 0;
    std::int32_t destroyed = 0;
};

FakeLayerBase* create_layer(void* userdata) {
    auto* state = static_cast<FactoryState*>(userdata);
    ++state->created;
    auto* layer = new FakeLayerBase();
    layer->userdata = userdata;
    return layer;
}

void destroy_layer(FakeLayerBase* layer, void* userdata) {
    auto* state = static_cast<FactoryState*>(userdata);
    ++state->destroyed;
    delete layer;
}

}  // namespace

TEST(Layer, DefaultFlagsAndLifecycle) {
    FakeLayerBase layer;
    EXPECT_FALSE(layer.one_blob_only);
    EXPECT_FALSE(layer.support_inplace);
    EXPECT_FALSE(layer.support_packing);
    EXPECT_EQ(layer.featmask, 0);
    EXPECT_EQ(layer.typeindex, -1);

    const mformat::ParamDict params;
    const mformat::ModelBin model_bin;
    const FakeOption option;
    EXPECT_EQ(layer.load_param(params), 0);
    EXPECT_EQ(layer.load_model(model_bin), 0);
    EXPECT_EQ(layer.create_pipeline(option), 0);
    EXPECT_EQ(layer.destroy_pipeline(option), 0);

    FakeMat bottom;
    FakeMat top;
    EXPECT_EQ(layer.forward(bottom, top, option), -1);
}

TEST(Layer, InplaceForwardClonesAndCallsInplace) {
    InplaceLayer layer;
    layer.support_inplace = true;

    FakeMat bottom;
    bottom.value = 41;
    FakeMat top;
    const FakeOption option;
    EXPECT_EQ(layer.forward(bottom, top, option), 0);
    EXPECT_EQ(top.value, 42);
}

TEST(LayerFactory, RegistersAndCreatesCustomLayer) {
    mgraph::LayerFactory<FakeLayerBase> factory(0x100);
    FactoryState state;
    ASSERT_TRUE(factory.register_custom("custom", create_layer, destroy_layer, &state));
    EXPECT_EQ(factory.custom_index("custom"), 0);

    FakeLayerBase* layer = factory.create_custom("custom");
    ASSERT_NE(layer, nullptr);
    EXPECT_EQ(layer->typeindex, 0x100);
    EXPECT_EQ(state.created, 1);
    factory.destroy(layer);
    EXPECT_EQ(state.destroyed, 1);

    factory.clear();
    EXPECT_EQ(factory.custom_index("custom"), -1);
}

TEST(LayerFactory, RegistersByIndexAndReportsEntries) {
    mgraph::LayerFactory<FakeLayerBase> factory(0x100);
    FactoryState state;
    ASSERT_TRUE(factory.register_custom(2, create_layer, destroy_layer, &state));
    ASSERT_EQ(factory.custom_entries().size(), 3U);
    EXPECT_EQ(factory.custom_entries()[2].creator, create_layer);

    FakeLayerBase* layer = factory.create_custom(2);
    ASSERT_NE(layer, nullptr);
    EXPECT_EQ(layer->typeindex, 0x102);
    factory.destroy(layer);
}

TEST(LayerFactory, OverwritesBuiltinLayer) {
    mgraph::LayerFactory<FakeLayerBase> factory(0x100);
    FactoryState state;
    factory.set_custom_bit(0x200);
    factory.register_overwrite(7, create_layer, destroy_layer, &state);
    EXPECT_EQ(factory.find_overwrite(7), 0);
    ASSERT_EQ(factory.overwrite_entries().size(), 1U);

    FakeLayerBase* layer = factory.create_overwrite(7);
    ASSERT_NE(layer, nullptr);
    EXPECT_EQ(layer->typeindex, 7);
    EXPECT_EQ(state.created, 1);
    factory.destroy(layer);
    EXPECT_EQ(state.destroyed, 1);

    factory.register_overwrite(7, create_layer, destroy_layer, &state);
    EXPECT_EQ(factory.overwrite_entries().size(), 1U);
}
