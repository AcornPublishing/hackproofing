/*
Copyright (c) 2000  by Network ICE Corporation.  All rights reserved.
The  source  code to this program  is being  publicly  disclosed  for
purposes  of  discussion.  However,  this  source code  is not in the
public domain (or open-source or GPL).  However,  we plan to GPL this
code in the future.

A license is hearby granted to  use this software only in cases where
an individual monitors his/her own network traffic. If you would like
to purchase a license for other uses of this software, please contact
<sales@altivore.com>.  Using this software to surreptitiously monitor
other people's data may be in violation of USC 18 2512.


ALTIVORE v0.9.3

  This is a  sample program  containing  some of the  features of the
  features of the FBI's "Carnivore" program.  It is intended to serve
  as a point of discussion about Carnivore features.  It has not been
  thoroughly tested and contains numerous bugs.

  This may also serve as an "alternative" for ISPs who do not wish to
  install a black-box from the FBI.  Court orders demanding data from
  the  ISP do not necessarily  require that Carnivore must be used if
  the ISP is able to obtain the data in another manner.

  This software  may also  be useful in  network management,  such as
  backing up data or  sniffing a  consumer's  dial-up connection when
  they are reporting problems to customer support.

HOW TO USE THIS SOFTWARE

  This software must be compiled and linked with the libpcap library.
  Libpcap  must likewise be  installed on the  system in order to for
  this to run.

  This  software has  been compiled  and briefly tested under Windows
  and Linux. It should run on  pretty much any system with some minor
  adjustments.

  Windows Compilation

  Download WINPCAP developers kit from: 
    http://netgroup-serv.polito.it/winpcap/
  Point the include directory to "WPdpack/include" and link  with the
  libraries "libpcap.lib", "user32.lib", and "wsock32.lib".

  Linux Compilation

  gcc altivore.c -lpcap -Ipcap -o altivore

  Note: libpcap broken on RedHat 6.2

WHAT DATA IS COLLECTED?

  This  module was  written  to  match  the  FBI's  solicitation  for
  indepedent  technical review  of Carnivore  that was  published  on
  August 25, 2000.  Attachment 1  of that document describes  several
  scenarios for Carnivore usage.

  Throughout this document,  the term  "Alice" refers to the criminal
  suspect whose email is being monitored.  The term  "Bob"  refers to
  the person Alice  is communicating with.  The sections below can be
  copied/pasted into the file "altivore.ini", which this program uses
  to store its configuration information.

  [1.1 email header pen-register]
    ;Monitors all the email headers to and from Alice's
    ;account. This does not capture the "Subject:" field, 
    ;which is considered by courts to be part of the "data" 
    ;rather than the "call records". This should be 
    ;deployed on or near the email server that processes 
    ;Alice's mail.
    mode = email-headers
    email.address = alice@example.com
    logfile = alice.txt

  [1.2 HTTP pen-register from dial-up user]
    ;Monitors the IP address of web-sites the user visits. 
    ;A complication in this is that the user dials-up and 
    ;receives a unique IP address each time. We monitor 
    ;the dial-up password protocol known as "RADIUS" in 
    ;order to trigger when Alice logs on and in order to 
    ;find out what IP address she is using. This should be 
    ;deployed on a segment behind the bank of dialup servers 
    ;as well as where it can sniff the RADIUS packets. This
    ;version of Carnivore only monitors Accounting packets; 
    ;you may have to enable this feature in order to get 
    ;this to work right.
    mode = server-access
    radius.account = alice@example.com
    server.port = 80
    logfile = alice.csv

  [1.3 FTP pen-register from dial-up user]
    ;Same as above, but monitors FTP instead of HTTP.
    mode = server-access
    radius.account = alice@example.com
    server.port = 80
    logfile = alice.csv

  [2.1 email content-wiretap]
    ;Instead of capturing just the headers, this scenario 
    ;captures the full contents of the email
    mode = email-content
    email.address = alice@example.com
    tracefile = alice.tcp
    
  [2.2 email content-wiretap]
    ;Captures the full content to/from a specific IP 
    ;address. This is the same as running the freeware 
    ;product called TCPDUMP. Example: 
    ;    tcpdump -w tracefile.tcp host 192.0.2.189
    mode = ip-content
    ip.address = 192.0.2.189
    tracefile = alice.tcp
    

DESIGN

  No reassembly/reordering
    This software does not support  IP fragmentation  or  TCP segment
    reordering.  As a result, it may miss some emails or accidentally
    include  segments from other people's  emails.  This is a crucial
    area  of discussion;  fragmentation issues are  an important flaw
    in many products, and is likely a flaw of Carnivore as well.
    
  Little SMTP server state
    Altivore  only monitors a  little bit of SMTP server state (it is
    impossible  to fully support  SMTP  state without  reassembly and 
    re-ording of fragments). As a result, it may indvertently capture
    email not belonging to Alice (the suspect).  For example,  if the
    system is unable to determine when an email message ends,  it may
    accidentally capture subsequent emails transfered across the same
    SMTP connection.  It is believed that  this is a problem with the
    FBI's Carnivore as well.

  RADIUS incomplete
    This RADIUS parsing code has only been tested at a few ISPs. This
    is a concern in some deployments  because it won't work.  One way
    arround  this is to force  RADIUS  Accounting  during  deployment.
    More work on RADIUS decoding needs to be done with Altivore.

  Evidence Authentication
    Evidence handling is a big concern. Altivore and Carnivore really
    should support MD5, PGP, or X.509 private-key signing in order to
    fully  authenticate files.  This would  detect later unauthorized
    tampering of the evidence.

ALTIVORE VS. NETWORK ICE

  Network ICE is  a leading  software vendor  of products  similar to
  this technology.  The  "sniff" network traffic looking for signs of
  hacker activity in order to  protect customer networks. Our primary
  competitive advantages are our  stateful protocol decoding features
  and high-speed sniffing. This means we can monitor gigabit networks
  with full packet reassembly and application protocol state.

  In contrast,  Carnivore was probably written using many of the same
  short-cuts that our competitors have taken.  We've written Altivore
  using similar short-cuts  in order to demonstrate the problems with
  this approach.  We've included a small amount  of state in order to
  show why stateful inspection is needed in this class of products.

*/
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>

/* Links to the libpcap library, standard library on Windows and 
 * UNIX for sniffing.*/
#include <pcap.h>


#define HASH_ENTRIES        1024
#define ADD_IF_NOT_FOUND    1
#define IGNORE_IF_NOT_FOUND 0
#define TCP_TO_SERVER       0x100
#define TCP_FROM_SERVER     0x000

/** Maximum length of an email address. Portions of the address
 * longer than this length are ignored. */
#define MAX_ADDRESS_LENGTH 1024

/** Maximum number of recipients. More recipients than this are 
 * ignored. */
#define MAX_RECIPIENTS 100

