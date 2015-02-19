/**
 * @file
 *
 * Define the abstracted socket interface for Linux.
 */

/******************************************************************************
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
#ifndef _OS_QCC_SOCKETTYPES_H
#define _OS_QCC_SOCKETTYPES_H

#include <qcc/platform.h>

#include <arpa/inet.h>
#include <limits.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

/** Windows uses SOCKET_ERROR to signify errors */
#define SOCKET_ERROR -1

namespace qcc {

/**
 * GCC definition of QCC_IOVec matches the POSIX definition of struct iovec for
 * direct casting.
 */
struct IOVec {
    void* buf;  /**< Pointer to a buffer to be included in a scatter-gather list. */
    size_t len; /**< Length of the buffer. */
};

#define QCC_MAX_SG_ENTRIES (IOV_MAX)  /**< Maximum number of scatter-gather list entries. */

typedef socklen_t SockAddrSize;  /**< Abstraction of the socket address length type. */

/**
 * Enumeration of address families.
 */
typedef enum {
    QCC_AF_UNSPEC = PF_UNSPEC,  /**< unspecified address family */
    QCC_AF_INET  = PF_INET,     /**< IPv4 address family */
    QCC_AF_INET6 = PF_INET6,    /**< IPv6 address family */
    QCC_AF_UNIX  = PF_UNIX      /**< UNIX file system sockets address family */
} AddressFamily;

/**
 * Enumeration of socket types.
 */
typedef enum {
    QCC_SOCK_STREAM =    SOCK_STREAM,    /**< TCP */
    QCC_SOCK_DGRAM =     SOCK_DGRAM,     /**< UDP */
    QCC_SOCK_SEQPACKET = SOCK_SEQPACKET, /**< Sequenced data transmission */
    QCC_SOCK_RAW =       SOCK_RAW,       /**< Raw IP packet */
    QCC_SOCK_RDM =       SOCK_RDM        /**< Reliable datagram */
} SocketType;


/**
 * The abstract message header structure defined to match the Linux definition of struct msghdr.
 */
struct MsgHdr {
    void* name;             /**< IP Address. */
    socklen_t nameLen;      /**< IP Address length. */
    struct IOVec* iov;      /**< Array of scatter-gather entries. */
    size_t iovLen;          /**< Number of elements in iov. */
    void* control;          /**< Ancillary data buffer. */
    socklen_t controlLen;   /**< Ancillary data buffer length. */
    int flags;              /**< Flags on received message. */
};

}

#endif
