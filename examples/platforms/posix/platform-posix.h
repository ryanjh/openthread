/*
 *  Copyright (c) 2016, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 * @brief
 *   This file includes the (posix or windows) platform-specific initializers.
 */

#ifndef PLATFORM_POSIX_H_
#define PLATFORM_POSIX_H_

#ifdef OPENTHREAD_CONFIG_FILE
#include OPENTHREAD_CONFIG_FILE
#else
#include <openthread-config.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#if _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#define POLL WSAPoll
#define ssize_t int
#include <time.h>
#define localtime _localtime32
// In user mode, define some Linux functions
__forceinline int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    (void)tz;
    tv->tv_sec = _time32(NULL);
    tv->tv_usec = 0;
    return 0;
}
__forceinline void timersub(struct timeval *a, struct timeval *b, struct timeval *res)
{
    res->tv_sec = (long)_difftime32(a->tv_sec, b->tv_sec);
    res->tv_usec = 0;
}
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#define POLL poll
#endif

#include <openthread/openthread.h>

OT_TOOL_PACKED_BEGIN
struct RadioMessage
{
    uint8_t mChannel;
    uint8_t mPsdu[kMaxPHYPacketSize];
} OT_TOOL_PACKED_END;

typedef struct otPlatformAlarm
{
    bool s_is_running;
    uint32_t s_alarm;
} otPlatformAlarm;

typedef struct otPlatformRadio
{
    PhyState sState;

    struct RadioMessage sReceiveMessage;
    struct RadioMessage sTransmitMessage;
    struct RadioMessage sAckMessage;
    RadioPacket sReceiveFrame;
    RadioPacket sTransmitFrame;
    RadioPacket sAckFrame;

    uint8_t  sExtendedAddress[OT_EXT_ADDRESS_SIZE];
    uint16_t sShortAddress;
    uint16_t sPanid;
    int      sSockFd;

    bool sPromiscuous;
    bool sAckWait;
    uint16_t sPortOffset;
} otPlatformRadio;

typedef struct otPlatformInstance
{
    uint32_t nodeId;

    //
    // Platform Alarm
    //
    otPlatformAlarm platformAlarm;

    //
    // Platform Radio
    //
    otPlatformRadio platformRadio;

} otPlatformInstance;

/**
 * Unique node ID.
 *
 */
extern uint32_t NODE_ID;

/**
 * Well-known Unique ID used by a simulated radio that supports promiscuous mode.
 *
 */
extern uint32_t WELLKNOWN_NODE_ID;

static inline otPlatformInstance* getPlatformInstance(otInstance *aInstance)
{
    return (otPlatformInstance*)((uint8_t*)aInstance - sizeof(otPlatformInstance));
}

static inline uint32_t getPlatformNodeId(otInstance *aInstance)
{
#ifdef OPENTHREAD_MULTIPLE_INSTANCE
    return getPlatformInstance(aInstance)->nodeId;
#else
    return NODE_ID;
#endif
}

/**
 * This function initializes the alarm service used by OpenThread.
 *
 */
void platformAlarmInit(void);

/**
 * This function retrieves the time remaining until the alarm fires.
 *
 * @param[out]  aTimeval  A pointer to the timeval struct.
 *
 */
void platformAlarmUpdateTimeout(otInstance *aInstance, struct timeval *tv);

/**
 * This function performs alarm driver processing.
 *
 * @param[in]  aInstance  The OpenThread instance structure.
 *
 */
void platformAlarmProcess(otInstance *aInstance);

/**
 * This function initializes the radio service used by OpenThread.
 *
 */
void platformRadioInit(void);

void platformRadioCopy(otInstance *aInstance);

/**
 * This function updates the file descriptor sets with file descriptors used by the radio driver.
 *
 * @param[inout]  aReadFdSet   A pointer to the read file descriptors.
 * @param[inout]  aWriteFdSet  A pointer to the write file descriptors.
 * @param[inout]  aMaxFd       A pointer to the max file descriptor.
 *
 */
void platformRadioUpdateFdSet(otInstance *aInstance, fd_set *aReadFdSet, fd_set *aWriteFdSet, int *aMaxFd);

/**
 * This function performs radio driver processing.
 *
 * @param[in]  aInstance  The OpenThread instance structure.
 *
 */
void platformRadioProcess(otInstance *aInstance);

/**
 * This function initializes the random number service used by OpenThread.
 *
 */
void platformRandomInit(void);

void platformAlarmCopy(otInstance *aInstance);

/**
 * This function updates the file descriptor sets with file descriptors used by the UART driver.
 *
 * @param[inout]  aReadFdSet   A pointer to the read file descriptors.
 * @param[inout]  aWriteFdSet  A pointer to the write file descriptors.
 * @param[inout]  aMaxFd       A pointer to the max file descriptor.
 *
 */
void platformUartUpdateFdSet(fd_set *aReadFdSet, fd_set *aWriteFdSet, fd_set *aErrorFdSet, int *aMaxFd);

/**
 * This function performs radio driver processing.
 *
 */
void platformUartProcess(void);

#endif  // PLATFORM_POSIX_H_