#undef TRUE
#define TRUE 1
#undef FALSE
#define FALSE 0

/** For pretty printing IP addresses */
#define _XIP(a,n) (int)(((a)>>(n))&0xFF)
#define P_IP_ADDR(a) _XIP(a,24), _XIP(a,16), _XIP(a,8), _XIP(a,0)

/**
 * TCP/IP protocol extraction stuff.
 */
#define ex8(p,f)            ((p)[f]) 
#define ex16(p,f)            ((p)[f] << 8 | (p)[f+1])
#define ex32(p,f)            ((ex16(p,f)<<16) | ex16(p,f+2))
#define IP_VERSION(p,f)            ((ex8(p,f+0) >> 4) & 0x0F)
#define IP_SIZEOF_HDR(p,f)        ((ex8(p,f+0) & 0x0F) * 4)
#define IP_TOTALLENGTH(p,f)        ex16(p,f+2)
#define IP_PROTOCOL(p,f)        ex8(p,f+9)
#define IP_SRC(p,f)                ex32(p,f+12)                
#define IP_DST(p,f)                ex32(p,f+16)
#define TCP_SRC(p,f)            ex16(p,f+0)
#define TCP_DST(p,f)            ex16(p,f+2)
#define TCP_SEQNO(p,f)            ex32(p,f+4)
#define TCP_ACKNO(p,f)            ex32(p,f+8)
#define TCP_FLAGS(p,f)            (ex8(p,f+13)&0x3F)
#define TCP_SIZEOF_HDR(p,f)        (((ex8(p,f+12)>>4) & 0x0f)*4)
#define TCP_FIN                    1
#define TCP_SYN                    2
#define TCP_RST                    4

#define FREE(x) if (x) free(x)

/**
 * A utility function for assigning strings. It solves several
 * string handling issues, such as copying over "counted strings"
 * rather than NUL-terminated strings.
 */
static void 
setString(char **r_str, const void *vstr, int offset, int len)
{
    const char *str = vstr; /*kludge: avoid warnings*/
    if (*r_str)
        free(*r_str);
    if (str == NULL) {
        *r_str = NULL;
        return;
    }
    if (len == -1)
        len = strlen((const char*)str);
    *r_str = (char*)malloc(len+1);
    memcpy(*r_str, str+offset, len);
    (*r_str)[len] = '\0';
}

/** Case-insensitive memcmp() */
static int 
memcasecmp(const void *lhs, const void *rhs, int length)
{
    int i;
    for (i=0; i<length; i++) {
        if (tolower(((char*)lhs)[i]) != tolower(((char*)rhs)[i]))
            return -1;
    }
    return 0;
}

/** Utility for case-insensitive comparisons*/
static int 
startsWith(const char lhs[], const char rhs[])
{
    int len = strlen(lhs);
    if ((int)strlen(rhs) < len)
        len = strlen(rhs);
    return memcmp(lhs, rhs, len) == 0;
}


/** 
 * Encapsulates the idea of an array of nul terminated strings. Use
 * straXXXX() functions.
 */
struct stringarray {
    char **str;
    int length;
    int max;
};
typedef struct stringarray stringarray;


/** stringarray.straAddElement()
 * Appends a string onto the end of an array of strings.
 */
void 
straAddElement(stringarray *lhs, const char rhs[])
{
    if (lhs->length + 1 >= lhs->max) {
        int new_max = lhs->max * 2 + 1;
        char **new_array = (char**)malloc(sizeof(char*)*(new_max));
        if (lhs->str) {
            memcpy(    new_array, 
                    lhs->str, 
                    sizeof(new_array[0])*lhs->length);
            free(lhs->str);
        }
        lhs->str = new_array;
        lhs->max = new_max;
    }
    lhs->str[lhs->length] = strdup(rhs);
    lhs->length++;
}


/** 
 * These are the several modes that Carnivore/Altivore can run as.
 * See the explanation above for more information on how to 
 * configure these modes.
 */
enum {
    /** Capture the headers of email to a text file */
    mode_email_headers = 1,

    /** Capture just the addresses to/from Alice */
    mode_email_addresses,

    /** Record accesses to servers with a specific TCP port. */
    mode_server_access,

    /** Record the full email content for Alice's email*/
    mode_email_content,

    /** Record a full sniffer trace for the indicated
     * IP address. */
    mode_ip_content
};
#define MODE(carn, m) ((carn)->mode == m)
static const char *modeNames[] = {"unspecified", "email-headers",
        "email-addresses", "server-access", "email-content",
        "ip-content", 0};

int 
parseMode(const char modeName[])
{
    int i;

    for (i=0; modeNames[i]; i++) {
        if (strcmp(modeName, modeNames[i]) == 0)
            return i;
    }
    return 0;
}

struct intlist {
    int list[32];
    int count;
};
typedef struct intlist intlist;

/**
 * The root object for the Carnivore system.
 */
struct Carnivore
{
    /** What mode of operation we are in */
    int mode;

    /** The name of the sniffer compatible tracefile that data will
     * be copied to (when doing full-content wiretaps).*/
    char *tracefile;
    FILE *fp_trace;
    
    /** Logfile for text information. */
    char *logfile;

    /** A list of IP addresses to filter for. This is used when a 
     * court order specifies IP addresses. TODO: allow ranges and
     * more IP addresses.*/
    intlist ip;

    /** Contains a list of ports that we will use in order to
     * monitor when a certain type of server has been accessed.
     */
    intlist port;

    /** TCP/IP connection table for maintaining session state*/
    struct TcpCnxn *cxTable[HASH_ENTRIES];
    int cxId;

    /** Whether or not we should save the last frame to a file */
    int do_filter;

    /** Whether or not we should remove this connection from
     * our list. */
    int do_remove;

    /** A list of email addresses. We compare these addresses to
     * emails as they go by in order to determine if we need to
     * make a copy. */
    stringarray email_addresses;

    /** A list of RADIUS account names that we should monitor
     * when doing IP wiretaps. */
    stringarray radius_accounts;

    /** An array of tracefiles that we will read in order to test
     * the system. They must be in tcpdump/libpcap format. */
    stringarray testinput;

    /** An array of adapter names that we need to open in
     * promiscuous mode. */
    stringarray interfaces;
};
typedef struct Carnivore Carnivore;

/** 
 * Test to see if either the source or destination IP address is
 * being filtered for. If we are filtering for this IP address,
 * then we'll likely save it to a file. Note that we are doing a
 * linear search through the array on the assumption that we are
 * filtering only a few IP addresses, often just a single one.
 */
int 
has_integer(intlist *ip, int ip1, int ip2)
{
    int i;
    for (i=0; i<ip->count; i++) {
        if (ip->list[i] == ip1 || ip->list[i] == ip2)
            return 1;
    }
    return 0;
}

/** Adds the specified IP address to the list of addresses that we
 * are filtering for. This may be a configured IP address or one 
 * that is auto-configured by the RADIUS parsing. */
