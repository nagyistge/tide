/*********************************************************************/
/* Copyright (c) 2013-2017, EPFL/Blue Brain Project                  */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
/*                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>     */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/*   1. Redistributions of source code must retain the above         */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer.                                                  */
/*                                                                   */
/*   2. Redistributions in binary form must reproduce the above      */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer in the documentation and/or other materials       */
/*      provided with the distribution.                              */
/*                                                                   */
/*    THIS  SOFTWARE IS PROVIDED  BY THE  UNIVERSITY OF  TEXAS AT    */
/*    AUSTIN  ``AS IS''  AND ANY  EXPRESS OR  IMPLIED WARRANTIES,    */
/*    INCLUDING, BUT  NOT LIMITED  TO, THE IMPLIED  WARRANTIES OF    */
/*    MERCHANTABILITY  AND FITNESS FOR  A PARTICULAR  PURPOSE ARE    */
/*    DISCLAIMED.  IN  NO EVENT SHALL THE UNIVERSITY  OF TEXAS AT    */
/*    AUSTIN OR CONTRIBUTORS BE  LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL,  SPECIAL, EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES    */
/*    (INCLUDING, BUT  NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE    */
/*    GOODS  OR  SERVICES; LOSS  OF  USE,  DATA,  OR PROFITS;  OR    */
/*    BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON ANY THEORY OF    */
/*    LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR TORT    */
/*    (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY OUT    */
/*    OF  THE  USE OF  THIS  SOFTWARE,  EVEN  IF ADVISED  OF  THE    */
/*    POSSIBILITY OF SUCH DAMAGE.                                    */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#ifndef SERIALIZATION_DEFLECTTYPES_H
#define SERIALIZATION_DEFLECTTYPES_H

#include <deflect/Frame.h>
#include <deflect/Segment.h>
#include <deflect/SegmentParameters.h>

#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/split_free.hpp>

namespace boost
{
namespace serialization
{
template <class Archive>
void serialize(Archive& ar, deflect::Frame& frame, const unsigned int)
{
    // clang-format off
    ar & frame.segments;
    ar & frame.uri;
    // clang-format on
}

template <class Archive>
void save(Archive& ar, const deflect::Segment& segment, const unsigned int)
{
    // clang-format off
    ar & segment.parameters;
    ar & segment.view;

    int size = segment.imageData.size();
    ar & size;

    ar & make_binary_object((void*)segment.imageData.data(),
                            segment.imageData.size());
    // clang-format on
}

template <class Archive>
void load(Archive& ar, deflect::Segment& segment, const unsigned int)
{
    // clang-format off
    ar & segment.parameters;
    ar & segment.view;

    int size = 0;
    ar & size;
    segment.imageData.resize(size);

    ar & make_binary_object((void*)segment.imageData.data(), size);
    // clang-format on
}

template <class Archive>
void serialize(Archive& ar, deflect::Segment& s, const unsigned int version)
{
    split_free(ar, s, version);
}

template <class Archive>
void serialize(Archive& ar, deflect::SegmentParameters& params,
               const unsigned int)
{
    // clang-format off
    ar & params.x;
    ar & params.y;
    ar & params.width;
    ar & params.height;
    ar & params.dataType;
    // clang-format on
}
}
}

#endif
