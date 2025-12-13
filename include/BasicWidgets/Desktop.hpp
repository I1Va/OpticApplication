#pragma once
#include <iostream>
#include "BasicWidgets/Containers.hpp"

namespace roa
{

class Desktop final : public Container {
public:
    using Container::Container;
    Desktop(const Container&) = delete;
    ~Desktop() = default;  
    Desktop& operator=(const Desktop&) = delete;
    Desktop(Desktop&&) = default;
    Desktop& operator=(Desktop&&) = default;

protected:
    void Redraw() const override {
        GetTexture().Clear({61, 61, 61});

        for (auto &child : children) {
            child->DrawOn(GetTexture());
        }
    }
};

} // namespace roa
