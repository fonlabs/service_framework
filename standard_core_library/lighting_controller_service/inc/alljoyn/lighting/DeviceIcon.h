#ifndef LSF_DEVICE_ICON_H
#define LSF_DEVICE_ICON_H
/**
 * \ingroup ControllerService
 */
/**
 * @file
 * This file provides definitions for device icon
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

#include <qcc/String.h>

namespace lsf {
/**
 * Device Icon URL
 */
extern const qcc::String DeviceIconURL;
/**
 * Device Icon Mime Type
 */
extern const qcc::String DeviceIconMimeType;
/**
 * Device Icon embedded data
 */
extern uint8_t DeviceIcon[];
/**
 * Device icon size
 */
extern const size_t DeviceIconSize;

}

#endif
