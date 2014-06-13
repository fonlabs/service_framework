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

#include <FileParser.h>
#include <LSFTypes.h>

namespace lsf {

void ParseLampState(std::istream& stream, LampState& state)
{
    state.onOff = (bool) ParseValue<uint32_t>(stream);
    state.hue = ParseValue<uint32_t>(stream);
    state.saturation = ParseValue<uint32_t>(stream);
    state.colorTemp = ParseValue<uint32_t>(stream);
    state.brightness =  ParseValue<uint32_t>(stream);
}

std::string ParseString(std::istream& stream)
{
    std::string name;
    stream >> name;

    if (name[0] == '"') {
        name = name.substr(1, std::string::npos);

        while (name[name.length() - 1] != '"') {
            std::string s;
            stream >> s;
            name += ' ' + s;
        }

        if (name[name.length() - 1] == '"') {
            name = name.substr(0, name.length() - 1);
        }
    }

    return name;
}

std::ostream& WriteValue(std::ostream& stream, const std::string& name)
{
    stream << name;
    return stream;
}

std::ostream& WriteString(std::ostream& stream, const std::string& name)
{
    stream << '"' << name << '"';
    return stream;
}

}
