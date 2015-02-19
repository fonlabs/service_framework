/**
 * @file
 *
 * Define a class that helps handle differences in wchar_t on different OSs
 */

/******************************************************************************
 *
 *
 * Copyright (c) 2009-2011, 2014 AllSeen Alliance. All rights reserved.
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

#ifndef _QCC_UNICODE_H
#define _QCC_UNICODE_H

#include <qcc/platform.h>

#if defined(QCC_OS_GROUP_POSIX)
#include <qcc/posix/unicode.h>
#elif defined(QCC_OS_GROUP_WINDOWS)
#include <qcc/windows/unicode.h>
#else
#error No OS GROUP defined.
#endif

#endif
