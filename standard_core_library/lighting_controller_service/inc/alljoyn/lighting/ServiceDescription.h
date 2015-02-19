#ifndef _SERVICE_DESCRIPTION_H_
#define _SERVICE_DESCRIPTION_H_
/**
 * \ingroup ControllerService
 */
/**
 * @file
 * This file provides definitions for service description
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

#include <string>

namespace lsf {

extern const std::string ControllerServiceDescription;
extern const std::string ControllerServiceLampDescription;
extern const std::string ControllerServiceLampGroupDescription;
extern const std::string ControllerServicePresetDescription;
extern const std::string ControllerServiceSceneDescription;
extern const std::string ControllerServiceMasterSceneDescription;
extern const std::string LeaderElectionAndStateSyncDescription;

}

#endif
