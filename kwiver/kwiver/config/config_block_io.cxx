/*ckwg +29
 * Copyright 2013-2015 by Kitware, Inc.
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

/**
 * \file
 * \brief config_block IO operations implementation
 */

#include "config_block_io.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "config_block_io_exceptions.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace kwiver
{

/// Basic configuration key/value encapsulation structure
struct config_block_value_s {
  /// the configuration path within the block structure
  config_block_keys_t key_path;

  /// value associated with the given key
  config_block_value_t value;
};

/// The type that represents multiple stored configuration values.
typedef std::vector<config_block_value_s> config_block_value_set_t;

}

// Adapting structure to boost fusion tuples for use in spirit::qi
BOOST_FUSION_ADAPT_STRUCT(
  kwiver::config_block_value_s,
  (kwiver::config_block_keys_t, key_path)
  (kwiver::config_block_value_t, value)
)

namespace kwiver
{

namespace
{

/// Token representing the beginning of a block declaration
static token_t const config_block_start = token_t("block");
/// Token representing the end of a block declaration
static token_t const config_block_end = token_t("endblock");
/// Token representing the start of a comment
static token_t const config_comment_start = token_t("#");

/// Grammar definition for configuration files
template <typename Iterator>
class config_block_grammar
  : public qi::grammar<Iterator, config_block_value_set_t()>
{
  public:
    config_block_grammar();
    ~config_block_grammar();

  private:
    /// Optional whitespace (spaces/tabs)
    qi::rule<Iterator> opt_whitespace;
    /// Required whitespace (at least one space/tab)
    qi::rule<Iterator> whitespace;
    /// End-of-line rule (single EoL)
    qi::rule<Iterator> eol;
    /// Optional eol rule (0 or more)
    qi::rule<Iterator> opt_line_end;
    /// Required eol rule (1 or more)
    qi::rule<Iterator> line_end;

    /// Matches a single config_block key element
    qi::rule<Iterator, config_block_key_t()> config_block_key;
    /// Matches full config_block key path
    qi::rule<Iterator, config_block_keys_t()> config_block_key_path;
    /// Matches any valid "value" (printable characters with joining spaces)
    qi::rule<Iterator, config_block_value_t()> config_block_value; ///< excludes '#'s
    /// A key/value pair
    qi::rule<Iterator, config_block_value_s()> config_block_value_full;

    /// A comment within the config file
    qi::rule<Iterator, config_block_value_t()> comment_value; ///< includes '#''s
    qi::rule<Iterator, config_block_value_t()> comment;
    // TODO: (advanced) include logic for slurping up comment values into
    //       config_block structure as key descriptions.

    /// Result set of config block key/value pairs
    qi::rule<Iterator, config_block_value_set_t()> config_block_value_set;
};


template <typename Iterator>
config_block_grammar<Iterator>
::config_block_grammar()
  : config_block_grammar::base_type(config_block_value_set)
  , opt_whitespace()
  , whitespace()
  , eol()
  , opt_line_end()
  , line_end()
  , config_block_key()
  , config_block_key_path()
  , config_block_value()
  , config_block_value_full()
  , comment_value()
  , comment()
  , config_block_value_set()
{
  using namespace qi::labels;
  using phoenix::push_back;
  using phoenix::at_c;

  opt_whitespace.name("opt-whitespace");
  opt_whitespace %= *( qi::blank );

  whitespace.name("whitespace");
  whitespace %= +( qi::blank );

  eol.name("eol");
  eol %= qi::lit("\r\n")
       | qi::lit("\n");

  opt_line_end.name("opt-line-end");
  opt_line_end %= *( eol );

  line_end.name("line-end");
  line_end %= +( eol );


  config_block_key.name("config-key");
  config_block_key %=
    +( qi::alnum | qi::char_("_") | qi::char_(".") );

  config_block_key_path.name("config-key-path");
  config_block_key_path %=
    config_block_key % qi::lit(kwiver::kw_config_block::block_sep);

  config_block_value.name("config-value");
  // Works as intended in 1.47.0 and up, but broken for 1.46.1
  //config_block_value %=
  //  +( (qi::graph - "#")
  //   | qi::hold[+(qi::blank) >> (qi::graph - "#")]
  //   );
  config_block_value %=
    +( (qi::graph - "#")
     | qi::blank
     );

  config_block_value_full.name("config-value-full");
  config_block_value_full =
    ( (opt_whitespace >> config_block_key_path[at_c<0>(_val) = _1] >> opt_whitespace)
    > '='
    > (opt_whitespace >> -(config_block_value[at_c<1>(_val) = _1]) >> opt_whitespace)
    );


  comment_value.name("comment-value");
  comment_value =
    +( qi::graph | qi::blank );

  comment.name("comment");
  comment =
    +( config_comment_start >> opt_whitespace >> *comment_value[_val = _1] );


  // Main grammar parsing rule
  config_block_value_set.name("config-values");
  config_block_value_set =
    +( (opt_whitespace >> eol)  // empty line with possible extra whitespace
     | line_end                 // one or more empty lines
       // comment lines
     | (opt_whitespace >> comment >> opt_whitespace >> eol)
       // keypath/value specification
     | (config_block_value_full[push_back(_val, _1)] >> -comment >> opt_whitespace >> eol)
     );

}

template <typename Iterator>
config_block_grammar<Iterator>
::~config_block_grammar()
{
}

/// Return a string repeated \c n times.
config_block_description_t operator*(config_block_description_t const& s, size_t n)
{
  config_block_description_t r;
  r.reserve(s.size() * n);
  for (size_t i=0; i<n; ++i)
  {
    r += s;
  }
  return r;
}


// ------------------------------------------------------------------
/// Helper method to write out a comment to a configuration file ostream
/**
 * Makes sure there is no trailing white-space printed to file.
 */
void write_cb_comment(std::ostream & ofile, config_block_description_t const& comment)
{
  typedef config_block_description_t cbd_t;
  size_t line_width = 80;
  cbd_t comment_token = cbd_t(config_comment_start);
  cbd_t space_token = cbd_t(" ");

  // Add a leading new-line to separate comment block from previous config
  // entry.
  ofile << "\n";

  // preserve manually specified new-lines in the comment string, adding a
  // trailing new-line
  std::list<cbd_t> blocks;
  boost::algorithm::split(blocks, comment, boost::is_any_of("\n"));
  while (blocks.size() > 0)
  {
    cbd_t cur_block = blocks.front();
    blocks.pop_front();

    // Comment lines always start with the comment token
    cbd_t line_buffer = comment_token;

    // Counter of additional spaces to place in front of the next non-empty
    // word added to the line buffer. There is always at least one space
    // between words.
    size_t spaces = 1;

    std::list<cbd_t> words;
    // Not using token-compress in case there is purposeful use of multiple
    // adjacent spaces, like in bullited lists. This, however, leaves open
    // the appearance of empty-string words in the loop, which are handled.
    //boost::algorithm::split(words, cur_block, boost::is_any_of(" "), boost::token_compress_on);
    boost::algorithm::split(words, cur_block, boost::is_any_of(" "));
    while (words.size() > 0)
    {
      cbd_t cur_word = words.front();
      words.pop_front();

      // word is an empty string, meaning an intentional space was encountered.
      if (cur_word.size() == 0)
      {
        ++spaces;
      }
      else
      {
        if ((line_buffer.size() + spaces + cur_word.size()) > line_width)
        {
          ofile << line_buffer << "\n";
          line_buffer = comment_token;
          // On a line split, it makes sense to me that leading spaces are
          // treated as trailing white-space, which should not be output.
          spaces = 1;
        }
        line_buffer += (space_token * spaces) + cur_word;
        spaces = 1;
      }
    }

    // flush remaining contents of line buffer if there is anything
    if (line_buffer.size() > 0)
    {
      ofile << line_buffer << "\n";
    }
  }
}

} //end anonymous namespace


// ------------------------------------------------------------------
/// Read in a configuration file, producing a \c config_block object
config_block_sptr read_config_file(path_t const& file_path,
                                   config_block_key_t const& block_name)
{
  // Check that file exists
  if( ! boost::filesystem::exists(file_path) )
  {
    throw file_not_found_exception(file_path, "File does not exist.");
  }
  else if ( ! boost::filesystem::is_regular_file(file_path) )
  {
    throw file_not_found_exception(file_path, "Path given doesn't point to "
                                              "a regular file!");
  }

  // Reading in input file data
  std::ifstream input_stream(file_path.c_str(), std::fstream::in);
  if( ! input_stream )
  {
    throw file_not_read_exception(file_path, "Could not open file at given "
                                             "path.");
  }
  std::string storage;
  input_stream.unsetf(std::ios::skipws);
  std::copy(std::istream_iterator<char>(input_stream),
            std::istream_iterator<char>(),
            std::back_inserter(storage));
  // std::cerr << "Storage string:" << std::endl
  //           << storage << std::endl;

  // Commence parsing!
  typedef std::string::const_iterator str_iter;
  config_block_value_set_t config_block_values;
  config_block_grammar<str_iter> grammar;
  std::string::const_iterator s_begin = storage.begin();
  std::string::const_iterator s_end = storage.end();

  try
  {
    bool r = qi::parse(s_begin, s_end, grammar, config_block_values);

    if( r && s_begin == s_end )
    {
      std::cerr << "File parsed! Contained " << config_block_values.size() << " k/v entries." << std::endl;
    }
    else if (s_begin != s_end )
    {
      std::ostringstream sstr;
      sstr << "File not parsed completely! Parameters read in: " << config_block_values.size();
      throw file_not_read_exception(file_path, sstr.str().c_str());
    }
    else if (!r)
    {
      throw file_not_read_exception(file_path, "File not parsed!");
    }
  }
  catch (qi::expectation_failure<str_iter> const& e)
  {
    std::ostringstream sstr;
    sstr << "Grammar expectation failure: " << e.what_;
    throw file_not_parsed_exception(file_path, sstr.str().c_str());
  }

  // Now that we have the various key/value pairs, construct the config
  // object.
  //using std::cerr;
  //using std::endl;
  //cerr << "Constructing config_block from file:" << endl;
  config_block_sptr cb = config_block::empty_config(block_name);
  BOOST_FOREACH( config_block_value_s kv, config_block_values)
  {
    config_block_key_t key_path = boost::algorithm::join(kv.key_path, config_block::block_sep);
    // std::cerr << "KEY: " << key_path << std::endl;
    // std::cerr << "VALUE: " << kv.value << std::endl << std::endl;

    // add key/value to config object here
    // TODO: Parse out comment above k/v, set as description
    cb->set_value(key_path, boost::algorithm::trim_copy(kv.value));

    //cerr << "\t`" << key_path << "` -> `" << kv.value << "`" << endl;
  }

  return cb;
}


// ------------------------------------------------------------------
/// Output to file the given \c config_block object to the specified file path
void write_config_file(config_block_sptr const& config,
                       path_t const& file_path)
{
  using std::cerr;
  using std::endl;
  namespace bfs = boost::filesystem;

  // If there are no config parameters in the given config_block, throw
  if(!config->available_values().size())
  {
    throw file_write_exception(file_path, "No parameters in the given "
                                          "config_block!");
  }

  // If the given path is a directory, we obviously can't write to it.
  if(bfs::is_directory(file_path))
  {
    throw file_write_exception(file_path, "Path given is a directory, to "
                                          "which we clearly can't write.");
  }

  // Check that the directory of the given filepath exists, creating necessary
  // directories where needed.
  path_t parent_dir = bfs::absolute(file_path.parent_path());
  if(!bfs::is_directory(parent_dir))
  {
    //std::cerr << "at least one containing directory not found, creating them..." << std::endl;
    if(!bfs::create_directories(parent_dir))
    {
      throw file_write_exception(parent_dir, "Attempted directory creation, "
                                             "but no directory created! No "
                                             "idea what happened here...");
    }
  }

  // Gather available keys and sort them alphanumerically for a sensibly laidout
  // file.
  config_block_keys_t avail_keys = config->available_values();
  std::sort(avail_keys.begin(), avail_keys.end());

  // open output file and write each key/value to a line.
  std::ofstream ofile(file_path.c_str());
  bool prev_had_descr = false;  // for additional spacing
  BOOST_FOREACH( config_block_key_t key, avail_keys )
  {
    // Each key may or may not have an associated description string. If there
    // is one, write that out as a comment.
    // - comments will be limited to 80 character width lines, including "# "
    //   prefix.
    // - value output format: ``key_path = value\n``

    config_block_description_t descr = config->get_description(key);
    if (descr != config_block_description_t())
    {
      //std::cerr << "[write_config_file] Writing comment for '" << key << "'." << std::endl;
      write_cb_comment(ofile, descr);
      prev_had_descr = true;
    }
    else if( prev_had_descr )
    {
      // Add a spacer line after a k/v with a description
      ofile << "\n";
      prev_had_descr = false;
    }

    ofile << key << " = " << config->get_value<config_block_value_t>(key) << "\n";
  }
  ofile.flush();
  ofile.close();
}

}
