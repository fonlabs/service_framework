#ifndef LSF_RANK_H
#define LSF_RANK_H
/**
 * \defgroup Common
 * Common code
 */
/**
 * \ingroup Common
 */
/**
 * @file
 * This file provides definitions for the Rank class
 */
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

#include <qcc/platform.h>

namespace lsf {

class Rank {
  public:
    /**
     * class constructor
     */
    Rank();

    /**
     * Parameterized Constructor
     */
    Rank(uint64_t higherBits, uint64_t lowerBits);

    /**
     * Copy Constructor
     */
    Rank(const Rank& other);

    /**
     * class destructor
     */
    ~Rank();

    /**
     * Set the parameters of the rank
     */
    void Set(uint64_t higherBits, uint64_t lowerBits);

    /**
     * Returns the higher order bits of the rank
     */
    uint64_t GetHigherOrderBits(void) { return higherOrderBits; };

    /**
     * Returns the lower order bits of the rank
     */
    uint64_t GetLowerOrderBits(void) { return lowerOrderBits; };

    /**
     * Assignment operator
     */
    Rank& operator =(const Rank& other);

    /**
     * Rank == operator
     */
    bool operator ==(const Rank& other) const;

    /**
     * Rank != operator
     */
    bool operator !=(const Rank& other) const;

    /**
     * Rank < operator
     */
    bool operator <(const Rank& other) const;

    /**
     * Rank > operator
     */
    bool operator >(const Rank& other) const;

    /**
     * Return the details of the Rank as a string
     *
     * @return string
     */
    const char* c_str(void) const;

    /**
     * Returns true if the rank has been initialized
     */
    bool IsInitialized(void) { return initialized; }

    /**
     * Set initialized to true
     */
    void SetInitialized(void) { initialized = true; };

  private:

    /**
     * 64 higher order bits in the 128-bit rank
     */
    uint64_t higherOrderBits;

    /**
     * 64 lower order bits in the 128-bit rank
     */
    uint64_t lowerOrderBits;

    /**
     * Indicates if the rank has been initialized
     */
    bool initialized;
};

}

#endif
