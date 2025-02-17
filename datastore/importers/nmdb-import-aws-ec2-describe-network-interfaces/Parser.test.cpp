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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <netmeld/datastore/parsers/ParserTestHelper.hpp>

#include "Parser.hpp"

namespace nmdo = netmeld::datastore::objects;
namespace nmdp = netmeld::datastore::parsers;

using qi::ascii::blank;

class TestParser : public Parser
{
  public:
    using Parser::processIps;
    using Parser::processSecurityGroups;
    using Parser::processInterfaceAttachment;
    using Parser::processInterfaces;
    using Parser::fromJson;
};

BOOST_AUTO_TEST_CASE(testProcessIps)
{
  TestParser tp;

  {
    json tv1 = json::parse(
        R"({"Ipv6Addresses": [], "PrivateIpAddresses": []})"
      );
    nmdoa::NetworkInterface tobj;
    tp.processIps(tv1, tobj);
    const std::string tev1 {R"(cidrBlocks: [])"};
    nmdp::testInString(tobj.toDebugString(), tev1);
  }
  {
    json tv1 = json::parse(R"(
        { "Ipv6Addresses": [{ "Ipv6Address": "1::2" }]
        , "PrivateIpAddresses": []
        }
        )"
      );
    nmdoa::NetworkInterface tobj;
    tp.processIps(tv1, tobj);
    const std::string tev1 {
        R"(cidrBlock: 1::2,)"
      };
    nmdp::testInString(tobj.toDebugString(), tev1);
  }
  {
    json tv1 = json::parse(R"(
          { "Ipv6Addresses": []
          , "PrivateIpAddresses":
            [{ "PrivateDnsName": "aB-1.c.D", "PrivateIpAddress": "1.2.3.4" }]
          }
        )"
      );
    nmdoa::NetworkInterface tobj;
    tp.processIps(tv1, tobj);
    const std::vector<std::string> tevs {
          R"(cidrBlock: 1.2.3.4,)"
        , R"(aliases: [aB-1.c.D])"
      };
    for (const auto& tev : tevs) {
      nmdp::testInString(tobj.toDebugString(), tev);
    }
  }
  {
    json tv1 = json::parse(R"(
          { "Ipv6Addresses": []
          , "PrivateIpAddresses":
            [{ "Primary": true
            , "PrivateIpAddress": "10.2.3.4"
            , "PrivateDnsName": "aB-1.c.D"
            , "Association":
              { "IpOwnerId": "1"
              , "CarrierIp": "1.2.3.4"
              , "CustomerOwnedIp": "1.2.3.4"
              , "PublicIp": "1.2.3.4"
              , "PublicDnsName": "aB-2.c.D"
              }
            }]
          }
        )"
      );
    nmdoa::NetworkInterface tobj;
    tp.processIps(tv1, tobj);
    const std::vector<std::string> tevs {
          R"(cidrBlock: 1.2.3.4,)"
        , R"(cidrBlock: 10.2.3.4,)"
        , R"(aliases: [aB-2.c.D])"
        , R"(aliases: [aB-1.c.D])"
      };
    for (const auto& tev : tevs) {
      nmdp::testInString(tobj.toDebugString(), tev);
    }
  }
  {
    json tv1 = json::parse(R"(
          { "Ipv6Addresses": [{ "Ipv6Address": "1::2" }]
          , "PrivateIpAddresses":
            [{ "Primary": true
            , "PrivateIpAddress": "10.2.3.4"
            , "PrivateDnsName": "a1"
            , "Association":
              { "IpOwnerId": "1"
              , "CarrierIp": "1.2.3.4"
              , "CustomerOwnedIp": "1.2.3.5"
              , "PublicIp": "1.2.3.6"
              , "PublicDnsName": "a2"
              }
            }]
          }
        )"
      );
    nmdoa::NetworkInterface tobj;
    tp.processIps(tv1, tobj);
    const std::vector<std::string> tevs {
          R"([cidrBlock: 1.2.3.4, state: , description: , aliases: [a2]])"
        , R"([cidrBlock: 1.2.3.5, state: , description: , aliases: [a2]])"
        , R"([cidrBlock: 1.2.3.6, state: , description: , aliases: [a2]])"
        , R"([cidrBlock: 10.2.3.4, state: , description: , aliases: [a1]])"
        , R"([cidrBlock: 1::2, state: , description: , aliases: []])"
      };
    for (const auto& tev : tevs) {
      nmdp::testInString(tobj.toDebugString(), tev);
    }
  }
}

BOOST_AUTO_TEST_CASE(testProcessSecurityGroups)
{
  TestParser tp;

  {
    json tv1 = json::parse(
        R"({ "Groups": [{ "GroupId": "" , "GroupName": "" }] })"
      );
    nmdoa::NetworkInterface tobj;
    tp.processSecurityGroups(tv1, tobj);
    const std::string tev1 {R"(securityGroups: [])"};
    nmdp::testInString(tobj.toDebugString(), tev1);
  }
  {
    json tv1 = json::parse(R"(
          { "Groups": [{ "GroupId": "a1" , "GroupName": "a B-c 1 2.3" }] }
        )"
      );
    nmdoa::NetworkInterface tobj;
    tp.processSecurityGroups(tv1, tobj);
    const std::string tev1 {R"(securityGroups: [a1])"};
    nmdp::testInString(tobj.toDebugString(), tev1);
  }
}