void 
add_integer(intlist *ip, int ip_address)
{
    if (ip_address == 0)
        return; /*ignore empty IP addresses*/
    if (has_integer(ip, ip_address, ip_address))
        return; /*ignore duplicates*/
    if (ip->count < sizeof(ip->list)/sizeof(int)) {
        ip->list[ip->count] = ip_address;
        ip->count++;
    }
}

/** Delete an IP address from the list of filters. This is called
 * when the RADIUS parsing determines that the monitored user has
 * hung up. */
void 
del_integer(intlist *ip, int ip_address) 
{
    int i;
    for (i=0; i<ip->count; i++) {
        if (ip->list[i] == ip_address) {
            memmove(ip->list+i, ip->list+i+1, 
                            (ip->count - i - 1)*sizeof(int));
            ip->count--;
        }
    }
}



/** matchName()
 * Tests to see if the desired email address should be filtered
 * for. This is presumably the email address of somebody that we
 * have a court-order to  monitor.
 */
int 
matchName(const char addr[], int addr_len, stringarray *list)
{
    int i;
    if (addr == NULL)
        return 0;
    for (i=0; i<list->length; i++) {
        int lhs_len = strlen(list->str[i]);
        if (list->str[i][0] == '*') {
            /*match end of string, e.g. allow specification of
             * "*@suspect.com" to match any emails for a domain*/
            if (addr_len >= lhs_len - 1) {
                const char *new_lhs = list->str[i]+1;
                const char *new_addr = addr+addr_len-lhs_len+1;
                if (memcasecmp(new_lhs, new_addr, lhs_len-1) == 0)
                    return TRUE;
            }
        }
        else if (addr_len == lhs_len 
                && memcasecmp(list->str[i], addr, addr_len) == 0)
            return TRUE;
    }
    return FALSE;
}


/**
 * A TCP connection entry. We maintain one of these for every
 * outstanding connection that we might be tracking. This contains
 * the basic TCP info, as well as some higher level protocol info
 * for SMTP.
 */
struct TcpCnxn
{
    /** Each new connection is identified with a unique ID */
    int msg_id;
    int server_ip;
    int client_ip;
    int server_port;
    int client_port;
    int server_seqno;
    int client_seqno;
    struct TcpCnxn *next;
    time_t creation_time;
    char *sender;
    int sender_matches;
    char *recipient;
    stringarray recipients;
    
    /** Whether or not we should save the email message for this
     * connection. */
    int do_filter;

    /** Whether we should filter this one frame. We need this in
     * order to capture the trailing dot that ends an email message. 
     */
    int filter_one_frame;

    /** Whether or not we should remove this connection entry at
     * the next opportunity. */
    int do_remove;

    /** Whether we are parsing the 'envelope' or the message
     * itself. */
    int state;
};
typedef struct TcpCnxn TcpCnxn;


/**
 * Create a hash entry for our table. The hash entry is based
 * only on the IP addresses and port numbers. The exact
 * hash algorithm is unimportant, and should be adjusted over
 * time to produce the best results. Note that since we've
 * already converted the (src,dst) to (srvr,clnt), we don't
 * need to make the hash symmetric.
 */
int 
cxHash(TcpCnxn *cx)
{
    int result = 0;
    result = abs((cx->server_ip ^ (cx->client_ip*2)) 
                ^ ((cx->server_port<<16) | cx->client_port));
    return result % HASH_ENTRIES;
}


/**
 * Compares two connection objects in order to see if they are the 
 * same one. Only IP address and TCP port info is used in this 
 * comparison.
 */
int 
cxEquals(TcpCnxn *lhs, TcpCnxn *rhs)
{
    if (lhs->server_ip != rhs->server_ip)
        return 0;
    if (lhs->client_ip != rhs->client_ip)
        return 0;
    if (lhs->server_port != rhs->server_port)
        return 0;
    if (lhs->client_port != rhs->client_port)
        return 0;
    return 1;
}


/**
 * Looks up a TCP connection object within our table. If not found,
 * it may add it (depending upon a parameter).
 * @param carn
 *        This object.
 * @param rhs
 *        A copy of the connection object we are looking up (we simply
 *        pull out the address/ports from this to compare them).
 * @param add_if_not_found
 *        Whether we should add a new connection object if we cannot
 *        find an    existing one. It is important that we only add
 *        connection objects during a SYN/SYN-ACK in order to avoid
 *        accidentally getting state in the middle of the connection.
 */
TcpCnxn *
cxLookup(Carnivore *carn, TcpCnxn *rhs, int add_if_not_found)
{
    int h = cxHash(rhs);
    TcpCnxn **r_cx = &carn->cxTable[h];
    
    for (;;) {
        if (*r_cx == NULL) {
            /* The connection object wasn't found. If this was
             * a SYN or SYN-ACK, then we'll need to add this
             * connection. */
            if (add_if_not_found) {
                *r_cx = (TcpCnxn*)malloc(sizeof(TcpCnxn));
                memset(*r_cx, 0, sizeof(**r_cx));
                (*r_cx)->server_ip = rhs->server_ip;
                (*r_cx)->client_ip = rhs->client_ip;
                (*r_cx)->server_port = rhs->server_port;
                (*r_cx)->client_port = rhs->client_port;
                (*r_cx)->server_seqno = rhs->server_seqno;
                (*r_cx)->client_seqno = rhs->client_seqno;
                (*r_cx)->creation_time = time(0);
            }
            return *r_cx;
        }

        if (cxEquals(*r_cx, rhs))
            return *r_cx;
        else
            r_cx = &(*r_cx)->next;
    }
}


/**
 * Resets the SMTP protocol info back to a known state. It is
 * important that this be as delicate as possible: it should reset
 * data at the slightest provocation in order to avoid accidentally
 * capturing somebody else's email.
 */
void 
cxResetMsg(TcpCnxn *cx)
{
    cx->do_filter = FALSE; /*don't capture these emails*/
    if (cx->sender) {
        free(cx->sender);
        cx->sender = NULL;
    }
    cx->sender_matches = FALSE;
    if (cx->recipients.length) {
        int i;
        for (i=0; i<cx->recipients.length; i++)
            free(cx->recipients.str[i]);
        free(cx->recipients.str);
        cx->recipients.str = NULL;
        memset(&cx->recipients, 0, sizeof(cx->recipients));
    }
}


/**
 * Removes a TCP connection object from our table. This is called
 * whenever we reach the end of SMTP processing, the TCP connection
 * closes, or when we timeout and clean up a connection.
 */
void 
cxRemove(Carnivore *carn, TcpCnxn *rhs)
{
    int h = cxHash(rhs);
    TcpCnxn **r_cx = &carn->cxTable[h];
    for (;;) {
        if (*r_cx == NULL)
            break; /*not found*/
        else if (cxEquals(*r_cx, rhs)) {
            TcpCnxn *cx = *r_cx;
            *r_cx = cx->next;
            cxResetMsg(cx);
            free(cx);
            break;
        }
        else
            r_cx = &(*r_cx)->next;
    }
}


