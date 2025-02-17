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

#ifndef PORT_HPP
#define PORT_HPP

#include <compare>
#include <netmeld/datastore/objects/AbstractDatastoreObject.hpp>
#include <netmeld/datastore/objects/IpAddress.hpp>


namespace netmeld::datastore::objects {
  class Port : public AbstractDatastoreObject {
    // =========================================================================
    // Variables
    // =========================================================================
    private:
    protected:
      int           port {INT_MAX};
      std::string   protocol;
      std::string   state;
      std::string   reason;
      IpAddress     ipAddr;

    public:

    // =========================================================================
    // Constructors
    // =========================================================================
    private:
    protected:
    public:
      Port();
      explicit Port(const IpAddress&);

    // =========================================================================
    // Methods
    // =========================================================================
    private:
    protected:
    public:
      void setPort(const int);
      void setProtocol(const std::string&);
      void setReason(const std::string&);
      void setState(const std::string&);
      void setIpAddr(const IpAddress&);

      const IpAddress& getIpAddress() const;
      std::string getProtocol() const;
      int getPort() const;

      bool isValid() const override;

      void save(pqxx::transaction_base&,
                const nmco::Uuid&, const std::string&) override;

      std::string toDebugString() const override;

      std::partial_ordering operator<=>(const Port&) const;
      bool operator==(const Port&) const;
  };
}
#endif //PORT_HPP
