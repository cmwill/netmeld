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

// NOTE This implementation is included in the header (at the end) since it
//      leverages templating.

#include <netmeld/datastore/parsers/ParserHelper.hpp>
#include <netmeld/datastore/utils/QueriesCommon.hpp>

namespace nmcu = netmeld::core::utils;
namespace nmdp = netmeld::datastore::parsers;
namespace nmdu = netmeld::datastore::utils;

namespace netmeld::datastore::tools {
  // ===========================================================================
  // Constructors
  // ===========================================================================
  template<typename P, typename R>
  AbstractImportTool<P,R>::AbstractImportTool()
  {}

  template<typename P, typename R>
  AbstractImportTool<P,R>::AbstractImportTool(
      const char* _helpBlurb,
      const char* _programName,
      const char* _version) :
    AbstractDatastoreTool(_helpBlurb, _programName, _version)
  {}


  // ===========================================================================
  // Tool Entry Points (execution order)
  // ===========================================================================
  template<typename P, typename R>
  int
  AbstractImportTool<P,R>::runTool()
  {
    if (opts.exists("data-path")) {
      dataPath = sfs::canonical(opts.getValue("data-path"));
    }

    setToolRunId();

    parseData(); // only returns on success

    pqxx::connection db {getDbConnectString()};
    nmdu::dbPrepareCommon(db);
    pqxx::work t{db};

    if (opts.exists("tool-run-metadata")) {
      LOG_DEBUG << "Running as tool-run-metadata\n";
      toolRunMetadataInserts(t);
    }
    else {
      LOG_DEBUG << "Running as general/specific tool\n";
      generalInserts(t, dataPath.string());
      specificInserts(t);
    }

    t.commit();

    if (!opts.exists("tool-run-id")) {
      LOG_INFO << "tool-run-id: " << toolRunId << '\n';
    }

    return nmcu::Exit::SUCCESS;
  }

  template<typename P, typename R>
  void
  AbstractImportTool<P,R>::printHelp() const
  {
    LOG_NOTICE << "Import output from " << helpBlurb
               << "\nUsage: " << programName << " [options] DATA_PATH"
               << "\nOptions:\n"
               << opts
               << this->bugTeam
               << '\n';
  }

  template<typename P, typename R>
  void
  AbstractImportTool<P,R>::setToolRunId()
  {
    toolRunId
      = opts.exists("tool-run-id")
      ? (nmco::Uuid(opts.getValue("tool-run-id")))
      : (nmco::Uuid());
  }

  template<typename P, typename R>
  void
  AbstractImportTool<P,R>::parseData()
  {
    executionStart = nmco::Time();
    tResults = nmdp::fromFilePath<P,R>(dataPath.string());
    executionStop = nmco::Time();
  }

  template<typename P, typename R>
  void
  AbstractImportTool<P,R>::toolRunMetadataInserts(pqxx::transaction_base&)
  {}

  template<typename P, typename R>
  void
  AbstractImportTool<P,R>::specificInserts(pqxx::transaction_base&)
  {}

  template<typename P, typename R>
  void
  AbstractImportTool<P,R>::addModuleOptions()
  {
    AbstractDatastoreTool::addModuleOptions();

    addRequiredDeviceId();

    opts.addRequiredOption("data-path", std::make_tuple(
          "data-path",
          po::value<std::string>()->required(),
          "Data to parse. Either --data-path param or implicit last argument.")
        );

    opts.addOptionalOption("pipe", std::make_tuple(
          "pipe",
          NULL_SEMANTIC,
          "Read input from STDIN; Save a copy to DATA_PATH for parsing.")
        );

    opts.addAdvancedOption("tool-run-id", std::make_tuple(
          "tool-run-id",
          po::value<std::string>(),
          "UUID for this run of the tool.")
        );
    opts.addAdvancedOption("tool-run-metadata", std::make_tuple(
          "tool-run-metadata",
          NULL_SEMANTIC,
          "Insert data into tool_run tables instead of device tables.")
        );

    opts.addPositionalOption("data-path", -1);
  }

  template<typename P, typename R>
  void
  AbstractImportTool<P,R>::addToolOptions()
  {}


  // ===========================================================================
  // General Functions (alphabetical)
  // ===========================================================================
  template<typename P, typename R>
  void
  AbstractImportTool<P,R>::generalInserts(
      pqxx::transaction_base& t,
      const std::string& dataFile)
  {
    t.exec_prepared("insert_tool_run",
        toolRunId,
        programName,
        helpBlurb, // commandLine
        dataFile,
        executionStart,
        executionStop);

    if (opts.exists("device-id")) {
      devInfo.setDeviceId(opts.getValue("device-id"));
    }
    if (opts.exists("device-type")) {
      devInfo.setDeviceType(opts.getValue("device-type"));
    }
    if (opts.exists("device-color")) {
      devInfo.setDeviceColor(opts.getValue("device-color"));
    }

    devInfo.save(t, toolRunId);
  }

  template<typename P, typename R>
  sfs::path const
  AbstractImportTool<P,R>::getDataPath() const
  {
    return dataPath;
  }

  template<typename P, typename R>
  const std::string
  AbstractImportTool<P,R>::getDeviceId() const
  {
    return devInfo.getDeviceId();
  }

  template<typename P, typename R>
  nmco::Uuid const
  AbstractImportTool<P,R>::getToolRunId() const
  {
    return toolRunId;
  }
}