/** Writes a little-endian integer to the buffer */
void 
writeint(unsigned char hdr[], int offset, int x)
{
    hdr[offset+0] = (unsigned char)(x>>0);
    hdr[offset+1] = (unsigned char)(x>>8);
    hdr[offset+2] = (unsigned char)(x>>16);
    hdr[offset+3] = (unsigned char)(x>>24);
}


/**
 * Saves the current packet to a TCPDUMP compatible file. Note
 * that I could use the built-in libpcap file saving mechanism
 * but I want to eventually at digital-signatures, so I'll be
 * doing strange stuff with the file in the future.
 */
void 
carnSavePacket(Carnivore *carn, const unsigned char buf[], 
        int orig_len, time_t timestamp, int usecs)
{
    unsigned char hdr[16];
    int snap_len = orig_len;

    /* We were triggered to save the frame, now turn this off.
     * The SMTP state engine will have to revalidate the next
     * packet in order to make sure we should be saving it. */
    carn->do_filter = FALSE;

    /* Exit from this function (without saving content) if we
     * are not running in the appropriate mode.*/
    switch (carn->mode) {
    case mode_email_content:
    case mode_ip_content:
        break;
    default:
        return;
    }
    if (carn->tracefile == NULL)
        return; /*no filename*/

    /*Open the tracefile if need be*/
    if (carn->fp_trace == NULL) {
        struct stat s = {0};
        if (stat(carn->tracefile, &s) == 0)    {
            /*Ooops, it already exists. Maybe we crashed before? 
             * We should not put the header on the file if it 
             * already exists */
            carn->fp_trace = fopen(carn->tracefile, "a+b");
        } else {
            /*Create a new one.*/
            carn->fp_trace = fopen(carn->tracefile, "wb");
            if (carn->fp_trace) {
                /*create a file header*/
                static const char *foo = 
                        "\xD4\xC3\xB2\xA1" /*MAGIC*/
                        "\x02\x00\x04\x00" /*major/minor version*/
                        "\x00\x00\x00\x00" /*this timezone (GMT)*/
                        "\x00\x00\x00\x00" /*sig figs */
                        "\xDC\x05\x00\x00" /*snap length*/
                        "\x01\x00\x00\x00"; /*link type*/
                if (fwrite(foo, 1, 24, carn->fp_trace) != 24) {
                    int xxx = errno;
                    fclose(carn->fp_trace);
                    carn->fp_trace = NULL;
                    errno = xxx;
                }
            }
        }
        if (carn->fp_trace == NULL) {
            perror(carn->tracefile);
            return;
        }
    }

    /* Write the frame to the file */
    writeint(hdr, 0, ((int)timestamp));
    writeint(hdr, 4, usecs);        /*microseconds*/
    writeint(hdr, 8, snap_len);        /*snapped size of frame*/
    writeint(hdr, 12, orig_len);    /*original size of frame*/

    fwrite(hdr, 1, 16, carn->fp_trace);
    fwrite(buf, 1, snap_len, carn->fp_trace);
}


/**
 * Prints some text to the logfile.
 */
void 
logprint(Carnivore *carn, const char fmt[], ...)
{
    FILE *fp;
    struct stat s = {0};
    va_list marker;

    if (carn->logfile == NULL)
        return;
    if (stat(carn->logfile,&s) == 0)
        fp = fopen(carn->logfile, "a");
    else
        fp = fopen(carn->logfile, "w");
    if (fp == NULL) {
        perror(carn->logfile);
        return;
    }

    va_start(marker, fmt);
    vfprintf(fp, fmt, marker);
    va_end(marker);

    fclose(fp);
}

/**
 * For logging purposes, we frequently need to grab the current
 * time. This function formats the current GMT time in ISO
 * format. BUG: the time should really be retrieved from the 
 * packet, not the system time (in case we read from tracefiles
 * rather the live network).
 */
void 
formatNow(char tbuf[], int sizeof_tbuf)
{
    time_t now = time(0);
    struct tm *tmptr = gmtime(&now); /*must be GMT*/
    if (tmptr == NULL)
        strcpy(tbuf, "err");
    else
        strftime(tbuf, sizeof_tbuf, "%Y-%m-%d %H:%M:%S", tmptr);
}


/**
 * This function captures just the email addresses.
 */
void 
carnPenEmail(Carnivore *carn, const char sender[],
            const unsigned char rcpt[], int offset, int length)
{
    char tbuf[64];

    if (!MODE(carn, mode_email_addresses))
        return; /*not recording email addresses*/

    if (carn->logfile == NULL)
        return; /*no logfile specified by user*/

    if (sender == NULL)
        sender = "(nul)";
    
    /*format time: eg. 2000-08-24 08:23:59*/
    formatNow(tbuf, sizeof(tbuf));
    logprint(carn, "%s, %s, %.*s\n", tbuf, sender,
                                        length, rcpt+offset);
    printf("%s, %s, %.*s\n", tbuf, sender,
                                        length, rcpt+offset);
}




enum {
    parsing_envelope, parsing_message
};


/**
 * Tests to see if the TCP packet data starts with the specified
 * command.
 */
int 
SMTP_COMMAND(const unsigned char buf[], int offset, 
                                int max_offset, const char cmd[])
{
    int cmd_length = strlen(cmd);
    int line_length = max_offset-offset;

    if (line_length < cmd_length)
        return FALSE;
    if (memcasecmp(buf+offset, cmd, cmd_length) != 0)
        return FALSE;
    offset += cmd_length;

    /*TODO: test for some boundary conditions*/
    return TRUE;
}

/**
 * Tests to see if the email body contains a dot '.' on a blank
 * line by itself.
 */
int 
SMTP_IS_DOT(const unsigned char buf[], int offset, int max_offset)
{
    int i;
    char last_char = '\0';

    for (i=offset; i<max_offset; i++) {
        char c = buf[i];
        if (c == '.') {
            if (i+1 < max_offset 
                && (buf[i+1] == '\n' || buf[i+1] == '\r')
                && (last_char == '\n' || last_char == '\r')) {
                return TRUE;
            }
        }
        last_char = c;
    }
    return FALSE;
}


static const char *MAIL_FROM = "MAIL FROM:";
static const char *RCPT_TO = "RCPT TO:";


/**
 * Processes the email address in a RCPT TO: or MAIL FROM:
 */
