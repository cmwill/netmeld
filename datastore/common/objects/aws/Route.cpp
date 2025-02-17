// =============================================================================
// Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC
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

#include <netmeld/datastore/objects/aws/RouteTable.hpp>


namespace netmeld::datastore::objects::aws {

  Route::Route()
  {}

  void
  Route::setId(const std::string& _id)
  {
    typeId = _id;
  }
  void
  Route::setState(const std::string& _state)
  {
    state = _state;
  }
  void
  Route::addCidrBlock(const std::string& _cidrBlock)
  {
    if (_cidrBlock.empty()) { return; } // Don't add empties
    CidrBlock cb {_cidrBlock};
    cidrBlocks.insert(cb);
  }
  void
  Route::addNonCidrBlock(const std::string& _dest)
  {
    if (_dest.empty()) { return; } // Don't add empties
    nonCidrBlocks.insert(_dest);
  }

  bool
  Route::isValid() const
  {
    return !(typeId.empty())
        ;
  }

  void
  Route::save(pqxx::transaction_base& t,
              const nmco::Uuid& toolRunId, const std::string& deviceId)
  {
    if (!isValid()) {
      LOG_DEBUG << "AWS Route object is not saving: " << toDebugString()
                << std::endl;
      return;
    }

    for (auto ip : cidrBlocks) {
      ip.save(t, toolRunId, deviceId);

      t.exec_prepared("insert_raw_aws_route_table_route_cidr"
          , toolRunId
          , deviceId
          , typeId
          , state
          , ip.toString()
        );
    }
    for (auto dest : nonCidrBlocks) {
      t.exec_prepared("insert_raw_aws_route_table_route_non_cidr"
          , toolRunId
          , deviceId
          , typeId
          , state
          , dest
        );
    }
  }

  std::string
  Route::toDebugString() const
  {
    std::ostringstream oss;

    oss << '['
        << "typeId: " << typeId
        << ", state: " << state
        << ", cidrBlocks: " << cidrBlocks
        << ", nonCidrBlocks: " << nonCidrBlocks
        << ']'
        ;

    return oss.str();
  }

  std::partial_ordering
  Route::operator<=>(const Route& rhs) const
  {
    return std::tie( typeId
                   , state
                   , nonCidrBlocks
                   , cidrBlocks
                   )
       <=> std::tie( rhs.typeId
                   , rhs.state
                   , rhs.nonCidrBlocks
                   , rhs.cidrBlocks
                   )
      ;
  }

  bool
  Route::operator==(const Route& rhs) const
  {
    return 0 == operator<=>(rhs);
  }
}
