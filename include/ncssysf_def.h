/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
 * under the GNU Lesser General Public License Version 2.1, February 1999.
 * The complete license can be accessed from the following location:
 * http://opensource.org/licenses/lgpl-license.php
 * See the Copying file included with the OpenSAF distribution for full
 * licensing terms.
 *
 * Author(s): Emerson Network Power
 *
 */

/*****************************************************************************
..............................................................................

$Header: /ncs/software/release/leap/leap.25/os_svcs/common/inc/sysf_def.h 49    10/08/01 4:01p Questk $



..............................................................................

  DESCRIPTION:

  This module contains target system specific declarations related to
  System "hooks" and other assorted defines.

..............................................................................
*/

/*
 * Module Inclusion Control...
 */
#ifndef NCSSYSF_DEF_H
#define NCSSYSF_DEF_H

#include "ncsgl_defs.h"

#ifdef  __cplusplus
extern "C" {
#endif

/****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 **                                                                        **
 **                                                                        **
 **                  Logging & File Interface                              **
 **                                                                        **
 **                                                                        **
 **                                                                        **
 **                                                                        **
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ***************************************************************************/

#define sysf_fprintf                m_NCS_OS_LOG_FPRINTF
#define sysf_fclose                 m_NCS_OS_LOG_FCLOSE
#define sysf_fopen(fname,fmode)     m_NCS_OS_LOG_FOPEN(fname,fmode)
#define sysf_sprintf                m_NCS_OS_LOG_SPRINTF
#define sysf_getc                   m_NCS_OS_GETC
#define sysf_fgets                  m_NCS_OS_FGETS

#define m_NCS_FOPEN_R(filename)      m_NCS_OS_FOPEN_R(filename)
#define m_NCS_LOG_WRITE(filename, string)  m_NCS_TS_LOG_WRITE(filename, string)



/****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 **                                                                        **
 **                                                                        **
 **                    CLIB Interfaces                                     **
 **                                                                        **
 **                                                                        **
 **                                                                        **
 **                                                                        **
 ****************************************************************************
 ****************************************************************************
 ****************************************************************************
 ***************************************************************************/



/*****************************************************************************
 **                                                                         **
 **                                                                         **
 **                      Byte order conversions                             **
 **                                                                         **
 **                                                                         **
 ****************************************************************************/
#define sysf_ntohl(x)     m_NCS_OS_NTOHL(x)
#define sysf_htonl(x)       m_NCS_OS_HTONL(x)
#define sysf_ntohs(s)       m_NCS_OS_NTOHS(s)
#define sysf_htons(s)       m_NCS_OS_HTONS(s)
/*****************************************************************************
 **                                                                         **
 **                                                                         **
 **  Function Defines for byte order conversions with                       **
 **  autoincrement of the pointer                                           **
 **                                                                         **
 ****************************************************************************/

EXTERN_C LEAPDLL_API uns32 decode_32bitOS_inc( uns8 **stream);
EXTERN_C LEAPDLL_API uns32 encode_32bitOS_inc( uns8 **stream, uns32 val);
EXTERN_C LEAPDLL_API uns32 encode_16bitOS_inc( uns8 **stream, uns32 val);
EXTERN_C LEAPDLL_API uns16 decode_16bitOS_inc( uns8 **stream);

#define m_NCS_OS_HTONL_P_INC(p8,v32) encode_32bitOS_inc(&p8, v32)
#define m_NCS_OS_HTONS_P_INC(p8,v16) encode_16bitOS_inc(&p8, v16)
#define m_NCS_OS_NTOHL_P_INC(p8)     decode_32bitOS_inc(&p8) 
#define m_NCS_OS_NTOHS_P_INC(p8)    decode_16bitOS_inc(&p8)


/*****************************************************************************
 **                                                                         **
 **                                                                         **
 **                Console Interfaces                                       **
 **                                                                         **
 **                                                                         **
 ****************************************************************************/
#define m_NCS_CONS_PUTCHAR        m_NCS_OS_PUTCHAR
#define m_NCS_CONS_GETCHAR        m_NCS_OS_GETCHAR




#ifndef m_NCS_CONS_VFPRINTF
#define m_NCS_CONS_VFPRINTF       vfprintf
#endif

#define m_NCS_CONS_UNBUF_GETCHAR  m_NCS_OS_UNBUF_GETCHAR

#define m_NCS_CONIO_NORMALIZE_CHR m_NCS_OS_NORMALIZE_CHR

/* redefine printf() and getchar() */
#ifdef NEW_GUI_CODE
#undef m_NCS_OS_PRINTF
extern LEAPDLL_API int send_cons_to_gui(const char *format , ...);
#define m_NCS_OS_PRINTF           send_cons_to_gui

#undef m_NCS_OS_GETCHAR
extern LEAPDLL_API int get_char_from_gui();
#define m_NCS_OS_GETCHAR           get_char_from_gui

#endif

/*****************************************************************************
 **                                                                         **
 **                                                                         **
 **                       Operating System Library Calls                    **
 **                                                                         **
 **                                                                         **
 ****************************************************************************/
#define sysf_srand             m_NCS_OS_SRAND
#define sysf_rand              m_NCS_OS_RAND
#define sysf_difftime(t0, t1)  m_NCS_OS_DIFFTIME (t0, t1)


#define m_NCS_SYSTEM            m_NCS_OS_SYSTEM
#define m_NCS_START(a,b)        m_NCS_OS_START(a,b)
#define m_NCS_ASSERT(exp)       m_NCS_OS_ASSERT(exp)

#define m_NCS_MEMCPY(d,s,n)     m_NCS_OS_MEMCPY(d,s,n)
#define m_NCS_MEMSET(b,c,n)     m_NCS_OS_MEMSET(b,c,n)
#define m_NCS_STRCPY(d,s)       m_NCS_OS_STRCPY(d,s)
#define m_NCS_STRNCPY(d,s,n)    m_NCS_OS_STRNCPY(d,s,n)
#define m_NCS_STRLEN(s)         m_NCS_OS_STRLEN(s)

#define m_NCS_STRCMP(s1,s2)     m_NCS_OS_STRCMP(s1,s2)
#define m_NCS_STRNCMP(s1,s2,n)  m_NCS_OS_STRNCMP(s1,s2,n)
#define m_NCS_STRCSPN(s,d)      m_NCS_OS_STRCSPN(s,d)


#define m_NCS_STRCAT(s1,s2)     m_NCS_OS_STRCAT(s1,s2)
#define m_NCS_STRNCAT(s1,s2,n)  m_NCS_OS_STRNCAT(s1,s2,n)
#define m_NCS_MEMCMP(d,s,n)     m_NCS_OS_MEMCMP(d,s,n)

/* Caution:  This macro is not THREAD SAFE. 
 * Use sysf_strtok_r for re-entrant functionality
 */
#define m_NCS_STRTOK(s,d)       m_NCS_OS_STRTOK(s,d)

#define m_NCS_STRTOK_R          sysf_strtok_r

#define m_NCS_CONS_PRINTF        m_NCS_OS_CONS_PRINTF

#define m_NCS_DBG_PRINTF         m_NCS_OS_DBG_PRINTF

#define m_NCS_SYSLOG             m_NCS_OS_SYSLOG


/*****************************************************************************
 **                                                                         **
 **                                                                         **
 **             Operating System Task Premption Lock macros                 **
 **                                                                         **
 **                                                                         **
 ****************************************************************************/

#define m_INIT_CRITICAL                m_NCS_OS_INIT_TASK_LOCK
#define m_START_CRITICAL               m_NCS_OS_START_TASK_LOCK
#define m_END_CRITICAL                 m_NCS_OS_END_TASK_LOCK


/* IR00061535 */
#define m_NCS_REBOOT                   m_NCS_OS_REBOOT


/*****************************************************************************
 **                                                                         **
 **                                                                         **
 **                   System Timestamp Interface Macros                     **
 **                                                                         **
 ** ncs_os_get_time_stamp:       Return the current timestamp as "time_t" in **
 **                             the argument tod.                           **
 **                                                                         **
 ** ncs_os_get_ascii_time_stamp: Fetch the current timestamp (tod), as an    **
 **                             ascii string, in asc_tod. Note the size of  **
 **                             the ASCII string is limited to 32 octets.   **
 **                                                                         **
 ****************************************************************************/

#define m_GET_ASCII_TIME_STAMP(timestamp, asc_timestamp)  \
    m_NCS_OS_GET_ASCII_TIME_STAMP(timestamp, asc_timestamp)

/* IR00061477 */
#define m_NCS_DATE_TIME_TO_STR(timestamp, asc_timestamp)  \
    m_NCS_OS_DATE_TIME_TO_STR(timestamp, asc_timestamp)
    
#define m_NCS_TIME_TO_STR(timestamp, asc_timestamp)  \
    m_NCS_OS_TIME_TO_STR(timestamp, asc_timestamp)

#define m_GET_ASCII_DATE_TIME_STAMP(timestamp, asc_timestamp) \
    m_NCS_OS_GET_ASCII_DATE_TIME_STAMP(timestamp, asc_timestamp)

#define m_GET_ASCII_HDR_DATE_TIME_STAMP(timestamp, asc_timestamp) \
    m_NCS_OS_GET_ASCII_HDR_DATE_TIME_STAMP(timestamp, asc_timestamp)

#define m_GET_TIME_STAMP(timestamp) \
    m_NCS_OS_GET_TIME_STAMP(timestamp)

#define m_NCS_GET_TIME_MS  \
    m_NCS_OS_GET_TIME_MS

#define m_NCS_GET_TIME_NS  \
    m_NCS_OS_GET_TIME_NS  /* IR00058792 */ 

#define m_NCS_GET_UPTIME \
    m_NCS_OS_GET_UPTIME  /* IR00084262 */

#define m_NCS_GETGRGID(gid) \
    m_NCS_OS_GETGRGID(gid)/* IR00059532 */ 

#define m_NCS_GETGROUPS(num_of_groups,list) \
    m_NCS_OS_GETGROUPS(num_of_groups,list) /* IR00059532 */

#define m_GET_TIME_STAMP_STR(timestamp, asc_timestamp)  \
    m_NCS_OS_GET_TIME_STAMP_STR(timestamp, asc_timestamp)

/** following is for subsystems which use the old style time stamping!
 **/
#define sysf_time_stamp  ncs_time_stamp()

EXTERN_C LEAPDLL_API time_t ncs_time_stamp(void);


/*****************************************************************************
 **                                                                         **
 **                                                                         **
 **             LEAP Debug conditional compile stuff                        **
 **                                                                         **
 **                                                                         **
 ****************************************************************************/

#define m_KEY_CHK_FMT(k,f)  { if (k.fmat != f) m_LEAP_DBG_SINK(0);}
#define m_KEY_CHK_LEN(l)    { if (l > SYSF_MAX_KEY_LEN) m_LEAP_DBG_SINK(0); }
#define m_KEY_CHK_SLEN(s)   { uns32 l = m_NCS_STRLEN(s); m_KEY_CHK_LEN(l); }


/*
 * m_LEAP_DBG_SINK
 *
 * If LEAP fails an if-conditional or other test that we would not expect
 * under normal conditions, it will call this macro.
 *
 * If ENABLE_LEAP_DBG == 1, fall into the sink function. A customer can put
 * a breakpoint there or leave the CONSOLE print statement, etc.
 *
 * If ENABLE_LEAP_DBG == 0, just echo the value passed. It is typically a
 * return code or a NULL.
 *
 * ENABLE_LEAP_DBG can be enabled in ncs_opt,h
 */


EXTERN_C LEAPDLL_API uns32 leap_dbg_sink (uns32,char*,long);

#define m_LEAP_GOTO_DBG_SINK(l,f,r) leap_dbg_sink(l,f,(uns32)r)
#define m_LEAP_DBG_SINK(r)          leap_dbg_sink(__LINE__,__FILE__,(long)r)
#define m_LEAP_DBG_SINK_VOID(r)          leap_dbg_sink(__LINE__,__FILE__,(uns32)r)


/*****************************************************************************
 **                                                                         **
 **                   Socket Interface Macros                               **
 **                                                                         **
 ****************************************************************************/
#define NCSSOCK_ERROR                       NCS_TS_SOCK_ERROR
#define NCSSOCK_INVALID                     NCS_TS_SOCK_INVALID
#define m_NCSSOCK_CREATE                    m_NCS_TS_SOCK_CREATE
#define m_NCSSOCK_DESTROY                   m_NCS_TS_SOCK_DESTROY
#define m_NCSSOCK_SOCKET              m_NCS_TS_SOCK_SOCKET
#define m_NCSSOCK_BIND                      m_NCS_TS_SOCK_BIND
#define m_NCSSOCK_CONNECT                   m_NCS_TS_SOCK_CONNECT
#define m_NCSSOCK_ACCEPT                    m_NCS_TS_SOCK_ACCEPT
#define m_NCSSOCK_LISTEN                    m_NCS_TS_SOCK_LISTEN
#define m_NCSSOCK_SELECT                    m_NCS_TS_SOCK_SELECT
#define m_NCSSOCK_RECV                      m_NCS_TS_SOCK_RECV
#define m_NCSSOCK_RECVFROM                  m_NCS_TS_SOCK_RECVFROM
#define m_NCSSOCK_SENDTO                    m_NCS_TS_SOCK_SENDTO
#define m_NCSSOCK_SEND                      m_NCS_TS_SOCK_SEND
#define m_NCSSOCK_CLOSE                     m_NCS_TS_SOCK_CLOSE
#define m_NCSSOCK_ERROR                     m_NCS_TS_SOCK_ERROR
#define m_NCSSOCK_IOCTL                     m_NCS_TS_SOCK_IOCTL
#define m_NCSSOCK_FIONBIO                   m_NCS_TS_SOCK_FIONBIO
#define m_NCSSOCK_FIONREAD                  m_NCS_TS_SOCK_FIONREAD
#define m_NCSSOCK_SETSOCKOPT                m_NCS_TS_SOCK_SETSOCKOPT
#define m_NCSSOCK_GETSOCKOPT                m_NCS_TS_SOCK_GETSOCKOPT
#define m_NCSSOCK_GETSOCKNAME               m_NCS_TS_SOCK_GETSOCKNAME
#define m_NCSSOCK_FD_ISSET                  m_NCS_TS_SOCK_FD_ISSET
#define m_NCSSOCK_FD_ISSET                  m_NCS_TS_SOCK_FD_ISSET
#define m_NCSSOCK_FD_ZERO                   m_NCS_TS_SOCK_FD_ZERO

#define m_NCSSOCK_SET_UDP_SADDR             m_NCS_TS_SOCK_SET_UDP_SADDR
#if (NCS_IPV6 == 1)
#define m_NCSSOCK_SET_IPV6_UDP_SADDR        m_NCS_TS_SOCK_SET_IPV6_UDP_SADDR
#define m_NCSSOCK_IPV6_MREQ_OBJ             m_NCS_TS_SOCK_IPV6_MREQ_OBJ
#define m_NCSSOCK_SET_IPV6_MULTIADDR        m_NCS_TS_SOCK_SET_IPV6_MULTIADDR
#endif

#define NCSSOCK_MREQ_OBJ                    NCS_TS_SOCK_MREQ_OBJ
#define m_NCSSOCK_SET_MULTIADDR             m_NCS_TS_SOCK_SET_MULTIADDR
#define m_NCSSOCK_SHARE                     m_NCS_TS_SOCK_SHARE
#define m_NCSSOCK_NEW_OWNER                 m_NCS_TS_SOCK_NEW_OWNER


/*****************************************************************************
 **                                                                         **
 **                   POSIX System call Macros                              **
 **                                                                         **
 ****************************************************************************/
#define m_NCS_POSIX_OPEN                    m_NCS_OS_POSIX_OPEN
#define m_NCS_POSIX_READ                    m_NCS_OS_POSIX_READ
#define m_NCS_POSIX_WRITE                   m_NCS_OS_POSIX_WRITE
#define m_NCS_POSIX_DUP                     m_NCS_OS_POSIX_DUP
#define m_NCS_POSIX_CLOSE                   m_NCS_OS_POSIX_CLOSE
#define m_NCS_POSIX_UNLINK                  m_NCS_OS_POSIX_UNLINK

#define m_NCS_POSIX_FORK                    m_NCS_OS_POSIX_FORK
#define m_NCS_POSIX_EXECVP                  m_NCS_OS_POSIX_EXECVP

#define m_NCS_POSIX_MKFIFO                  m_NCS_OS_POSIX_MKFIFO
#define m_NCS_POSIX_PIPE                    m_NCS_OS_POSIX_PIPE

#define m_NCS_POSIX_OPENLOG                 m_NCS_OS_POSIX_OPENLOG
#define m_NCS_POSIX_SYSLOG                  m_NCS_OS_POSIX_SYSLOG
#define m_NCS_POSIX_CLOSELOG                m_NCS_OS_POSIX_CLOSELOG


#define m_NCS_POSIX_SIGACTION               m_NCS_OS_POSIX_SIGACTION
#define m_NCS_POSIX_SIGADDSET               m_NCS_OS_POSIX_SIGADDSET
#define m_NCS_POSIX_SIGEMPTYSET             m_NCS_OS_POSIX_SIGEMPTYSET
#define m_NCS_POSIX_SIGPROCMASK             m_NCS_OS_POSIX_SIGPROCMASK


/*****************************************************************************
 **                                                                         **
 **                   Supplemental Macros                                   **
 **                                                                         **
 ****************************************************************************/

#define m_NCS_JSE_CFUNC                     m_NCS_OS_JSE_CFUNC
#define m_NCS_JSE_PFUNC                     m_NCS_OS_JSE_PFUNC
#define m_NCS_JSECALLSEQ(type)              m_NCS_OS_JSECALLSEQ(type)

#define NCS_PATH_SEPARATOR                  NCS_OS_PATHSEPARATOR_CHAR
#define NCS_PATH_MAX                        NCS_OS_PATH_MAX

#define MAX_FILENAME_LEN                   128

#define m_NCS_IPV4_TO_IFIDX(hbo_ipv4)       m_NCS_OS_IPV4_TO_IFIDX(hbo_ipv4)

#define NCS_HAVE_FLOATINGPOINT              NCS_OS_HAVE_FLOATINGPOINT

#define m_NCS_CUR_CPU_USAGE                 m_NCS_OS_CUR_CPU_USAGE

#define m_NCS_INIT_CPU_MON                  m_NCS_OS_INIT_CPU_MON

#define m_NCS_SHUTDOWN_CPU_MON              m_NCS_OS_SHUTDOWN_CPU_MON

/*****************************************************************************
 **                                                                         **
 **                   Task Priorities                                       **
 **                                                                         **
 ****************************************************************************/

#define NCS_TASK_PRIORITY_0               NCS_OS_TASK_PRIORITY_0
#define NCS_TASK_PRIORITY_1               NCS_OS_TASK_PRIORITY_1
#define NCS_TASK_PRIORITY_2               NCS_OS_TASK_PRIORITY_2
#define NCS_TASK_PRIORITY_3               NCS_OS_TASK_PRIORITY_3
#define NCS_TASK_PRIORITY_4               NCS_OS_TASK_PRIORITY_4
#define NCS_TASK_PRIORITY_5               NCS_OS_TASK_PRIORITY_5
#define NCS_TASK_PRIORITY_6               NCS_OS_TASK_PRIORITY_6
#define NCS_TASK_PRIORITY_7               NCS_OS_TASK_PRIORITY_7
#define NCS_TASK_PRIORITY_8               NCS_OS_TASK_PRIORITY_8
#define NCS_TASK_PRIORITY_9               NCS_OS_TASK_PRIORITY_9
#define NCS_TASK_PRIORITY_10              NCS_OS_TASK_PRIORITY_10
#define NCS_TASK_PRIORITY_11              NCS_OS_TASK_PRIORITY_11
#define NCS_TASK_PRIORITY_12              NCS_OS_TASK_PRIORITY_12
#define NCS_TASK_PRIORITY_13              NCS_OS_TASK_PRIORITY_13
#define NCS_TASK_PRIORITY_14              NCS_OS_TASK_PRIORITY_14
#define NCS_TASK_PRIORITY_15              NCS_OS_TASK_PRIORITY_15
#define NCS_TASK_PRIORITY_16              NCS_OS_TASK_PRIORITY_16


/*****************************************************************************
 **                                                                         **
 **                   m_NCS_SYSLOG Priorities                               **
 **                                                                         **
 ****************************************************************************/

#define  NCS_LOG_EMERG      NCS_OS_LOG_EMERG   /* system is unusable */
#define  NCS_LOG_ALERT      NCS_OS_LOG_ALERT  /* action must be taken immediately */
#define  NCS_LOG_CRIT       NCS_OS_LOG_CRIT   /* critical conditions */
#define  NCS_LOG_ERR        NCS_OS_LOG_ERR      /* error conditions */
#define  NCS_LOG_WARNING    NCS_OS_LOG_WARNING   /* warning conditions */
#define  NCS_LOG_NOTICE     NCS_OS_LOG_NOTICE  /* normal but significant condition */
#define  NCS_LOG_INFO       NCS_OS_LOG_INFO   /* informational */
#define  NCS_LOG_DEBUG      NCS_OS_LOG_DEBUG  /* debug-level messages */


/*****************************************************************************
 **                                                                         **
 **                   Task Stack Size                                       **
 **                                                                         **
 ****************************************************************************/

/* (relative) stack size options... Most all tasks use MEDIUM */

#ifndef NCS_STACKSIZE_SMALL
#define NCS_STACKSIZE_SMALL    16000
#endif

#ifndef NCS_STACKSIZE_MEDIUM
#define NCS_STACKSIZE_MEDIUM   32000
#endif

#ifndef NCS_STACKSIZE_LARGE
#define NCS_STACKSIZE_LARGE    64000
#endif

#ifndef NCS_STACKSIZE_HUGE
#define NCS_STACKSIZE_HUGE     128000
#endif

/*****************************************************************************
 **                                                                         **
 **                   Replacement strtoks                                   **
 **                                                                         **
 ****************************************************************************/

EXTERN_C LEAPDLL_API int8 *sysf_strtok(int8 *s, const int8 *delim);
EXTERN_C LEAPDLL_API int8 *sysf_strtok_r(int8 *s, const int8 *delim, int8 **stores);
EXTERN_C LEAPDLL_API int32 sysf_strrcspn(const uns8 *s, const int32 start_pos, const uns8 *reject);
EXTERN_C LEAPDLL_API int32 sysf_strincmp(const uns8 *s1, const uns8 *s2, uns32 n);


/*****************************************************************************
 **                                                                         **
 **                              Stack Trace                                **
 **                                                                         **
 ****************************************************************************/
#if (NCS_MMGR_STACKTRACE == 1)

#define NCS_STACKTRACE_INFO       NCS_OS_STACKTRACE_INFO

#define m_NCS_STACKTRACE_GET      m_NCS_OS_STACKTRACE_GET
#define m_NCS_STACKTRACE_EXPAND   m_NCS_OS_STACKTRACE_EXPAND
#endif  /*#if (NCS_MMGR_STACKTRACE == 1)*/


#ifdef  __cplusplus
}
#endif

/*****************************************************************************
 **                                                                         **
 **             LEAP ENVIRONMENT INITIALIZATION AND CLEAN UP                **
 **                                                                         **
 ****************************************************************************/

EXTERN_C LEAPDLL_API uns32 leap_env_init   (void);
EXTERN_C LEAPDLL_API uns32 leap_env_destroy(void);

#endif