void 
match_email(Carnivore *carn, const char *cmd,
        const unsigned char buf[], int offset, int max_offset, 
                                                TcpCnxn *cx)
{
    int length = -1;
    int address_matched = FALSE;


    /* See if this starts with RCPT TO: or MAIL FROM:, and then
     * skip beyond it. */
    if (!SMTP_COMMAND(buf, offset, max_offset, cmd))
        return;
    offset += strlen(cmd);

    /* Skip beyond leading whitespace and the initial '<' character
     * (if they exist). */
    while (offset < max_offset 
        && (isspace(buf[offset]) || buf[offset] == '<'))
        offset++;

    /* Figure out how long the email address is */
    for (length=0; offset+length<max_offset; length++) {
        char c = (char)buf[offset+length];
        if (c == '\r' || c == '\n' || c == '>')
            break;
    }
    if (length < 0)
        return;

    if (MODE(carn, mode_email_addresses) && cmd == MAIL_FROM ) {
        /* If we are doing a pen-register style capturing of email
         * addresses, then save off the SOURCE email address. */
        if (cx->sender)
            free(cx->sender);
        cx->sender = (char*)malloc(length+1);
        memcpy(cx->sender, buf+offset, length);
        cx->sender[length] = '\0';
    }

    /* See if the email addresses match */
    if (matchName((char*)buf+offset, length, 
                                        &carn->email_addresses)) {
        cx->do_filter = TRUE;
        address_matched = TRUE;
    }

    if (cmd == MAIL_FROM) {
        if (address_matched)
            cx->sender_matches = TRUE;
    } else if (cmd == RCPT_TO) {
        if (address_matched || cx->sender_matches)
            carnPenEmail(carn, cx->sender, buf, offset, length);
    }
}


/**
 * Read the number of remaining characters in the line.
 */
int 
readLine(const unsigned char buf[], int offset, int max_offset)
{
    int length = 0;
    while (offset + length < max_offset) {
        char c = buf[offset+length];
        length++;
        if (c == '\n')
            break;
    }
    return length;
}

/**
 * Examine the line from the packet in order to determine whether
 * it constitutes a legal RFC822 email header. We stop processing
 * data at the end of the headers.
 */
int 
isEmailHeader(const unsigned char buf[], int offset, int max_offset)
{
    int leading_space = 0;
    int saw_colon = 0;

    while (offset < max_offset && isspace(buf[offset])) {
        offset++; /*strip leading whitespace*/
        leading_space++;
    }

    if (offset >= max_offset)
        return FALSE; /*empty lines are not a header*/

    if (buf[offset] == '>')
        return FALSE;

    while (offset < max_offset) {
        if (buf[offset] == ':')
            saw_colon = TRUE;
        offset++;
    }

    if (saw_colon)
        return TRUE;
    if (leading_space)
        return TRUE;
    return FALSE;
}


/**
 * This function processes a single TCP segment sent by the client
 * to the SMTP server.
 */
int 
sniffSmtp(Carnivore *carn, TcpCnxn *rhs, int tcp_flags, 
              const unsigned char buf[], int offset, int max_offset)
{
    TcpCnxn *cx;
    int length;

    /* Lookup the TCP connection record to see if we are saving 
     * packets on the indicated TCP connection. */
    cx = cxLookup(carn, rhs, IGNORE_IF_NOT_FOUND);

    /* Process data within this TCP segment */
    length = max_offset - offset;
    if (length > 0) {

        if (cx == NULL) {
            /* Add a record for this connection whenever we see a
             * an address in an envelope. */
            if (SMTP_COMMAND(buf, offset, max_offset, "RCPT TO:"))
                cx = cxLookup(carn, rhs, ADD_IF_NOT_FOUND);
            if (SMTP_COMMAND(buf, offset, max_offset, "MAIL FROM:"))
                cx = cxLookup(carn, rhs, ADD_IF_NOT_FOUND);
        } 
        
        if (cx != NULL) {
            switch (cx->state) {
            case parsing_envelope:
                match_email(carn, MAIL_FROM,
                                    buf, offset, max_offset, cx);

                match_email(carn, RCPT_TO,
                                    buf, offset, max_offset, cx);

                if (SMTP_COMMAND(buf, offset, max_offset, "DATA")) {
                    if (cx->do_filter)
                        cx->state = parsing_message;
                    else
                        cx->do_remove = TRUE;
                }
                if (SMTP_COMMAND(buf, offset, max_offset, "QUIT"))
                    cx->do_remove = TRUE;
                if (SMTP_COMMAND(buf, offset, max_offset, "RSET"))
                    cx->do_remove = TRUE;
                if (SMTP_COMMAND(buf, offset, max_offset, "ERST"))
                    cx->do_remove = TRUE;
                break;
            case parsing_message:
                if (MODE(carn, mode_email_headers)) {
                    int i;
                    char tbuf[64];
                    formatNow(tbuf, sizeof(tbuf));
                    logprint(carn, "--- %08X->%08X %s ---\n", 
                        cx->client_ip, cx->server_ip, tbuf);

                    /*Parse just the headers from the first packet*/
                    for (i=offset; i<max_offset; i++) {
                        int len;
                        len = readLine(buf, offset, max_offset);
                        if (!isEmailHeader(buf, offset, offset+len))
                            break;
                        if (len > 8 && 
                          startsWith((char*)buf+offset, "Subject:"))
                            logprint(carn, "Subject: <removed>\n");
                        else {
                            /*Write line to log file*/
                            logprint(carn, "%.*s", len, buf+offset);
                        }
                        offset += len;
                    }
                    logprint(carn,"---EOM---\n");
                    cx->do_remove = TRUE;
                    cx->do_filter = FALSE;
                    carn->do_filter = FALSE;
                }
                if (SMTP_IS_DOT(buf, offset, max_offset))
                    cx->do_remove = TRUE;
                break;
            }
        }
    }

    if (cx) {
    
        if (cx->do_filter)
            carn->do_filter = TRUE;

        if (cx->filter_one_frame) {
            carn->do_filter = TRUE;
            cx->filter_one_frame = FALSE;
        }

        if (cx->do_remove 
            || (tcp_flags & TCP_RST) || (tcp_flags & TCP_FIN))
            cxRemove(carn, rhs);
    }

    return 0;
}


/**
 * RADIUS protocol information we parse out of a packet. In the
 * future versions of this software, we are going to need to
 * store these records over time; for now, we just parse the
 * protocol into this normalized structure.
 */
struct RadiusRecord
{
    int radius_client;
    int radius_server;
    int nas_ip;
    int nas_port;
    int direction;
    int code;
    int xid;
    int status;
    char *user_name;
    char *caller_id;
    char *called_phone;
    char *session_id;
    int ip_address;
    int session_duration;
};
typedef struct RadiusRecord RadiusRecord;

/** Frees the allocated information */
void 
radFree(RadiusRecord *rad)
{
    FREE(rad->user_name);
    FREE(rad->caller_id);
    FREE(rad->called_phone);
    FREE(rad->session_id);
}


/**
 * Process a single RADIUS command that we saw on the network.
 * For right now, we are primarily going to process radius
 * accounting packets, as these are the ones most likely to give
 * us solid information.
 */
