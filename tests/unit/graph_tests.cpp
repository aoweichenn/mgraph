// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

#include <cstdint>
#include <cstring>

#include <vector>

#include <gtest/gtest.h>
#include <mgraph/graph.hpp>

namespace {

void append_i32(std::vector<std::uint8_t>& data, std::int32_t value) {
    const auto* bytes = reinterpret_cast<const std::uint8_t*>(&value);
    data.insert(data.end(), bytes, bytes + sizeof(value));
}

}  // namespace

TEST(Graph, ParsesTextTopologyAndParams) {
    const char text[] =
        "7767517 3 3\n"
        "Input input 0 1 data\n"
        "Convolution conv 1 1 data conv1 0=1 1=1\n"
        "ReLU relu 1 1 conv1 output\n";
    const auto* memory = reinterpret_cast<const std::uint8_t*>(text);
    mformat::DataReaderFromMemory reader(memory);

    mgraph::Graph graph;
    ASSERT_EQ(graph.load_param(reader), 0);
    ASSERT_EQ(graph.layers.size(), 3U);
    ASSERT_EQ(graph.blobs.size(), 3U);

    EXPECT_EQ(graph.layers[0].type, "Input");
    EXPECT_EQ(graph.layers[0].name, "input");
    EXPECT_EQ(graph.layers[0].type_index, -1);
    EXPECT_EQ(graph.layers[0].tops.size(), 1U);
    EXPECT_EQ(graph.layers[1].bottoms[0], 0);
    EXPECT_EQ(graph.layers[1].tops[0], 1);
    EXPECT_EQ(graph.layers[2].bottoms[0], 1);

    EXPECT_EQ(graph.blobs[0].name, "data");
    EXPECT_EQ(graph.blobs[0].producer, 0);
    EXPECT_EQ(graph.blobs[0].consumer, 1);
    EXPECT_EQ(graph.blobs[1].name, "conv1");
    EXPECT_EQ(graph.blobs[2].name, "output");
    EXPECT_EQ(graph.blobs[2].consumer, -1);

    EXPECT_EQ(graph.layers[1].params.get(0, 0), 1);
    EXPECT_EQ(graph.layers[1].params.get(1, 0), 1);

    graph.clear();
    EXPECT_TRUE(graph.layers.empty());
    EXPECT_TRUE(graph.blobs.empty());
}

TEST(Graph, ParsesBinaryTopology) {
    std::vector<std::uint8_t> data;
    append_i32(data, 7767517);
    append_i32(data, 2);
    append_i32(data, 2);
    append_i32(data, 0);
    append_i32(data, 0);
    append_i32(data, 1);
    append_i32(data, 0);
    append_i32(data, -233);
    append_i32(data, 1);
    append_i32(data, 1);
    append_i32(data, 1);
    append_i32(data, 0);
    append_i32(data, 1);
    append_i32(data, -233);

    const std::uint8_t* memory = data.data();
    mformat::DataReaderFromMemory reader(memory, data.size());

    mgraph::Graph graph;
    ASSERT_EQ(graph.load_param_bin(reader), 0);
    ASSERT_EQ(graph.layers.size(), 2U);
    EXPECT_EQ(graph.layers[0].type_index, 0);
    EXPECT_EQ(graph.layers[1].type_index, 1);
    EXPECT_EQ(graph.layers[1].bottoms[0], 0);
    EXPECT_EQ(graph.layers[1].tops[0], 1);
    EXPECT_EQ(graph.blobs[0].producer, 0);
    EXPECT_EQ(graph.blobs[0].consumer, 1);
    EXPECT_EQ(graph.blobs[1].producer, 1);
}

TEST(Graph, BuildsExecutionPlan) {
    const char text[] =
        "7767517 3 3\n"
        "Input input 0 1 data\n"
        "Convolution conv 1 1 data conv1 0=1 1=1\n"
        "ReLU relu 1 1 conv1 output\n";
    const auto* memory = reinterpret_cast<const std::uint8_t*>(text);
    mformat::DataReaderFromMemory reader(memory);

    mgraph::Graph graph;
    ASSERT_EQ(graph.load_param(reader), 0);
    const mgraph::ExecutionPlan plan = graph.build_execution_plan();
    ASSERT_EQ(plan.layer_order.size(), 3U);
    EXPECT_EQ(plan.layer_order[0], 0);
    EXPECT_EQ(plan.layer_order[2], 2);
    ASSERT_EQ(plan.blob_last_use.size(), 3U);
    EXPECT_EQ(plan.blob_last_use[0], 1);
    EXPECT_EQ(plan.blob_last_use[1], 2);
    EXPECT_EQ(plan.blob_last_use[2], -1);
    EXPECT_EQ(plan.input_blobs, std::vector<std::int32_t>{});
    ASSERT_EQ(plan.output_blobs.size(), 1U);
    EXPECT_EQ(plan.output_blobs[0], 2);
}

TEST(Graph, RejectsOldMagic) {
    const char text[] = "7767516 1 1 Input input 0 1 data\n";
    const auto* memory = reinterpret_cast<const std::uint8_t*>(text);
    mformat::DataReaderFromMemory reader(memory);
    mgraph::Graph graph;
    EXPECT_NE(graph.load_param(reader), 0);
    EXPECT_TRUE(graph.layers.empty());
}
