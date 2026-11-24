#include <cstdint>

#include <mgraph/mgraph.hpp>

int main() {
    const char text[] = "7767517 1 1 Input input 0 1 data\n";
    const auto* memory = reinterpret_cast<const std::uint8_t*>(text);
    mformat::DataReaderFromMemory reader(memory);

    mgraph::Graph graph;
    if (graph.load_param(reader) != 0) {
        return 1;
    }
    if (graph.layers.size() != 1 || graph.blobs.size() != 1 || graph.blobs[0].name != "data") {
        return 2;
    }
    return 0;
}