void 
radProcess(Carnivore *carn, RadiusRecord *rad)
{
    enum {account_start=1, account_stop=2};
    if (rad->code == 4 || rad->code == 5) {
        /* ACCOUNTING packet: This packet contains an accounting
         * record. Accounting records will often contains IP address
         * assignments that normal authentication packets won't.*/
        if (rad->user_name && matchName(rad->user_name, 
                strlen(rad->user_name), &carn->radius_accounts)) {
            /* Found Alice! Therefore, we going add add Alice's
             * IP address to the list of IP addresses currently
             * being filtered. Conversely, if this is a stop
             * packet, then we will delete the IP address from
             * our list. */
            if (rad->status == account_start)
                add_integer(&carn->ip, rad->ip_address);
            else {
                /* Default: any unknown accounting message should
                 * trigger us to stop capturing data. If we make a
                 * mistake, we should err on the side of not
                 * collecting data. */
                del_integer(&carn->ip, rad->ip_address);
            }
            carn->do_filter = TRUE; /*capture this packet*/
        }

        /* Double-check: Look to see if the IP address belongs to
         * another person.*/
        else if (has_integer(&carn->ip, rad->ip_address, 0)) {
            /* The names did not match, yet we have seen some sort 
             * of packet dealing with the account that we are 
             * monitoring. This is bad -- it indicates that we might
             * have dropped a packet somewhere. Therefore, we are
             * going to immediately drop this packet.*/
            del_integer(&carn->ip, rad->ip_address);
            carn->do_filter = TRUE; /*capture this packet*/
        }
    }

}


/**
 * This function sniffs RADIUS packets off the network, then passes
 * the processed RADIUS information to another function that
 * deals with the content.
 */
int 
sniffRadius(Carnivore *carn, int ip_src, int ip_dst,
              const unsigned char buf[], int offset, int max_offset)
{
    RadiusRecord recx = {0};
    RadiusRecord *rad = &recx;
    const static int minimum_length = 20;
    int code;
    int xid;
    int radius_length;
    int i;

    if (carn->radius_accounts.length == 0)
        return 0; /*not scanning radius*/

    if (max_offset - offset <= minimum_length)
        return 0; /*corrupt*/

    /* Parse the RADIUS header info and verify */
    code = ex8(buf, offset+0);
    if (code < 1 || code > 5)
        return 0; /*unknown command/operationg*/
    xid = ex8(buf, offset+1);
    radius_length = ex16(buf, offset+2);
    if (offset + radius_length > max_offset)
        return 0; /*packet corrupt*/
    else if (offset + radius_length < minimum_length)
        return 0; /*packet corrupt*/
    else if (max_offset > offset + radius_length)
        max_offset = offset + radius_length; /*ignore padding*/

    /* Verify the attributes field */
    for (i=offset+minimum_length; i<max_offset-2; /*nul*/) {
        /*int type = buf[i];*/
        int len = buf[i+1];
        if (i+len > max_offset)
            return 0;
        i += len;
    }


    /* Grab the IP addresses of the client (the Network Access
     * Server like Livingston) and the RADIUS authentication
     * server. */
    if (code == 1 || code == 4) {
        rad->radius_client = ip_src;
        rad->radius_server = ip_dst;
    } else {
        rad->radius_client = ip_dst;
        rad->radius_server = ip_src;
    }
    rad->code = code;
    rad->xid = xid;

    /* Parse the attributes field */
    for (i=offset+minimum_length; i<max_offset-2; ) {
        int type = buf[i];
        int len = buf[i+1];
        int data_offset = i+2;
        if (i+len > max_offset)
            break;
        i += len;
        len -= 2;

        switch (type) {
        case 1: /*User-Name*/
            /*Lots of names appear to have a trailing nul that we
             *should strip from the end of the name.*/
            if (len > 1 && buf[data_offset+len-1] == '\0')
                len--;
            setString(&rad->user_name, buf, data_offset, len);
            break;
        case 2: /*User-Password*/
            break;
        case 4: /*NAS-IP-Address*/
            rad->nas_ip = ex32(buf,data_offset);
            break;
        case 5: /*NAS-Port*/
            rad->nas_port = ex32(buf,data_offset);
            break;
        case 8: /*Framed-IP-Address*/
            rad->ip_address = ex32(buf,data_offset);
            break;
        case 19: /*Callback-Number*/
        case 20: /*Callback-Id*/
            /*TODO: sounds like something we might want to record*/
            break;
        case 30: /*Called-Station-Id*/
            /*Find out the phone number of the NAS. This could be
             *important in cases where the evidence will later be
             *correlated with phone records.*/
            setString(&rad->called_phone, buf, data_offset, len);
            break;
        case 31: /*Calling-Station-Id*/
            /*True "trap-and-trace"! Assuming that caller-id is 
             *enabled, this will reveal the phone number of the 
             *person dialing in.*/
            setString(&rad->caller_id, buf, data_offset, len);
            break;
        case 40: /*Acct-Status-Type*/
            /*When scanning accounting packets, this is critical in
             *order to be able to detect when the service starts and
             *stops.*/
            rad->status = ex32(buf,data_offset);
            if (rad->status < 1 || 8 < rad->status)
                rad->status = 2; /*STOP if unknown*/
            break;
        case 44: /*Acct-Session-Id*/
            setString(&rad->session_id, buf, data_offset, len);
            break;
        case 46: /*Acct-Session-Time*/
            /*Could be interesting information to collect*/
            if (len == 4)
                rad->session_duration = ex32(buf,data_offset);
            break;
        }
    }

    /* The data was parsed from the RADIUS packet, now process that
     * data in order to trigger on the suspect.*/
    radProcess(carn, rad);
    radFree(rad);
    return 0;
}

struct iphdr {
    int offset;
    int proto;
    int src;
    int dst;
    int data_offset;
    int max_offset;
};
struct tcphdr {
    int offset;
    int src;
    int dst;
    int seqno;
    int ackno;
    int flags;
    int data_offset;
};


/**
 * This packet is called for each packet received from the wire
 * (or from test input). This function will parse the packet into
 * the consituent IP and TCP headers, then find which stream the
 * packet belongs to, then parse the remaining data according
 * to that stream.
 */
