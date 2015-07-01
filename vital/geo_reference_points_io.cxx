/*ckwg +29
 * Copyright 2014-2015 by Kitware, Inc.
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
 * \brief Implementation of functions regarding geo reference points files
 */

#include "geo_reference_points_io.h"
#include "exceptions.h"
#include "eigen_io.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include <boost/foreach.hpp>

namespace kwiver {
namespace vital {


/// Load landmarks and tracks from reference points file
void
load_reference_file( path_t const&      reference_file,
                     local_geo_cs&      lgcs,
                     landmark_map_sptr& ref_landmarks,
                     track_set_sptr&    ref_track_set )
{
  using namespace std;

  // Read in file, creating a landmark map and a vector of tracks, associated
  // via IDs
  std::ifstream input_stream( reference_file.c_str(), std::fstream::in );
  if ( ! input_stream )
  {
    throw file_not_found_exception( reference_file, "Could not open reference points file!" );
  }

  // pre-allocated vars for loop
  landmark_id_t cur_id = 1;
  frame_id_t frm;
  vector_2d feat_loc;
  vector_3d vec( 0, 0, 0 );
  double x, y;
  int zone;
  bool northp;
  // used to stream file lines into data types
  std::istringstream ss;

  landmark_map::map_landmark_t reference_lms;
  std::vector< track_sptr > reference_tracks;

  // Resetting lgcs' logical initialization
  lgcs.set_utm_origin( vec );
  lgcs.set_utm_origin_zone( -1 );
  // Mean position of all landmarks.
  vector_3d mean( 0, 0, 0 );

  // TODO: put in try-catch around >>'s in case we have an ill-formatted file,
  // or there's a parse error
  cerr << "[load_reference_file] Reading from file: " << reference_file << endl;
  cerr << "[load_reference_file] Reading landmarks and tracks..." << endl;
  for ( std::string line; std::getline( input_stream, line ); )
  {
    ss.clear();
    ss.str( line );

    // input landmarks are given in lon/lat/alt format (ignoring alt for now)
    ss >> vec;

    // When this is called the first time, setzone is given a -1, which is the
    // default for the function.
    lgcs.geo_map_algo()->latlon_to_utm( vec.y(), vec.x(), x, y, zone, northp,
                                        lgcs.utm_origin_zone() );
    vec[0] = x;
    vec[1] = y;
    vec[2] = vec.z();
    mean += vec;

    // Use the zone of the first input landmark as the base zone from which we
    // interpret all other geo-positions with respect to.
    if ( lgcs.utm_origin_zone() == -1 )
    {
      cerr << "[load_reference_file] - lgcs origin zone: " << zone << endl;
      lgcs.set_utm_origin_zone( zone );
    }

    //cerr << "[load_reference_file] landmark " << cur_id << " position :: " << std::setprecision(12) << vec << endl;
    reference_lms[cur_id] = landmark_sptr( new landmark_d( vec ) );

    // while there's still input left, read in track states
    //cerr << "[] track:" << endl;
    track_sptr lm_track( new track() );
    lm_track->set_id( static_cast< track_id_t > ( cur_id ) );
    while ( ss.peek() != std::char_traits< char >::eof() )
    {
      ss >> frm;
      ss >> feat_loc;
      lm_track->append( track::track_state( frm, feature_sptr( new feature_d( feat_loc ) ), descriptor_sptr() ) );
      //cerr << "[]\t- " << frm << " :: " << feat_loc << endl;
    }
    reference_tracks.push_back( lm_track );

    ++cur_id;
  }

  // Initialize lgcs center
  mean /= static_cast< double > ( reference_lms.size() );
  lgcs.set_utm_origin( mean );
  cerr << "[load_reference_file] mean position (lgcs origin): " << mean << endl;

  // Scan through reference landmarks, adjusting their location by the lgcs
  // origin.
  cerr << "[load_reference_file] transforming lm geographic locations to local system..." << endl;
  BOOST_FOREACH( landmark_map::map_landmark_t::value_type & p, reference_lms )
  {
    dynamic_cast< landmark_d* > ( p.second.get() )->set_loc( p.second->loc() - mean );
    //cerr << "[load_reference_file] -- " << p.first << " :: " << p.second->loc() << endl;
  }

  ref_landmarks = landmark_map_sptr( new simple_landmark_map( reference_lms ) );
  ref_track_set = track_set_sptr( new simple_track_set( reference_tracks ) );
} // load_reference_file


} } // end namespace vital
