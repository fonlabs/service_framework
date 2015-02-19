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

#include <alljoyn/lighting/PropertyParser.h>
#include <fstream>
#include <sstream>
#include <qcc/Debug.h>

#define QCC_MODULE "PROPERTY_PARSER"

using namespace lsf;

qcc::String PropertyParser::trim(const qcc::String& str, const char* whitespace)
{
    size_t strBegin = str.find_first_not_of(whitespace);
    if (strBegin == qcc::String::npos) {
        return "";  // no content

    }
    size_t strEnd = str.find_last_not_of(whitespace);
    size_t strRange = strEnd - strBegin + 1;
    return str.substr(strBegin, strRange);
}

std::string PropertyParser::trim(const std::string& str, const char* whitespace)
{
    size_t strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos) {
        return "";  // no content

    }
    size_t strEnd = str.find_last_not_of(whitespace);
    size_t strRange = strEnd - strBegin + 1;
    return str.substr(strBegin, strRange);
}

bool PropertyParser::ParseFile(const std::string& fileName, std::map<qcc::String, qcc::String>& data)
{
    std::ifstream iniFile(fileName.c_str(), std::ifstream::in);
    if (!iniFile.is_open()) {
        return false;
    }

    std::string line;
    uint32_t lineNum = 0;
    while (std::getline(iniFile, line)) {
        ++lineNum;

        line = trim(line); // remove leading and trailing whitespaces

        // skip empty line
        if (!line.length()) {
            continue;
        }

        // skip comments
        if (line[0] == '#') {
            continue;
        }

        std::size_t found = line.find('=');
        if (found == qcc::String::npos) {
            QCC_DbgPrintf(("Line %u invalid: [%s]\n", lineNum, line.c_str()));
            continue;
        }

        std::string name = line.substr(0, found);
        name = trim(name); // remove leading and trailing whitespaces

        std::string value = line.substr(found + 1);
        value = trim(value);

        data[name.c_str()] = value.c_str();
    }

    iniFile.close();
    return true;
}

void PropertyParser::Tokenize(const qcc::String& inStr, std::vector<qcc::String>& strings, char sep)
{
    const std::string ws(1, sep);

    std::string token;
    std::stringstream stream(inStr.c_str());
    while (std::getline(stream, token, sep)) {
        token = trim(token, ws.c_str());

        if (!token.empty()) {
            strings.push_back(token.c_str());
        }
    }
}

bool PropertyParser::WriteFile(const std::string& fileName, const std::map<qcc::String, qcc::String>& data)
{
    std::ofstream iniFileWrite(fileName.c_str(), std::ofstream::out | std::ofstream::trunc);
    if (!iniFileWrite.is_open()) {
        return false;
    }

    std::map<qcc::String, qcc::String>::const_iterator it;
    for (it = data.begin(); it != data.end(); ++it) {
        iniFileWrite << it->first.c_str() << " = " << it->second.c_str() << '\n';
    }

    iniFileWrite.close();
    return true;
}
