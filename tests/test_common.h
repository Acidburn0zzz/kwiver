/*ckwg +29
 * Copyright 2011-2013 by Kitware, Inc.
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

#ifndef SPROKIT_TEST_TEST_COMMON_H
#define SPROKIT_TEST_TEST_COMMON_H

#include <boost/function.hpp>

#include <exception>
#include <iostream>
#include <map>
#include <string>

#include <cstdlib>

typedef std::string testname_t;

#define TEST_ERROR(msg)                         \
  do                                            \
  {                                             \
    std::cerr << "Error: " << msg << std::endl; \
  } while (false)

#define EXPECT_EXCEPTION(ex, code, action)  \
  do                                        \
  {                                         \
    bool got_exception = false;             \
                                            \
    try                                     \
    {                                       \
      code;                                 \
    }                                       \
    catch (ex const& e)                     \
    {                                       \
      got_exception = true;                 \
                                            \
      std::cerr << "Expected exception: "   \
                << e.what()                 \
                << std::endl;               \
    }                                       \
    catch (std::exception const& e)         \
    {                                       \
      TEST_ERROR("Unexpected exception: "   \
                 << e.what());              \
                                            \
      got_exception = true;                 \
    }                                       \
    catch (...)                             \
    {                                       \
      TEST_ERROR("Non-standard exception"); \
                                            \
      got_exception = true;                 \
    }                                       \
                                            \
    if (!got_exception)                     \
    {                                       \
      TEST_ERROR("Did not get "             \
                 "expected exception when " \
                 << action);                \
    }                                       \
  } while (false)

#define DECLARE_TEST_MAP()                                    \
  namespace                                                   \
  {                                                           \
    typedef boost::function<void TEST_ARGS> test_function_t;  \
    typedef std::map<testname_t, test_function_t> test_map_t; \
  }                                                           \
  test_map_t __all_tests;                                     \
  struct __add_test                                           \
  {                                                           \
    __add_test(testname_t const& name,                        \
               test_function_t const& func)                   \
    {                                                         \
      __all_tests[name] = func;                               \
    }                                                         \
  }                                                           \

#define TEST_PROPERTY(property, value, ...)

#define IMPLEMENT_TEST(testname)                       \
  static void                                          \
  test_##testname TEST_ARGS;                           \
  static __add_test const                              \
    __add_test_##testname(#testname, test_##testname); \
  void                                                 \
  test_##testname TEST_ARGS

#define CHECK_ARGS(numargs)     \
  do                            \
  {                             \
    if (argc != (numargs + 1))  \
    {                           \
      TEST_ERROR("Expected "    \
                 #numargs       \
                 " arguments"); \
                                \
      return EXIT_FAILURE;      \
    }                           \
  } while (false)

#define RUN_TEST(testname, ...)                 \
  do                                            \
  {                                             \
    test_map_t::const_iterator const i =        \
      __all_tests.find(testname);               \
                                                \
    if (i == __all_tests.end())                 \
    {                                           \
      TEST_ERROR("Unknown test: " << testname); \
                                                \
      return EXIT_FAILURE;                      \
    }                                           \
                                                \
    test_function_t const& func = i->second;    \
                                                \
    try                                         \
    {                                           \
      func(__VA_ARGS__);                        \
    }                                           \
    catch (std::exception const& e)             \
    {                                           \
      TEST_ERROR("Unexpected exception: "       \
                 << e.what());                  \
                                                \
      return EXIT_FAILURE;                      \
    }                                           \
                                                \
    return EXIT_SUCCESS;                        \
  } while (false)

#endif // SPROKIT_TEST_TEST_COMMON_H
