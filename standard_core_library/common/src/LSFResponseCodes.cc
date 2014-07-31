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

#include <LSFResponseCodes.h>
#include <LSFTypes.h>

namespace lsf {

const char* LSFResponseCodeText(LSFResponseCode responseCode)
{
    switch (responseCode) {
        LSF_CASE(LSF_OK);
        LSF_CASE(LSF_ERR_NULL);
        LSF_CASE(LSF_ERR_UNEXPECTED);
        LSF_CASE(LSF_ERR_INVALID);
        LSF_CASE(LSF_ERR_UNKNOWN);
        LSF_CASE(LSF_ERR_FAILURE);
        LSF_CASE(LSF_ERR_BUSY);
        LSF_CASE(LSF_ERR_REJECTED);
        LSF_CASE(LSF_ERR_RANGE);
        LSF_CASE(LSF_ERR_INVALID_FIELD);
        LSF_CASE(LSF_ERR_MESSAGE);
        LSF_CASE(LSF_ERR_INVALID_ARGS);
        LSF_CASE(LSF_ERR_EMPTY_NAME);
        LSF_CASE(LSF_ERR_RESOURCES);
        LSF_CASE(LSF_ERR_PARTIAL);
        LSF_CASE(LSF_ERR_NOT_FOUND);
        LSF_CASE(LSF_ERR_NO_SLOT);
        LSF_CASE(LSF_ERR_DEPENDENCY);
        LSF_CASE(LSF_RESPONSE_CODE_LAST);

    default:
        return "<unknown>";
    }
}

}
