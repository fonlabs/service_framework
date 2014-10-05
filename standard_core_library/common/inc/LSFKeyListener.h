#ifndef LSF_KEYLISTENER_H_
#define LSF_KEYLISTENER_H_
/**
 * \ingroup Common
 */
/**
 * \file  common/inc/LSFKeyListener.h
 * This file provides definitions for LSF key listener
 */
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
/**
 * \ingroup Common
 */

#include <alljoyn/AuthListener.h>

namespace lsf {

/**
 * class SrpKeyXListener. \n
 * A listener for Authentication. \n
 * instance of that class is given by bus attachment EnablePeerSecurity().
 */
class LSFKeyListener : public ajn::AuthListener {
  public:
    /**
     * LSFKeyListener constructor
     */
    LSFKeyListener();

    /**
     * ~LSFKeyListener destructor
     */
    virtual ~LSFKeyListener();

    /**
     * SetPassCode - called by the application to set the password
     * @param code PassCode to set
     */
    void SetPassCode(qcc::String const& code);

    /**
     * SetGetPassCodeFunc
     * @param GetPassCodeFunc - callback function to set
     */
    void SetGetPassCodeFunc(const char* (*GetPassCodeFunc)());

    /**
     * Authentication mechanism requests user credentials. If the user name is not an empty string \n
     * the request is for credentials for that specific user. A count allows the listener to decide \n
     * whether to allow or reject multiple authentication attempts to the same peer. \n\n
     *
     * An implementation must provide RequestCredentials or RequestCredentialsAsync but not both. \n
     *
     * @param authMechanism  The name of the authentication mechanism issuing the request.
     * @param authPeer       The name of the remote peer being authenticated.  On the initiating
     *                       side this will be a well-known-name for the remote peer. On the
     *                       accepting side this will be the unique bus name for the remote peer.
     * @param authCount      Count (starting at 1) of the number of authentication request attempts made.
     * @param userId       The user name for the credentials being requested.
     * @param credMask       A bit mask identifying the credentials being requested. The application
     *                       may return none, some or all of the requested credentials.
     * @param[out] creds    The credentials returned.
     *
     * @return  The caller should return true if the request is being accepted or false if the
     *          requests is being rejected. If the request is rejected the authentication is
     *          complete.
     */
    bool RequestCredentials(const char* authMechanism, const char* authPeer, uint16_t authCount, const char* userId,
                            uint16_t credMask, Credentials& creds);

    /**
     * Reports successful or unsuccessful completion of authentication.
     *
     * @param authMechanism  The name of the authentication mechanism that was used or an empty
     *                       string if the authentication failed.
     * @param authPeer       The name of the remote peer being authenticated.  On the initiating
     *                       side this will be a well-known-name for the remote peer. On the
     *                       accepting side this will be the unique bus name for the remote peer.
     * @param success        true if the authentication was successful, otherwise false.
     */
    void AuthenticationComplete(const char* authMechanism, const char* authPeer, bool success);

  private:
    qcc::String passCode;

    const char* (*GetPassCode)();
};

}

#endif


