#ifndef LSF_FILE_PARSER_H
#define LSF_FILE_PARSER_H
/******************************************************************************
 * Copyright (c) 2014, AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

#include <string>
#include <iostream>
#include <fstream>

namespace lsf {

class LampState;

extern const std::string resetID;

extern const std::string initialStateID;

void ParseLampState(std::istream& stream, LampState& state);

/**
 * Read a string from the stream.  Spaces will be included between double-quotes
 *
 * @param stream    The stream
 * @return          The next token in the stream
 */
std::string ParseString(std::istream& stream);

template <typename T>
T ParseValue(std::istream& stream)
{
    T t;
    stream >> t;
    return t;
}

std::ostream& WriteValue(std::ostream& stream, const std::string& name);

std::ostream& WriteString(std::ostream& stream, const std::string& name);


}

#endif