int 
sniffPacket(Carnivore *carn, const unsigned char buf[], 
                int max_offset, time_t timestamp, int usecs)
{
    struct iphdr ip;
    struct tcphdr tcp;
    TcpCnxn cn;
    
    /* Make sure that we have a frame long enough to hold the
     * Ethernet(14), IP(20), and UDP(8) or TCP(20) headers */
    if (max_offset < 14 + 20 + 20)
        return 1; /* packet fragment too small */

    if (ex16(buf,12) != 0x0800)
        return 1; /*not IP ethertype */

    /*IP*/
    ip.offset = 14; /*sizeof ethernet_header*/
    if (IP_VERSION(buf,ip.offset) != 4)
        return 1;
    ip.proto = IP_PROTOCOL(buf,ip.offset);
    ip.src = IP_SRC(buf,ip.offset);
    ip.dst = IP_DST(buf,ip.offset);
    ip.data_offset = ip.offset + IP_SIZEOF_HDR(buf,ip.offset);
    if (max_offset > IP_TOTALLENGTH(buf,ip.offset) + ip.offset)
        ip.max_offset = IP_TOTALLENGTH(buf,ip.offset) + ip.offset;
    else
        ip.max_offset = max_offset;

    /* If sniffing somebody's IP address, then sift for it */
    if (MODE(carn, mode_ip_content)
                        && has_integer(&carn->ip, ip.src, ip.dst))
        carn->do_filter = TRUE;

    if (ip.proto == 6) {
        /*TCP*/
        tcp.offset = ip.data_offset;
        tcp.dst = TCP_DST(buf,tcp.offset);
        tcp.src = TCP_SRC(buf,tcp.offset);
        tcp.flags = TCP_FLAGS(buf,tcp.offset);
        tcp.seqno = TCP_SEQNO(buf,tcp.offset);
        tcp.ackno = TCP_ACKNO(buf,tcp.offset);
        tcp.data_offset = tcp.offset 
                                + TCP_SIZEOF_HDR(buf,tcp.offset);

        if (MODE(carn, mode_server_access)) {
            /* We are watching for when the user attempts to access
             * servers of a specific type (HTTP, FTP, etc.). This
             * only tracks SYNs; though we could change the code
             * to track all packets. */
            if ((tcp.flags & TCP_SYN)
                && has_integer(&carn->ip, ip.src, ip.src)
                && has_integer(&carn->ip, tcp.dst, tcp.dst)) {
                char tbuf[64];
                formatNow(tbuf, sizeof(tbuf));
                logprint(carn, "%s, %d.%d.%d.%d, %d.%d.%d.%d, %d\n",
                    tbuf, P_IP_ADDR(ip.src), P_IP_ADDR(ip.dst),
                    tcp.dst);
            }
        }
        else
        switch (tcp.dst) {
        case 25:
            cn.server_ip = ip.dst;
            cn.client_ip = ip.src;
            cn.server_port = tcp.dst;
            cn.client_port = tcp.src;
            cn.server_seqno = tcp.ackno;
            cn.client_seqno = tcp.seqno;
            sniffSmtp(carn, &cn, tcp.flags | TCP_TO_SERVER, 
                            buf, tcp.data_offset, ip.max_offset);
            break;
        }
    } else if (ip.proto == 17) {
        /*UDP*/
        tcp.offset = ip.data_offset;
        tcp.dst = TCP_DST(buf,tcp.offset);
        tcp.src = TCP_SRC(buf,tcp.offset);
        tcp.data_offset = tcp.offset + 8;
        if (tcp.dst == 1812 || tcp.dst == 1813 
            || tcp.dst == 1645 || tcp.dst == 1646
            || tcp.src == 1812 || tcp.src == 1813 
            || tcp.src == 1645 || tcp.src == 1646) {
            /* This looks like a RADIUS packet, either using the
             * old port number or the new one. We are going to 
             * track both RADIUS authentication packets as well
             * as accounting packets (depending upon whwere we
             * are tapped into the network, we might see one,
             * the other, or both).*/
            sniffRadius(carn, ip.src, ip.dst, 
                            buf, tcp.data_offset, ip.max_offset);
        }

    }

    /* If one of the filters was successful, then save this packet
     * to the tracefile. This is only done*/
    if (carn->do_filter)
        carnSavePacket(carn, buf, max_offset, timestamp, usecs);

    return 0;
}


/**
 * A callback function that handles each packet as the 'libpcap'
 * subsystem receives it from the network.
 */
void pcapHandlePacket(unsigned char *carn, 
    const struct pcap_pkthdr *framehdr, const unsigned char *buf)
{
    int max_offset = framehdr->caplen;
    sniffPacket((Carnivore*)carn, buf, max_offset, 
        framehdr->ts.tv_sec, framehdr->ts.tv_usec);
}





/**
 * Sets the mode of operation according to the input parameter.
 */
void 
carnSetMode(Carnivore *carn, const char *value)
{
    if (startsWith(value, "email-head"))
        carn->mode = mode_email_headers;
    else if (startsWith(value, "email-addr"))
        carn->mode = mode_email_headers;
    else if (startsWith(value, "server-access"))
        carn->mode = mode_server_access;
    else if (startsWith(value, "email-content"))
        carn->mode = mode_email_content;
    else if (startsWith(value, "ip-content"))
        carn->mode = mode_ip_content;
    else
        carn->mode = -1;
}

/**
 * Parses the IP address. I use this rather than the sockets
 * inet_addr() for portability reasons. 
 */
int 
my_inet_addr(const char addr[])
{
    int num = 0;
    int offset=0;

    while (addr[offset] && !isalnum(addr[offset]))
        offset++;

    for (; addr[offset]; offset++) {
        char c = addr[offset];
        if (isdigit(c))
            num = (num&0xFFFFFF00) | (((num&0xFF)*10) + (c - '0'));
        else if (c == '.')
            num <<= 8;
        else
            break;
    }

    return num;
}


/**
 * Reads in the configuration from a a file such as "altivore.ini".
 */
void 
carnReadConfiguration(Carnivore *carn, const char filename[])
{
    FILE *fp;
    
    fp = fopen(filename, "r");
    if (fp == NULL)
        perror(filename);
    else {
        char line[1024];

        /* For all lines within the file */
        while (fgets(line, sizeof(line), fp)) {
            char *name = line;
            char *value;
            while (*name && isspace(*name))
                name++; /*strip leading whitespace*/
            if (*name == '\0' || ispunct(*name))
                continue;/*ignore blank lines and comments*/
            value = strchr(name, '=');
            if (value == NULL)
                continue; /*ignore when no equals sign*/
            else 
                value++; /*skip the equals itself*/
            while (*value && isspace(*value))
                value++; /*strip leading whitespace*/
            while (*value && isspace(value[strlen(value)-1]))
                value[strlen(value)-1] = '\0'; /*strip trailing WS*/

            if (startsWith(name, "mode"))
                carn->mode = parseMode(value);
            else if (startsWith(name, "email.address"))
                straAddElement(&carn->email_addresses, value);
            else if (startsWith(name, "radius.account"))
                straAddElement(&carn->radius_accounts, value);
            else if (startsWith(name, "ip.address"))
                add_integer(&carn->ip, my_inet_addr(value));
            else if (startsWith(name, "tracefile")) 
                setString(&carn->tracefile, value, 0, -1);
            else if (startsWith(name, "logfile"))
                setString(&carn->logfile, value, 0, -1);
            else if (startsWith(name, "testinput"))
                straAddElement(&carn->testinput, value);
            else if (startsWith(name, "interface"))
                straAddElement(&carn->interfaces, value);
            else if (startsWith(name, "server.port"))
                add_integer(&carn->ip, strtol(value,0,0));
            else
                fprintf(stderr, "bad param: %s\n", line);
        }
        fclose(fp);
    }
}


