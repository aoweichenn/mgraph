// Copyright 2017 Tencent
// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MGRAPH_LAYER_FACTORY_HPP
#define MGRAPH_LAYER_FACTORY_HPP

#include <cstdint>

#include <string>
#include <vector>

namespace mgraph {

// Registration and creation of custom / overwritten built-in layers.
// LayerT stays consumer-defined; only the registry bookkeeping lives here.
template<typename LayerT>
class LayerFactory {
public:
    using Creator = LayerT* (*)(void*);
    using Destroyer = void (*)(LayerT*, void*);

    struct CustomEntry {
        std::string name;
        Creator creator;
        Destroyer destroyer;
        void* userdata;
    };

    struct OverwriteEntry {
        std::int32_t typeindex;
        Creator creator;
        Destroyer destroyer;
        void* userdata;
    };

    explicit LayerFactory(std::int32_t custom_bit = 0) : custom_bit_(custom_bit) {}

    void set_custom_bit(std::int32_t custom_bit) { this->custom_bit_ = custom_bit; }

    // Register by name: overwrite an existing name or append a new custom index.
    // Return false when the same name is already registered.
    bool register_custom(const char* name, Creator creator, Destroyer destroyer, void* userdata) {
        const std::int32_t index = this->custom_index(name);
        if (index >= 0) {
            CustomEntry& entry = this->custom_entries_[static_cast<std::size_t>(index)];
            entry.creator = creator;
            entry.destroyer = destroyer;
            entry.userdata = userdata;
            return true;
        }

        CustomEntry entry;
        entry.name = name != nullptr ? name : "";
        entry.creator = creator;
        entry.destroyer = destroyer;
        entry.userdata = userdata;
        this->custom_entries_.push_back(entry);
        return true;
    }

    // Register by custom index, growing the table with empty slots.
    bool register_custom(std::int32_t index, Creator creator, Destroyer destroyer, void* userdata) {
        if (index < 0) {
            return false;
        }
        if (static_cast<std::size_t>(index) >= this->custom_entries_.size()) {
            this->custom_entries_.resize(static_cast<std::size_t>(index) + 1);
        }
        CustomEntry& entry = this->custom_entries_[static_cast<std::size_t>(index)];
        entry.creator = creator;
        entry.destroyer = destroyer;
        entry.userdata = userdata;
        return true;
    }

    void register_overwrite(std::int32_t typeindex, Creator creator, Destroyer destroyer, void* userdata) {
        const std::int32_t index = this->find_overwrite(typeindex);
        if (index >= 0) {
            OverwriteEntry& entry = this->overwrite_entries_[static_cast<std::size_t>(index)];
            entry.creator = creator;
            entry.destroyer = destroyer;
            entry.userdata = userdata;
            return;
        }

        OverwriteEntry entry;
        entry.typeindex = typeindex;
        entry.creator = creator;
        entry.destroyer = destroyer;
        entry.userdata = userdata;
        this->overwrite_entries_.push_back(entry);
    }

    [[nodiscard]] std::int32_t custom_index(const char* name) const {
        if (name == nullptr) {
            return -1;
        }
        for (std::size_t i = 0; i < this->custom_entries_.size(); ++i) {
            if (this->custom_entries_[i].name == name) {
                return static_cast<std::int32_t>(i);
            }
        }
        return -1;
    }

    [[nodiscard]] std::int32_t find_overwrite(std::int32_t typeindex) const {
        for (std::size_t i = 0; i < this->overwrite_entries_.size(); ++i) {
            if (this->overwrite_entries_[i].typeindex == typeindex) {
                return static_cast<std::int32_t>(i);
            }
        }
        return -1;
    }

    [[nodiscard]] LayerT* create_custom(const char* name) const {
        return this->create_custom(this->custom_index(name));
    }

    [[nodiscard]] LayerT* create_custom(std::int32_t index) const {
        if (index < 0 || static_cast<std::size_t>(index) >= this->custom_entries_.size()) {
            return nullptr;
        }
        const CustomEntry& entry = this->custom_entries_[static_cast<std::size_t>(index)];
        if (entry.creator == nullptr) {
            return nullptr;
        }
        LayerT* layer = entry.creator(entry.userdata);
        if (layer != nullptr) {
            layer->typeindex = this->custom_bit_ | index;
        }
        return layer;
    }

    [[nodiscard]] LayerT* create_overwrite(std::int32_t typeindex) const {
        const std::int32_t index = this->find_overwrite(typeindex);
        if (index < 0) {
            return nullptr;
        }
        const OverwriteEntry& entry = this->overwrite_entries_[static_cast<std::size_t>(index)];
        if (entry.creator == nullptr) {
            return nullptr;
        }
        LayerT* layer = entry.creator(entry.userdata);
        if (layer != nullptr) {
            layer->typeindex = typeindex;
        }
        return layer;
    }

    void destroy(LayerT* layer) const {
        if (layer == nullptr) {
            return;
        }
        if ((layer->typeindex & this->custom_bit_) != 0) {
            const std::int32_t index = layer->typeindex & ~this->custom_bit_;
            if (index >= 0 && static_cast<std::size_t>(index) < this->custom_entries_.size()) {
                const CustomEntry& entry = this->custom_entries_[static_cast<std::size_t>(index)];
                if (entry.destroyer != nullptr) {
                    entry.destroyer(layer, entry.userdata);
                    return;
                }
            }
        } else {
            const std::int32_t index = this->find_overwrite(layer->typeindex);
            if (index >= 0) {
                const OverwriteEntry& entry = this->overwrite_entries_[static_cast<std::size_t>(index)];
                if (entry.destroyer != nullptr) {
                    entry.destroyer(layer, entry.userdata);
                    return;
                }
            }
        }
        delete layer;
    }

    void clear() {
        this->custom_entries_.clear();
        this->overwrite_entries_.clear();
    }

    [[nodiscard]] const std::vector<CustomEntry>& custom_entries() const { return this->custom_entries_; }

    [[nodiscard]] const std::vector<OverwriteEntry>& overwrite_entries() const { return this->overwrite_entries_; }

private:
    std::int32_t custom_bit_;
    std::vector<CustomEntry> custom_entries_;
    std::vector<OverwriteEntry> overwrite_entries_;
};

}  // namespace mgraph

#endif  // MGRAPH_LAYER_FACTORY_HPP
