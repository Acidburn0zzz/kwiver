/*ckwg +29
 * Copyright 2012-2013 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SPROKIT_TOOLS_TOOL_USAGE_H
#define SPROKIT_TOOLS_TOOL_USAGE_H

#include "tools-config.h"

#include <sprokit/pipeline_util/pipeline_builder.h>
#include <sprokit/pipeline/scheduler_factory.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

namespace sprokit
{

SPROKIT_TOOLS_EXPORT SPROKIT_NO_RETURN void tool_usage(int ret, boost::program_options::options_description const& options);
SPROKIT_TOOLS_EXPORT void tool_version_message();

SPROKIT_TOOLS_EXPORT boost::program_options::options_description tool_common_options();

SPROKIT_TOOLS_EXPORT boost::program_options::variables_map tool_parse(
  int argc, char const* argv[],
  boost::program_options::options_description const& desc,
  std::string const& program_description);

SPROKIT_TOOLS_EXPORT boost::program_options::options_description pipeline_common_options();
SPROKIT_TOOLS_EXPORT boost::program_options::options_description pipeline_input_options();
SPROKIT_TOOLS_EXPORT boost::program_options::options_description pipeline_output_options();
SPROKIT_TOOLS_EXPORT boost::program_options::options_description pipeline_run_options();

/**
 * \brief Create pipeline from command line input.
 *
 * This is the all-in-one call to create a pipeline builder.
 *
 * \param vm Variable map from parsing the command line
 * \param desc Command line options descriptions
 */
SPROKIT_TOOLS_EXPORT
sprokit::pipeline_builder_sptr build_pipeline( boost::program_options::variables_map const& vm,
                                               boost::program_options::options_description const& desc );


/**
 * \brief Load options into builder.
 *
 * This method loads options as specified from the command
 * line. These options are supplementary config files and settings
 * as specified in the program options supplied.
 *
 * The result of this call is to add more entries to the internal
 * pipeline representation.
 *
 * \param vm Program options
 */
SPROKIT_TOOLS_EXPORT
sprokit::pipeline_builder_sptr load_from_options( boost::program_options::variables_map const& vm,
                                                  sprokit::pipeline_builder_sptr pbs = 0);
}

#endif // SPROKIT_TOOLS_TOOL_USAGE_H
