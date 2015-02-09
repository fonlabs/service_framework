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

#include <ControllerServiceRank.h>
#include <qcc/Debug.h>

#define QCC_MODULE "LSF_CONTROLLER_SERVICE_RANK"

namespace lsf {

void ControllerServiceRank::Initialize(void)
{
    QCC_DbgTrace(("%s", __func__));

    uint64_t higherOrderBits = 0, lowerOrderBits = 0;

    if (IsInitialized()) {
        QCC_DbgPrintf(("%s: Rank already initialized to %s", __func__, c_str()));
        return;
    }

    SetInitialized();

    uint64_t temp1 = 0, temp2 = 0;
    temp1 = (OEM_CS_GetMACAddress() & MAC_ADDR_BIT_MASK);
    QCC_DbgPrintf(("%s: Controller Service MAC address = %llu", __func__, temp1));
    temp2 = temp1 << BIT_SHIFT_FOR_MAC_ADDRESS;
    lowerOrderBits |= temp2;

    temp1 = 0;
    temp2 = 0;
    temp1 = CONTROLLER_SERVICE_VERSION;
    if (temp1 > MAX_CONTROLLER_SERVICE_VERSION) {
        QCC_DbgPrintf(("%s: Overriding Controller Service version invalid value %llu to 0", __func__, temp1));
        /*Setting to lowest possible value as the OEM has passed in an incorrect value*/
        temp1 = 0;
    }
    QCC_DbgPrintf(("%s: Controller Service version = %llu", __func__, temp1));
    temp2 = temp1 << BIT_SHIFT_FOR_CONTROLLER_SERVICE_VERSION;
    lowerOrderBits |= temp2;

    temp1 = 0;
    temp2 = 0;
    temp1 = OEM_CS_GetRankParam_NodeType();
    if (temp1 > ACCESS_POINT) {
        QCC_DbgPrintf(("%s: Overriding Node Type invalid value %llu to 0", __func__, temp1));
        /*Setting to lowest possible value as the OEM has passed in an incorrect value*/
        temp1 = 0;
    }
    QCC_DbgPrintf(("%s: Node Type = %llu", __func__, temp1));
    temp2 = temp1 << BIT_SHIFT_FOR_NODE_TYPE;
    lowerOrderBits |= temp2;

    temp1 = 0;
    temp2 = 0;
    temp1 = OEM_CS_GetRankParam_Power();
    if (temp1 > ALWAYS_AC_POWERED) {
        QCC_DbgPrintf(("%s: Overriding Power invalid value %llu to 0", __func__, temp1));
        /*Setting to lowest possible value as the OEM has passed in an incorrect value*/
        temp1 = 0;
    }
    QCC_DbgPrintf(("%s: Power = %llu", __func__, temp1));
    temp2 = temp1 << BIT_SHIFT_FOR_POWER;
    lowerOrderBits |= temp2;

    temp1 = 0;
    temp2 = 0;
    temp1 = OEM_CS_GetRankParam_Availability();
    if (temp1 >= OEM_CS_RANKPARAM_AVAILABILITY_LAST_VALUE) {
        QCC_DbgPrintf(("%s: Overriding Availability invalid value %llu to 0", __func__, temp1));
        /*Setting to lowest possible value as the OEM has passed in an incorrect value*/
        temp1 = 0;
    }
    QCC_DbgPrintf(("%s: Availability = %llu", __func__, temp1));
    temp2 = temp1 << BIT_SHIFT_FOR_AVAILABILITY;
    lowerOrderBits |= temp2;

    temp1 = 0;
    temp1 = OEM_CS_GetRankParam_Mobility();
    if (temp1 >= OEM_CS_RANKPARAM_MOBILITY_LAST_VALUE) {
        QCC_DbgPrintf(("%s: Overriding Mobility invalid value %llu to 0", __func__, temp1));
        /*Setting to lowest possible value as the OEM has passed in an incorrect value*/
        temp1 = 0;
    }
    QCC_DbgPrintf(("%s: Mobility = %llu", __func__, temp1));
    higherOrderBits = temp1;

    Set(higherOrderBits, lowerOrderBits);

    QCC_DbgPrintf(("%s: %s", __func__, c_str()));
}

}
