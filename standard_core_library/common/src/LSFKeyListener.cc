/******************************************************************************
 * Copyright (c) 2014 AllSeen Alliance. All rights reserved.
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

#include <LSFKeyListener.h>
#include <qcc/Debug.h>

#define QCC_MODULE "LSF_KEY_LISTENER"
using namespace lsf;

LSFKeyListener::LSFKeyListener() : passCode("000000"), GetPassCode(0)
{
    QCC_DbgTrace(("%s", __func__));
}

LSFKeyListener::~LSFKeyListener()
{
    QCC_DbgTrace(("%s", __func__));
}

void LSFKeyListener::SetPassCode(qcc::String const& code)
{
    QCC_DbgTrace(("%s", __func__));
    passCode = code;
}

void LSFKeyListener::SetGetPassCodeFunc(const char* (*GetPassCodeFunc)())
{
    QCC_DbgTrace(("%s", __func__));
    GetPassCode = GetPassCodeFunc;
}

bool LSFKeyListener::RequestCredentials(const char* authMechanism, const char* authPeer,
                                        uint16_t authCount, const char* userId, uint16_t credMask, Credentials& creds)
{
    QCC_DbgTrace(("%s", __func__));
    if (strcmp(authMechanism, "ALLJOYN_ECDHE_PSK") == 0 || strcmp(authMechanism, "ALLJOYN_PIN_KEYX") == 0) {
        if (credMask & AuthListener::CRED_PASSWORD) {
            if (authCount <= 3) {
                const char* passCodeFromGet = 0;
                if (GetPassCode) {
                    passCodeFromGet = GetPassCode();
                }
                creds.SetPassword(passCodeFromGet ? passCodeFromGet : passCode.c_str());
                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}

void LSFKeyListener::AuthenticationComplete(const char* authMechanism, const char* authPeer, bool success)
{
    QCC_DbgTrace(("%s", __func__));
}
