/*
 * Multiplatform Async Network Library
 * Copyright (c) 2007 Burlex
 *
 * SocketDefines.h - Any platform-specific defines/includes go here.
 *
 */

#ifndef SOCKET_DEFINES_H
#define SOCKET_DEFINES_H

#include "Logging/Log.hpp"
#include <cstring>

/* Implementation Selection */
#ifdef WIN32        // Easy
#define CONFIG_USE_IOCP
#else

// unix defines
#define SOCKET int
#define SD_BOTH SHUT_RDWR

#if __linux__

// select: epoll
#include <sys/epoll.h>
#define CONFIG_USE_EPOLL

#elif __FreeBSD__

// in case of freebsd 12+ we need just event.h but in 11 10 end etc need this all
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/errno.h>
#define CONFIG_USE_KQUEUE

#elif __APPLE__
// select: kqueue
#include <sys/event.h>
#define CONFIG_USE_KQUEUE


#endif

#endif

/* IOCP Defines */

#ifdef CONFIG_USE_IOCP
enum SocketIOEvent
{
    SOCKET_IO_EVENT_READ_COMPLETE   = 0,
    SOCKET_IO_EVENT_WRITE_END        = 1,
    SOCKET_IO_THREAD_SHUTDOWN        = 2,
    NUM_SOCKET_IO_EVENTS            = 3,
};

class OverlappedStruct
{
    public:
        OVERLAPPED m_overlap;
        SocketIOEvent m_event;
        volatile long m_inUse;
        OverlappedStruct(SocketIOEvent ev) : m_event(ev)
        {
            std::memset(&m_overlap, 0, sizeof(OVERLAPPED));
            m_inUse = 0;
        };

        OverlappedStruct()
        {
            std::memset(&m_overlap, 0, sizeof(OVERLAPPED));
            m_inUse = 0;
        }

        __forceinline void Reset(SocketIOEvent ev)
        {
            std::memset(&m_overlap, 0, sizeof(OVERLAPPED));
            m_event = ev;
        }

        void Mark()
        {
            long val = InterlockedCompareExchange(&m_inUse, 1, 0);
            if(val != 0)
                printf("!!!! Network: Detected double use of read/write event! Previous event was %u.", m_event);
        }

        void Unmark()
        {
            InterlockedExchange(&m_inUse, 0);
        }
};

#endif

#endif  //SOCKET_DEFINES_H
