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

#ifndef IP_ADDRESS_HPP
#define IP_ADDRESS_HPP

#include <compare>
#include <set>

#include <netmeld/datastore/objects/IpNetwork.hpp>


namespace netmeld::datastore::objects {

  class IpAddress : public IpNetwork {
    // =========================================================================
    // Variables
    // =========================================================================
    private:
    protected:
      bool isResponding  {false};

      // NOTE: allow adding aliases even if object const
      mutable std::set<std::string> aliases;

    public:

    // =========================================================================
    // Constructors and Destructors
    // =========================================================================
    private:
    protected:
    public:
      virtual ~IpAddress() = default;
      IpAddress();
      explicit IpAddress(const std::string&, const std::string& x="");
      explicit IpAddress(const std::vector<uint8_t>&);

    // =========================================================================
    // Methods
    // =========================================================================
    private:
    protected:
    public:
      static IpAddress getIpv4Default();
      static IpAddress getIpv6Default();

      void addAlias(const std::string&, const std::string&) const;
      std::set<std::string> getAliases() const;

      void setResponding(const bool);

      bool isValid() const override;

      void save(pqxx::transaction_base&,
                const nmco::Uuid&, const std::string&) override;

      std::string toString() const;
      std::string toDebugString() const override;

      auto operator<=>(const IpAddress&) const = default;
      bool operator==(const IpAddress&) const = default;
  };
}
#endif // IP_ADDRESS_HPP
