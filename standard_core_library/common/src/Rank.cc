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

#include <alljoyn/lighting/Rank.h>
#include <qcc/Debug.h>
#include <qcc/String.h>
#include <qcc/StringUtil.h>

#define QCC_MODULE "LSF_RANK"

using namespace qcc;

namespace lsf {

Rank::Rank() :
    higherOrderBits(0UL),
    lowerOrderBits(0UL),
    initialized(false)
{
    QCC_DbgTrace(("%s", __func__));
}

Rank::Rank(uint64_t higherBits, uint64_t lowerBits)
{
    QCC_DbgTrace(("%s", __func__));
    Set(higherBits, lowerBits);
}

Rank::Rank(const Rank& other) :
    higherOrderBits(other.higherOrderBits),
    lowerOrderBits(other.lowerOrderBits),
    initialized(other.initialized)
{
    QCC_DbgTrace(("%s", __func__));
}

Rank::~Rank()
{
    QCC_DbgTrace(("%s", __func__));
}

void Rank::Set(uint64_t higherBits, uint64_t lowerBits)
{
    QCC_DbgTrace(("%s", __func__));
    higherOrderBits = higherBits;
    lowerOrderBits = lowerBits;
    initialized = true;
}

Rank& Rank::operator =(const Rank& other)
{
    QCC_DbgTrace(("%s", __func__));
    higherOrderBits = other.higherOrderBits;
    lowerOrderBits = other.lowerOrderBits;
    initialized = other.initialized;
    return *this;
}

bool Rank::operator ==(const Rank& other) const
{
    QCC_DbgTrace(("%s", __func__));
    if ((higherOrderBits == other.higherOrderBits) && (lowerOrderBits == other.lowerOrderBits)) {
        QCC_DbgPrintf(("%s: Returning true", __func__));
        return true;
    }
    QCC_DbgPrintf(("%s: Returning false", __func__));
    return false;
}

bool Rank::operator !=(const Rank& other) const
{
    QCC_DbgTrace(("%s", __func__));
    if ((higherOrderBits != other.higherOrderBits) || (lowerOrderBits != other.lowerOrderBits)) {
        QCC_DbgPrintf(("%s: Returning true", __func__));
        return true;
    }
    QCC_DbgPrintf(("%s: Returning false", __func__));
    return false;
}

bool Rank::operator <(const Rank& other) const
{
    QCC_DbgTrace(("%s", __func__));
    if (higherOrderBits < other.higherOrderBits) {
        QCC_DbgPrintf(("%s: Returning true", __func__));
        return true;
    } else if ((higherOrderBits == other.higherOrderBits) && (lowerOrderBits < other.lowerOrderBits)) {
        QCC_DbgPrintf(("%s: Returning true", __func__));
        return true;
    }
    QCC_DbgPrintf(("%s: Returning false", __func__));
    return false;
}

bool Rank::operator >(const Rank& other) const
{
    QCC_DbgTrace(("%s", __func__));
    if (higherOrderBits > other.higherOrderBits) {
        QCC_DbgPrintf(("%s: Returning true", __func__));
        return true;
    } else if ((higherOrderBits == other.higherOrderBits) && (lowerOrderBits > other.lowerOrderBits)) {
        QCC_DbgPrintf(("%s: Returning true", __func__));
        return true;
    }
    QCC_DbgPrintf(("%s: Returning false", __func__));
    return false;
}

const char* Rank::c_str(void) const
{
    QCC_DbgPrintf(("%s", __func__));
    static qcc::String ret;
    ret.clear();
    ret = qcc::String("Higher Order Bits = 0x") + U64ToString(higherOrderBits, 16) + qcc::String("(") + U64ToString(higherOrderBits) + qcc::String(")");
    ret += qcc::String(" Lower Order Bits = 0x") + U64ToString(lowerOrderBits, 16) + qcc::String("(") + U64ToString(lowerOrderBits) + qcc::String(")") + qcc::String("\n");
    return ret.c_str();
}

}
