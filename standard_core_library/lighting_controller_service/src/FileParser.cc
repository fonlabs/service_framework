/******************************************************************************
 * Copyright AllSeen Alliance. All rights reserved.
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

#include <alljoyn/lighting/FileParser.h>
#include <alljoyn/lighting/LSFTypes.h>
#include <qcc/Debug.h>

#define QCC_MODULE "FILE_PARSER"

namespace lsf {

const std::string resetID = "Reset";

const std::string initialStateID = "InitialState";

void ParseLampState(std::istream& stream, LampState& state)
{
    bool nullState = (bool) ParseValue<uint32_t>(stream);
    if (!nullState) {
        bool onOff = (bool) ParseValue<uint32_t>(stream);
        uint32_t hue = ParseValue<uint32_t>(stream);
        uint32_t saturation = ParseValue<uint32_t>(stream);
        uint32_t colorTemp = ParseValue<uint32_t>(stream);
        uint32_t brightness = ParseValue<uint32_t>(stream);
        state = LampState(onOff, hue, saturation, colorTemp, brightness);
        QCC_DbgPrintf(("%s: Parsed State = %s", __func__, state.c_str()));
    } else {
        state = LampState();
        QCC_DbgPrintf(("%s: Parsed Null State", __func__));
    }
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