/**
 * Process a test input file.
 */
void 
processFile(Carnivore *carn, const char filename[])
{
    char errbuf[1024]; /*TODO: how long should this be?*/
    pcap_t *hPcap;
    
    /* Open the file */
    hPcap = pcap_open_offline(filename, errbuf);
    if (hPcap == NULL) {
        fprintf(stderr, "%s: %s\n", filename, errbuf);
        return; /*ignore this file and go onto next*/
    }

    /* Pump packets through it */
    for (;;) {
        int packets_read = pcap_dispatch(
                                hPcap, /*handle to PCAP*/
                                10,        /*next 10 packets*/
                                pcapHandlePacket, /*callback*/
                                (unsigned char*)carn /*canivore*/
                                );
        if (packets_read == 0)
            break;
    }

    /* Close the file and go onto the next one */
    pcap_close(hPcap);
}


/**
 * Sniff the wire for packets and process them using the libpcap
 * interface
 */
void 
processPackets(Carnivore *carn, const char devicename[])
{
    int traffic_seen = FALSE;
    int total_packets_processed = 0;
    pcap_t *hPcap;
    char errbuf[1024];

    hPcap = pcap_open_live(    (char*)devicename,
                            2000,    /*snap len*/
                            1,        /*promiscuous*/
                            10,        /*10-ms read timeout*/
                            errbuf
                            );
    if (hPcap == NULL) {
        fprintf(stderr, "%s: %s\n", devicename, errbuf);
        return;
    }

    /* Pump packets through it */
    for (;;) {
        int packets_read;
        
        packets_read = pcap_dispatch(
                                hPcap, /*handle to PCAP*/
                                10,        /*next 10 packets*/
                                pcapHandlePacket, /*callback*/
                                (unsigned char*)carn /*canivore*/
                                );
        total_packets_processed += packets_read;
        if (!traffic_seen && total_packets_processed > 0) {
            fprintf(stderr, "Traffic seen\n");
            traffic_seen = TRUE;
        }
    }

    /* Close the file and go onto the next one */
    pcap_close(hPcap);
}

/*----------------------------------------------------------------*/
int 
main(int argc, char *argv[])
{
    int i;
    Carnivore *carn;

    printf("--- ALTIVORE ---\n");
    printf("Copyright (c) 2000 by Network ICE Corporation\n");
    printf("Public disclosure of the source code does not\n");
    printf("constitute a license to use this software.\n");
    printf("Use \"altivore -?\" for help.\n");

    /* Create the carnivore subsystem */
    carn = (Carnivore*)malloc(sizeof(Carnivore));
    memset(carn, 0, sizeof(*carn));


    /* Read configuration info from "altivore.ini". */
    carnReadConfiguration(carn, "altivore.ini");

    /* Parse all the options from the command-line. Normally,
     * you wouldn't have any command-line options, you would
     * simply use the configuration file above. */
    for (i=1; i<argc; i++) {
        if (argv[i][0] != '-')
            straAddElement(&carn->email_addresses, argv[i]);
        else switch (argv[i][1]) {
            case 'h':
                add_integer(&carn->ip, my_inet_addr(argv[i]+2));
                break;
            case 'i':
                straAddElement(&carn->interfaces, argv[i]+2);
                break;
            case 'l':
                setString(&carn->logfile, argv[i]+2, 0, -1);
                break;
            case 'm':
                carn->mode = parseMode(argv[i]+2);
                break;
            case 'p':
                add_integer(&carn->port, strtol(argv[i]+2,0,0));
                break;
            case 'r':
                straAddElement(&carn->testinput, argv[i]+2);
                break;
            case 'w':
                setString(&carn->tracefile, argv[i]+2, 0, -1);
                break;
            case '?':
                printf("Options:\n"
                    "<email-address> address to filter for, e.g.:\n"
                    "  rob@altivore.com (exact match)\n"
                    "  *@altivore.com (partial match)\n"
                    "  * (match all emails)\n"
                    );
                printf("-h<ip-address>\n"
                    "\tIP of host to sniff\n");
                printf("-i<devicename>\n"
                    "\tNetwork interface to sniff on\n");
                printf("-l<logfile>\n"
                    "\tText-output logging\n");
                printf("-m<mode>\n"
                    "\tMode to run in, see docs\n");
                printf("-p<port>\n"
                    "\tServer port to filter on\n");
                printf("-r<tracefile>\n"
                    "\tTest input\n");
                printf("-w<tracefile>\n"
                    "\tEvidence tracefile to write packets to\n");
                return 1;
            default:
                fprintf(stderr, "Unknown parm: %s\n", argv[i]);
                break;
        }
    }

    /* Print the configuration for debugging purposes */
    printf("\tmode = %s\n", modeNames[carn->mode]);
    if (carn->tracefile)
        printf("\ttracefile = %s\n", carn->tracefile);
    if (carn->logfile)
        printf("\tlogfile = %s\n", carn->logfile);
    for (i=0; i<carn->ip.count; i++)
        printf("\tip = %d.%d.%d.%d\n", P_IP_ADDR(carn->ip.list[i]));
    for (i=0; i<carn->port.count; i++)
        printf("\tport = %d\n", carn->port.list);
    for (i=0; i<carn->email_addresses.length; i++)    
        printf("\temail.address = %s\n", carn->email_addresses.str[i]);
    for (i=0; i<carn->radius_accounts.length; i++)    
        printf("\tradius.accounts = %s\n", carn->radius_accounts.str[i]);
    for (i=0; i<carn->testinput.length; i++)    
        printf("\ttestinput = %s\n", carn->testinput.str[i]);
    for (i=0; i<carn->interfaces.length; i++)    
        printf("\tinterface = %s\n", carn->interfaces.str[i]);

    /* Testing only: user can specify tracefiles containing network
     * traffic for test purposes. */
    if (carn->testinput.length > 0) {
        int i;
        for (i=0; i<carn->testinput.length; i++)
            processFile(carn, carn->testinput.str[i]);
        return 0;
    }

    /* Open adapters and rea*/
    if (carn->interfaces.length > 0) {
        /*TODO: allow multiple adapters to be opened*/
        char *devicename = carn->interfaces.str[0];
        processPackets(carn, devicename);
    } else {
        char *devicename;
        char errbuf[1024];
        devicename = pcap_lookupdev(errbuf);
        if (devicename == NULL)
            fprintf(stderr, "%s\n", errbuf);
        else
            processPackets(carn, devicename);
    }

    return 0;
}

