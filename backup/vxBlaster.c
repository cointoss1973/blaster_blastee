/* vxBlaster.c - blasts another system with the given message size */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01f,06nov97,mm   added copyright.
01e,10oct97,mm   deleted line int ix = 0
01d,10oct97,mm   cast arg 4 of setsockopt and cast arg 2 of connect
01c,10oct97,mm   added include <sockLib.h> and "arpa/inet.h"
01b,29sep97,mm   cast arg 1 of 'bzero'
01a,29jan94,ms   cleaned up for VxDemo.
*/

/*****************************************************************************
 * blaster - client task for VxWorks target
 *
 * DESCRIPTION
 *
 *     This is a client task which connects to the server via TCP socket.
 *     It allows to configure the maximum  size  of  the  socket-level
 *     send buffer. It repeatedly sends a given size message to the given port
 *     at destination target. It stops sending the message when the global
 *     variable blasterStop is set to 1. 
 *
 * EXAMPLE:
 *
 *     To run this blaster task from the VxWorks shell do as follows: 
 *     -> sp (blaster, "192.2.200.42", 7000, 1000, 16000)
 *
 *     To stop blaster task from the VxWorks shell do as follows: 
 *     -> blasterStop = 1 
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "vxWorks.h"
#include "socket.h"
#include <sockLib.h>
#include "arpa/inet.h"
#include "in.h"

int blasterStop;


void blaster 
    (
    char    *targetAddr, /* targetAddress is the IP address of destination 
                            target that needs to be blasted */
    int	     port,       /* port number to send to */
    int      size,       /* size of the message to be sent */
    int      blen        /* maximum size of socket-level send buffer */
    )
    {

    struct sockaddr_in	sin;
    int    sock;
    char   *buffer;

    bzero ((char *) &sin, sizeof (sin));

    sock = socket (AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
        {
	perror ("cannot open socket");
        exit (1);
        }

    sin.sin_family 	= AF_INET;
    sin.sin_addr.s_addr	= inet_addr (targetAddr);
    sin.sin_port	= htons (port);


    if ((buffer = (char *) malloc (size)) == NULL)
	{
	perror ("cannot allocate buffer of size ");
	exit (1);
	}

    if (setsockopt (sock, SOL_SOCKET, SO_SNDBUF, (char *)&blen, sizeof (blen)) < 0)
	{
	perror ("setsockopt SO_SNDBUF failed");
	free (buffer);
	exit (1);
	}

    if (connect (sock, (struct sockaddr  *)&sin, sizeof (sin)) < 0)
	{
        perror ("connect failed");
    	printf ("host %s port %d\n", inet_ntoa (sin.sin_addr),
		ntohs (sin.sin_port));

	free (buffer);
	exit (1);
	}

    blasterStop = FALSE;
     
    for (;;)
	{
	int nBytes;

        if (blasterStop == TRUE)
            break;

	if ((nBytes = write(sock, buffer, size)) < 0)
	    {
	    perror ("blaster write error");
	    break;
	    }
	}
    
    close (sock);
    free (buffer);
    printf ("blaster exit.\n");
    }