BOOST_AUTO_TEST_CASE(testProcessInterfaceAttachment)
{
  TestParser tp;

  {
    json tv1 = json::parse(R"(
          { "Attachment" :
            { "AttachTime": ""
            , "AttachmentId": ""
            , "DeleteOnTermination": false
            , "DeviceIndex": 0
            , "NetworkCardIndex": 0
            , "Status": ""
            }
          }
        )"
      );
    nmdoa::NetworkInterface tobj;
    tp.processInterfaceAttachment(tv1, tobj);
    const std::string tev1 {
        R"(attachment: [attachmentId: , status: , deleteOnTermination: 0])"
      };
    nmdp::testInString(tobj.toDebugString(), tev1);
  }
  {
    json tv1 = json::parse(R"(
          { "Attachment" :
            { "AttachTime": ""
            , "AttachmentId": "aB1-2c"
            , "DeleteOnTermination": true
            , "DeviceIndex": 0
            , "NetworkCardIndex": 0
            , "Status": "aB1-2c"
            }
          }
        )"
      );
    nmdoa::NetworkInterface tobj;
    tp.processInterfaceAttachment(tv1, tobj);
    const std::vector<std::string> tevs {
          R"(attachmentId: aB1-2c,)"
        , R"(status: aB1-2c,)"
        , R"(deleteOnTermination: 1])"
      };
    for (const auto& tev : tevs) {
      nmdp::testInString(tobj.toDebugString(), tev);
    }
  }
}

BOOST_AUTO_TEST_CASE(testProcessInterfaces)
{
  {
    TestParser tp;

    json tv1 = json::parse(
        R"({ "NetworkInterfaces": [] })"
      );
    tp.processInterfaces(tv1);
    auto tobj = tp.getData()[0].interfaces;
    BOOST_TEST(0 == tobj.size());
  }
  {
    TestParser tp;

    json tv1 = json::parse(
        // Elements we handle
        R"(
          { "NetworkInterfaces":
            [{
              "Description": ""
            , "InterfaceType": ""
            , "MacAddress": ""
            , "NetworkInterfaceId": ""
            , "SourceDestCheck": false
            , "Status": ""
            , "SubnetId": ""
            , "VpcId": ""
        )"
        // Elements we ignore
        R"(
            , "Association": {}
            , "Attachment": {}
            , "Groups": []
            , "Ipv4Prefixes": []
            , "Ipv6Addresses": []
            , "Ipv6Prefixes": []
            , "OwnerId": ""
            , "PrivateDnsName": ""
            , "PrivateIpAddress": ""
            , "PrivateIpAddresses": []
            }]
          }
        )"
      );
    tp.processInterfaces(tv1);
    auto tobj = tp.getData()[0].interfaces;
    BOOST_TEST(1 == tobj.size());
    nmdoa::NetworkInterface tev1;
    BOOST_TEST(tev1 == tobj[0]);
  }
  {
    TestParser tp;

    json tv1 = json::parse(
        // Elements we handle
        R"(
          { "NetworkInterfaces":
            [{
              "Description": "a B-c 1 2.3"
            , "InterfaceType": "a B-c 1 2.3"
            , "MacAddress": "a B-c 1 2.3"
            , "NetworkInterfaceId": "a B-c 1 2.3"
            , "SourceDestCheck": true
            , "Status": "aBc-1@3"
            , "SubnetId": "a B-c 1 2.3"
            , "VpcId": "a B-c 1 2.3"
        )"
        // Elements we ignore
        R"(
            , "Association": {}
            , "Attachment": {}
            , "Groups": []
            , "Ipv4Prefixes": []
            , "Ipv6Addresses": []
            , "Ipv6Prefixes": []
            , "OwnerId": ""
            , "PrivateDnsName": ""
            , "PrivateIpAddress": ""
            , "PrivateIpAddresses": []
            }]
          }
        )"
      );
    tp.processInterfaces(tv1);
    auto tobj = tp.getData()[0].interfaces;
    BOOST_TEST(1 == tobj.size());

    const std::vector<std::string> tevs {
          R"(interfaceId: a B-c 1 2.3,)"
        , R"(type: a B-c 1 2.3,)"
        , R"(description: a B-c 1 2.3,)"
        , R"(sourceDestinationCheck: 1,)"
        , R"(status: aBc-1@3,)"
        , R"(subnetId: a B-c 1 2.3,)"
        , R"(vpcId: a B-c 1 2.3,)"
        , R"(macAddr: a B-c 1 2.3,)"
      };
    for (const auto& tev : tevs) {
      nmdp::testInString(tobj[0].toDebugString(), tev);
    }
  }
}

BOOST_AUTO_TEST_CASE(testFromJson)
{
  {
    TestParser tp;

    json tv1 = json::parse(
        R"({ "NetworkInterfaces": [] })"
      );
    tp.fromJson(tv1);
    auto tobj = tp.getData()[0].interfaces;
    BOOST_TEST(0 == tobj.size());
  }
}
