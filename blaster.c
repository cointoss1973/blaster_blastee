/* uxBlaster.c - blasts another system with the given message size */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01c,06nov97,mm   added copyright.
01b,25sep97,mm   cast arg 4 of setsockopt and arg 2 of connect
01a,29jan94,ms   cleaned up for VxDemo.
*/

/*****************************************************************************
 * blaster - client program for UNIX host or VxWorks
 *
 * DESCRIPTION
 *
 *     This is a client program which connects to the server via TCP socket.
 *     It allows to configure the maximum  size  of  the  socket-level
 *     send buffer. It repeatedly sends a given size message to the given port
 *     at destination target. 
 *
 * EXAMPLE:
 *
 *     To run this blaster program from your UNIX host do the following: 
 *     % blaster  <target name>  7000  1000  16000 &
 *
 * EXAMPLE:
 *
 *     To run this blaster task from the VxWorks shell do as follows: 
 *     -> sp (blaster, "192.2.200.42", 7000, 1000, 16000)
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void blaster( const char *targetAddr, int port, int size, int blen )
{
    struct sockaddr_in	sin;
    int    sock;
    char   *buffer; 

    printf("blaster target:%s  port:%d writesize:%d SO_SNDBUF:%d\n", targetAddr, port, size, blen);

    bzero ((void *)&sin, sizeof (sin));

    sock = socket (AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
	perror ("cannot open socket");
        exit (1);
    }

#ifdef VXWORKS
    sin.sin_addr.s_addr = inet_addr(targetAddr);/* Support dot-decimal notation only */
#else
    struct hostent  *hp;
    struct hostent  *gethostbyname ();
    hp = gethostbyname (targetAddr);
    if (hp == 0 && (sin.sin_addr.s_addr = inet_addr (targetAddr)) == -1) {
	fprintf (stderr, "%s: unkown host\n", targetAddr);
	exit (2);
    }
    if (hp != 0) {
        bcopy (hp->h_addr, &sin.sin_addr, hp->h_length);
    }
#endif

    sin.sin_family 	= AF_INET;
    sin.sin_port 	= htons (port);


    if ((buffer = (char *) malloc (size)) == NULL) {
	perror ("cannot allocate buffer of size");
	exit (1);
    }

    if (setsockopt (sock, SOL_SOCKET, SO_SNDBUF, (char*) &blen, sizeof (blen)) < 0) {
	perror ("setsockopt SO_SNDBUF failed");
	exit (1);
    }

    if (connect (sock, (struct sockaddr *) &sin, sizeof (sin)) < 0) {
	perror ("connect");
    	printf ("connect failed: host %s port %d\n", inet_ntoa (sin.sin_addr),
		ntohs (sin.sin_port));
	exit (1);
    }
    
    for (;;) {
	int y;
	if ((y = write(sock, buffer, size)) < 0) {
	    perror ("blaster write error");
	    break;
	}
    }
    
    close (sock);
    
    free (buffer);
}

#ifndef VXWORKS
int main (int argc, char *argv[])
{
    int port, size, blen;

    if (argc < 5) {
	printf ("usage: %s targetname port size bufLen\n", argv [0]);
	exit (1);
    }

    port = atoi (argv [2]);
    size = atoi (argv [3]);
    blen = atoi (argv [4]);

    blaster(argv[1], port, size, blen);

    printf ("blaster exit.\n");

    return 0;
}
#endif

/* end of file */
