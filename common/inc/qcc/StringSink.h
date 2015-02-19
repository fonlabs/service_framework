/**
 * @file
 *
 * Sink implementation which stores data in a qcc::String.
 */

/******************************************************************************
 *
 *
 * Copyright (c) 2009-2011, AllSeen Alliance. All rights reserved.
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

#ifndef _QCC_STRINGSINK_H
#define _QCC_STRINGSINK_H

#include <qcc/platform.h>
#include <qcc/String.h>
#include <qcc/Stream.h>

namespace qcc {

/**
 * StringSink provides Sink based storage for bytes.
 */
class StringSink : public Sink {
  public:

    /** Destructor */
    virtual ~StringSink() { }

    /**
     * Push bytes into the sink.
     *
     * @param buf          Buffer to store pulled bytes
     * @param numBytes     Number of bytes from buf to send to sink.
     * @param numSent      Number of bytes actually consumed by sink.
     * @return   ER_OK if successful.
     */
    QStatus PushBytes(const void* buf, size_t numBytes, size_t& numSent)
    {
        str.append((char*)buf, numBytes);
        numSent = numBytes;
        return ER_OK;
    }

    /**
     * Get reference to sink storage.
     * @return string used to hold Sink data.
     */
    qcc::String& GetString() { return str; }

    /**
     * Clear existing bytes from sink.
     */
    void Clear() { str.clear(); }

  private:
    qcc::String str;    /**< storage for byte stream */
};

}

#endif
