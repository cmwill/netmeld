// =============================================================================
// Copyright 2023 National Technology & Engineering Solutions of Sandia, LLC
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

#include "Parser.hpp"


// =============================================================================
// Parser logic
// =============================================================================
Parser::Parser()
{}

void
Parser::fromJson(const json& _data)
{
  try {
    processTransitGatewayAttachments(_data);
  } catch (json::parse_error& ex) {
    LOG_ERROR << "Parse error at byte " << ex.byte
              << std::endl;
    std::exit(nmcu::Exit::FAILURE);
  }
}

void
Parser::processTransitGatewayAttachments(const json& _json)
{
  if (!_json.contains("TransitGatewayAttachments")) {
    return;
  }

  for (const auto& json : _json.at("TransitGatewayAttachments")) {
    nmdoa::TransitGatewayAttachment tgwa;
    tgwa.setTgwId(json.value("TransitGatewayId", ""));
    tgwa.setTgwOwnerId(json.value("TransitGatewayOwnerId", ""));
    tgwa.setTgwAttachmentId(json.value("TransitGatewayAttachmentId", ""));
    tgwa.setResourceType(json.value("ResourceType", ""));
    tgwa.setResourceId(json.value("ResourceId", ""));
    tgwa.setResourceOwnerId(json.value("ResourceOwnerId", ""));
    tgwa.setState(json.value("State", ""));

    if (json.contains("Association")) {
      const auto& jsonSub {json.at("Association")};
      tgwa.setAssociationState(jsonSub.value("State", ""));
    }

    d.tgwas.emplace_back(tgwa);
  }
}


// =============================================================================
// Parser helper methods
// =============================================================================
Result
Parser::getData()
{
  Result r;
  r.emplace_back(d);
  return r;
}
