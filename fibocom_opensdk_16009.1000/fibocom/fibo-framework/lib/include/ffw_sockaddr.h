/**
 * @file ffw_sockaddr.h
 * @author your name (you@domain.com)
 * @brief 此文件定义了socket编程需要的是地址，struct sockaddr、struct sockaddr_storage、struct sockaddr_in、struct sockaddr_in6
 *        如果有些平台没有定义这些结构体，需要在ffw_plat_header.h包含此文件，如果平台本身已有，忽略此文件（如Linux）
 * @version 0.1
 * @date 2021-10-31
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef B6AE497C_E752_4ED0_BCD1_E6B1A148C618
#define B6AE497C_E752_4ED0_BCD1_E6B1A148C618

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

/* POSIX.1g specifies this type name for the `sa_family' member.  */
typedef unsigned short int sa_family_t;

/* This macro is used to declare the initial common members
   of the data types used for socket addresses, `struct sockaddr',
   `struct sockaddr_in', `struct sockaddr_un', etc.  */
#define __SOCKADDR_COMMON(sa_prefix) sa_family_t sa_prefix##family

/* Size of struct sockaddr_storage.  */
#define _SS_SIZE 128
#define __SOCKADDR_COMMON_SIZE (sizeof(unsigned short int))
#define __ss_aligntype unsigned long int
#define _SS_PADSIZE (_SS_SIZE - __SOCKADDR_COMMON_SIZE - sizeof(__ss_aligntype))

/* Structure describing a generic socket address.  */
struct sockaddr
{
    __SOCKADDR_COMMON(sa_); /* Common data: address family and length.  */
    char sa_data[14];       /* Address data.  */
};

///struct sockaddr_storage --> struct sci_sockaddrext
struct sockaddr_storage
{
    __SOCKADDR_COMMON(ss_); /* Address family, etc.  */
    char __ss_padding[_SS_PADSIZE];
    __ss_aligntype __ss_align; /* Force desired alignment.  */
};

/* Internet address.  */
typedef uint32_t in_addr_t;
typedef uint16_t in_port_t;
struct in_addr
{
    in_addr_t s_addr;
};

///struct sockaddr_in --> struct sci_sockaddr
struct sockaddr_in
{
    __SOCKADDR_COMMON(sin_);
    in_port_t sin_port;      /* Port number.  */
    struct in_addr sin_addr; /* Internet address.  */
    /* Pad to size of `struct sockaddr'.  */
    unsigned char sin_zero[sizeof(struct sockaddr) - __SOCKADDR_COMMON_SIZE - sizeof(in_port_t) - sizeof(struct in_addr)];
};

/* IPv6 address */
struct in6_addr
{
    union {
        uint8_t __u6_addr8[16];
        uint16_t __u6_addr16[8];
        uint32_t __u6_addr32[4];
    } __in6_u;
    // #define s6_addr __in6_u.__u6_addr8
};

///struct sockaddr_in --> struct sci_sockaddr6
struct sockaddr_in6
{
    __SOCKADDR_COMMON(sin6_);
    in_port_t sin6_port;       /* Transport layer port # */
    uint32_t sin6_flowinfo;    /* IPv6 flow information */
    struct in6_addr sin6_addr; /* IPv6 address */
    uint32_t sin6_scope_id;    /* IPv6 scope-id */
};

/* Structure to contain information about address of a service provider.  */
struct addrinfo
{
    int ai_flags;             /* Input flags.  */
    int ai_family;            /* Protocol family for socket.  */
    int ai_socktype;          /* Socket type.  */
    int ai_protocol;          /* Protocol for socket.  */
    int ai_addrlen;     /* Length of socket address.  */
    struct sockaddr *ai_addr; /* Socket address for socket.  */
    char *ai_canonname;       /* Canonical name for service location.  */
    struct addrinfo *ai_next; /* Pointer to next in list.  */
};

struct timeval
{
    long tv_sec;           /* seconds */
    unsigned long tv_usec; /* microseconds */
};

#define IPPROTO_TCP 6
#define IPPROTO_UDP 17

#define SOL_SOCKET 1

#endif /* B6AE497C_E752_4ED0_BCD1_E6B1A148C618 */
