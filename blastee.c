/* uxBlastee.c - reads given size message from the client*/

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01c,06nov97,mm   added copyright.
01b,26sep97,mm   cast arg 2 of bind , arg 2 of accept and arg 4 of setsockopt
01a,29jan94,ms   cleaned up and modified for VxDemo.
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/times.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

long	blastNum;
int 	wdIntvl = 10;

#define	FALSE	0
#define TRUE	1


static void blastRecv(int snew, int size);
static void blastHandler(int intvl);
static void blastAlarm(int intvl);
static void blastRate();

/*****************************************************************************
 * blastee - server process on UNIX host or VxWorks
 *
 * DESCRIPTION
 *
 *     This is a server program which communicates with client through a
 *     TCP socket. It allows to configure the maximum size of socket-level
 *     receive buffer. It repeatedly reads a given size message from the client
 *     and reports the number of bytes read every minute.
 *
 * EXAMPLE:
 *
 *     To run this blastee task from UNIX host do the following:
 *     % blastee   7000  1000  16000 &
 *
 *     To run this blastee task from the VxWorks shell do as follows:
 *     -> sp (blastee, 7000, 1000, 16000)
 *
 */
int blastee(int port, int size, int blen)
{
    struct sockaddr_in	serverAddr; /* server's address */
    struct sockaddr_in  clientAddr; /* client's address */

    int	   sock;
    int    snew;
#ifdef VXWORKS
    int    len;
#else
    socklen_t len;
#endif
    int    on = 1;

    printf("blastee  port:%d recvsize:%d SO_RCVBUF:%d cycle:%d\n",port, size, blen, wdIntvl);

    blastHandler(wdIntvl);

    /* Zero out the sock_addr structures.
     * This MUST be done before the socket calls.
     */
    bzero ((void *)&serverAddr, sizeof (serverAddr));
    bzero ((void *)&clientAddr, sizeof (clientAddr));

    /* Open the socket. Use ARPA Internet address format and stream sockets. */
    sock = socket (AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
	perror ("cannot open socket");
	exit (1);
    }
    /* maximum size of socket-level receive buffer */
    if (setsockopt (sock, SOL_SOCKET, SO_RCVBUF, (char *)&blen, sizeof (blen)) == -1) {
	perror ("setsockopt SO_RCVBUF failed");
	exit (1);
    }
    if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) == -1){
	perror ("setsockopt SO_REUSEADDR failed");
	exit (1);
    }
    

   /* Set up our internet address, and bind it so the client can connect. */
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port	= htons (port);

    if (bind (sock, (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
    	perror ("bind error");
	exit (1);
    }

    /* Listen, for the client to connect to us. */
    if (listen (sock, 2) < 0) {
    	perror ("listen failed");
	exit (1);
    }

    len = sizeof (clientAddr);

    /**/
    for(;;) {
	blastNum = 0;
	printf("accept...\n");
	snew = accept (sock, (struct sockaddr *) &clientAddr, &len);
	if (snew == -1) {
	    perror ("accept failed");
	    close (sock);
	    exit (1);
	}
	blastRecv(snew, size);
    }


    close (sock);
    close (snew);
    
    printf ("blastee end.\n");
    return 0;
}


#ifndef VXWORKS
int main (int argc, char *argv[])
{
    int port, size, blen;

    if (argc < 4) {
	printf ("usage: %s port size bufLen\n", argv [0]);
	exit (1);
    }

    port = atoi (argv [1]);
    size = atoi (argv [2]);
    blen = atoi (argv [3]); /* SO_RCVBUF */

    return blastee(port, size, blen);
}
#endif /* VXWORKS */


static void blastRecv(int snew, int size)
{
    char   *buffer = (char *) malloc(size);

    if (buffer == NULL) {
	perror ("cannot allocate buffer");
	exit (1);
    }


    for (;;) {
	int numRead;

	if ((numRead = read (snew, buffer, size)) < 0) {
	    perror ("blastee read error");
	    break;
	}
	if (numRead == 0 ){
	    break; /* disconnect by peer */
	}

	blastNum += numRead;
    }
    free (buffer);
    close (snew);
}


#ifdef VXWORKS
#include <wdLib.h>
static WDOG_ID blastWid;

static void blastHandler(int intvl)
{
    if (blastWid == NULL && (blastWid = wdCreate ()) == NULL) {
	perror ("cannot create blast watchdog");
	exit (1);
    }

    blastAlarm(wdIntvl);    /* Start watchdog after a minute */
}
#else
static void blastHandler(int intvl)
{
    /* Associate SIGALARM signal with blastRate signal handler */
    signal (SIGALRM, blastRate);
    blastAlarm(intvl); /* Start signal handler after a minute */
}
#endif



static void blastAlarm(int intvl)
{
#ifdef VXWORKS
    wdStart (blastWid, sysClkRateGet() * wdIntvl, (FUNCPTR)blastRate, 0);
#else
    alarm(intvl);
#endif
}


/*****************************************************************************
 * blastRate - signal handler routine executed every one minute which reports
 *             number of  bytes read
 *
 */
#ifdef VXWORKS
#include <logLib.h>
#endif

static void blastRate (void)
{
#ifdef VXWORKS
    if (blastNum > 0) {
	/* ISRs must not call routines that use a floating-point coprocessor. */
	logMsg ("%d bytes/sec\n", blastNum / wdIntvl,0, 0, 0, 0, 0);
	blastNum = 0;
    } else {
	logMsg ("No bytes read in the last %d seconds.\n",wdIntvl, 0, 0, 0, 0, 0);
    }
#else
    double rateMB = (blastNum/(1024*1024)) / (double)wdIntvl;
    if (blastNum > 0) {
	printf ("%.1f MB/sec (total %ld)\n", rateMB, blastNum);
	blastNum = 0;
    } else {
	printf ("No bytes read in the last %d seconds.\n",wdIntvl);
    }
#endif
    blastAlarm (wdIntvl);
}

/* end of file */
