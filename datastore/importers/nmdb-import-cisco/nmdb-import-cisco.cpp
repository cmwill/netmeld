// =============================================================================
// Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// =============================================================================
// Maintained by Sandia National Laboratories <Netmeld@sandia.gov>
// =============================================================================

#include <netmeld/datastore/tools/AbstractImportTool.hpp>

#include "Parser.hpp"

namespace nmdo = netmeld::datastore::objects;
namespace nmdt = netmeld::datastore::tools;


// =============================================================================
// Import tool definition
// =============================================================================
template<typename P, typename R>
class Tool : public nmdt::AbstractImportTool<P,R>
{
  // ===========================================================================
  // Variables
  // ===========================================================================
  private: // Variables should generally be private
  protected: // Variables intended for internal/subclass API
  public: // Variables should rarely appear at this scope


  // ===========================================================================
  // Constructors
  // ===========================================================================
  private: // Constructors should rarely appear at this scope
  protected: // Constructors intended for internal/subclass API
  public: // Constructors should generally be public
    Tool() : nmdt::AbstractImportTool<P,R>
      (
       // command line tool imports data from
       "IOS|NXOS|ASA: show running-config all",
       PROGRAM_NAME,     // program name (set in CMakeLists.txt)
       PROGRAM_VERSION   // program version (set in CMakeLists.txt)
      )
    {}


  // ===========================================================================
  // Methods
  // ===========================================================================
  private: // Methods part of internal API
    // Overriden from AbstractImportTool
    void
    specificInserts(pqxx::transaction_base& t) override
    {
      const auto& toolRunId        {this->getToolRunId()};
      const auto& defaultDeviceId  {this->getDeviceId()};

      bool first {true};

      LOG_DEBUG << "Iterating over results\n";
      for (auto& results : this->tResults) {
        auto deviceId {defaultDeviceId};

        LOG_DEBUG << "Adding device information\n";
        auto& devInfo {results.devInfo};
        if (!first && !devInfo.getDeviceId().empty()) {
          deviceId = defaultDeviceId + ":" + devInfo.getDeviceId();
        }
        devInfo.setDeviceId(deviceId);
        if (this->opts.exists("device-type") &&
            devInfo.getDeviceType().empty())
        {
          devInfo.setDeviceType(this->opts.getValue("device-type"));
        }
        if (this->opts.exists("device-color")) {
          devInfo.setDeviceColor(this->opts.getValue("device-color"));
        }
        devInfo.save(t, toolRunId);
        LOG_DEBUG << devInfo.toDebugString() << '\n';

        LOG_DEBUG << "Iteration over DNS search domains\n";
        for (auto& dnsSearchDomain : results.dnsSearchDomains) {
          t.exec_prepared("insert_raw_device_dns_search_domain",
              toolRunId,
              deviceId,
              dnsSearchDomain);
        }

        // 04-03-2019 NOTE: Manually saving here because we do not have nor
        LOG_DEBUG << "Iteration over AAAs\n";
        for (auto& result : results.aaas) {
          // 04-03-2019 NOTE: Manually saving here because we do not have nor
          // do we want a netmeld datastore object for AAA entries at this time.
          t.exec_prepared("insert_raw_device_aaa",
              toolRunId,
              deviceId,
              result);

          LOG_DEBUG << result << '\n';
        }

        LOG_DEBUG << "Iterating over Services\n";
        for (auto& result : results.services) {
          result.save(t, toolRunId, "");
          LOG_DEBUG << result.toDebugString() << '\n';
        }

        LOG_DEBUG << "Iterating over routes\n";
        for (auto& result : results.routes) {
          result.save(t, toolRunId, deviceId);
          LOG_DEBUG << result.toDebugString() << '\n';
        }

        LOG_DEBUG << "Iterating over vlans\n";
        for (auto& result : results.vlans) {
          result.save(t, toolRunId, deviceId);
          LOG_DEBUG << result.toDebugString() << '\n';
        }

        LOG_DEBUG << "Iterating over interfaces\n";
        for (auto& [_, result] : results.ifaces) {
          result.save(t, toolRunId, deviceId);
          LOG_DEBUG << result.toDebugString() << '\n';
        }

        const std::vector<std::string> vlanIfacePrefixes{
            "Vlan", "Vlan ",
            "vlan", "vlan ",
            "VLAN", "VLAN "
        };
        for (auto& [name, iface] : results.ifaces) {
          for (const auto& vlan : iface.getVlans()) {
            const uint16_t vlanId{vlan.getVlanId()};
            for (const auto& vlanIfacePrefix : vlanIfacePrefixes) {
              const std::string vlanIfaceName{
                vlanIfacePrefix + std::to_string(static_cast<unsigned int>(vlanId))
              };
              if (results.ifaces.contains(vlanIfaceName)) {
                t.exec_prepared("insert_raw_device_interface_hierarchy",
                    toolRunId,
                    deviceId,
                    iface.getName(),
                    results.ifaces.at(vlanIfaceName).getName());
              }
            }
          }
        }

        const std::vector<std::string> portChannelPrefixes{
            "Port-channel", "Port-channel ",
            "Port-Channel", "Port-Channel ",
            "port-channel", "port-channel ",
            "PORT-CHANNEL", "PORT-CHANNEL "
        };
        for (const auto& [portChannelId, ifaceNames] : results.portChannels) {
          for (const auto& portChannelPrefix : portChannelPrefixes) {
            const std::string portChannelIfaceName{
              portChannelPrefix + std::to_string(static_cast<unsigned int>(portChannelId))
            };
            if (results.ifaces.contains(portChannelIfaceName)) {
              for (const auto& ifaceName : ifaceNames) {
                t.exec_prepared("insert_raw_device_interface_hierarchy",
                    toolRunId,
                    deviceId,
                    ifaceName,
                    results.ifaces.at(portChannelIfaceName).getName());
              }
            }
          }
        }

        LOG_DEBUG << "Iterating over networkBooks\n";
        for (auto& [zone, nets] : results.networkBooks) {
          for (auto& [name, book] : nets) {
            book.setId(zone);
            book.setName(name);
            book.save(t, toolRunId, deviceId);
            LOG_DEBUG << book.toDebugString() << '\n';
          }
        }

        LOG_DEBUG << "Iterating over serviceBooks\n";
        for (auto& [zone, apps] : results.serviceBooks) {
          for (auto& [name, book] : apps) {
            book.setId(zone);
            book.setName(name);
            book.save(t, toolRunId, deviceId);
            LOG_DEBUG << book.toDebugString() << '\n';
          }
        }

        LOG_DEBUG << "Iterating over ruleBooks\n";
        for (auto& [name, book] : results.ruleBooks) {
          LOG_DEBUG << name << '\n';
          for (auto& [_, rule] : book) {
            rule.save(t, toolRunId, deviceId);
            LOG_DEBUG << rule.toDebugString() << '\n';
          }
        }

        LOG_DEBUG << "Iterating over Observations\n";
        results.observations.save(t, toolRunId, deviceId);
        LOG_DEBUG << results.observations.toDebugString() << '\n';

        first = false;
      }
    }

  protected: // Methods part of subclass API
  public: // Methods part of public API
};


// =============================================================================
// Program entry point
// =============================================================================
int
main(int argc, char** argv)
{
  Tool<Parser, Result> tool; // if parser needed
  return tool.start(argc, argv);
}
