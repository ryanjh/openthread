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

#include "platform-posix.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <openthread/platform/alarm.h>
#include <openthread/platform/diag.h>

static struct timeval s_start;
static bool isInitialized = false;

static otPlatformAlarm sPlatformAlarm =
{
    .s_is_running = false,
    .s_alarm = 0,
};

static inline otPlatformAlarm* getPlatformAlarm(otInstance *aInstance)
{
#ifdef OPENTHREAD_MULTIPLE_INSTANCE
    otPlatformInstance* pfInstance = getPlatformInstance(aInstance);
    return &(pfInstance->platformAlarm);
#else
    return &sPlatformAlarm;
#endif //OPENTHREAD_MULTIPLE_INSTANCE
}

void platformAlarmInit(void)
{
    if (!isInitialized)
    {
        gettimeofday(&s_start, NULL);
        isInitialized = true;
    }
}

void platformAlarmCopy(otInstance *aInstance)
{
    memcpy(getPlatformAlarm(aInstance), &sPlatformAlarm, sizeof(otPlatformAlarm));
}

uint32_t otPlatAlarmGetNow(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    timersub(&tv, &s_start, &tv);

    return (uint32_t)((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void otPlatAlarmStartAt(otInstance *aInstance, uint32_t t0, uint32_t dt)
{
    otPlatformAlarm *platformAlarm = getPlatformAlarm(aInstance);
    platformAlarm->s_alarm = t0 + dt;
    platformAlarm->s_is_running = true;
}

void otPlatAlarmStop(otInstance *aInstance)
{
    getPlatformAlarm(aInstance)->s_is_running = false;
}

void platformAlarmUpdateTimeout(otInstance *aInstance, struct timeval *aTimeout)
{
    int32_t remaining;
    otPlatformAlarm *platformAlarm = getPlatformAlarm(aInstance);

    if (aTimeout == NULL)
    {
        return;
    }

    if (platformAlarm->s_is_running)
    {
        remaining = (int32_t)(platformAlarm->s_alarm - otPlatAlarmGetNow());

        if (remaining > 0)
        {
            aTimeout->tv_sec = remaining / 1000;
            aTimeout->tv_usec = (remaining % 1000) * 1000;
        }
        else
        {
            aTimeout->tv_sec = 0;
            aTimeout->tv_usec = 0;
        }
    }
    else
    {
        aTimeout->tv_sec = 10;
        aTimeout->tv_usec = 0;
    }
}

void platformAlarmProcess(otInstance *aInstance)
{
    int32_t remaining;
    otPlatformAlarm *platformAlarm = getPlatformAlarm(aInstance);

    if (platformAlarm->s_is_running)
    {
        remaining = (int32_t)(platformAlarm->s_alarm - otPlatAlarmGetNow());

        if (remaining <= 0)
        {
            platformAlarm->s_is_running = false;

#if OPENTHREAD_ENABLE_DIAG

            if (otPlatDiagModeGet())
            {
                otPlatDiagAlarmFired(aInstance);
            }
            else
#endif
            {
                otPlatAlarmFired(aInstance);
            }
        }
    }
}
