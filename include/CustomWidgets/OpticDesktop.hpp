#pragma once

#include <unordered_map>
#include <string>

#include "cum/manager.hpp"
#include "cum/plugin.hpp"
#include "cum/ifc/pp.hpp"
#include "BasicWidgets/Desktop.hpp"

namespace roa
{

class OpticDesktop final : public Desktop {
    cum::Manager *pluginManager;
    std::unordered_map<std::string, cum::PPToolPlugin*> PPTable;

public:
    OpticDesktop(hui::UI *ui, cum::Manager *pluginManager_);
    ~OpticDesktop() override = default;

    bool ExistsPPPlugin(const std::string &path);
    bool LoadPPPlugin(const std::string &path);
};

} // namespace roa
