/* vxBlastee.c - reads given size message from the client*/

/* Copyright 1984-1997 Wind River Systems, Inc. */
 
/*
modification history
--------------------
01f,06nov97,mm   added copyright.
01e,10Oct97,mm   cast arg 4 of function setsockopt
01d,10Oct97,mm   added arguments to file logMsg
01c,29Sep97,mm   cast arg 1 of `bzero'
01b,16Sep97,mm   added include <stdio.h>, <stdlib.h>, <errno.h>, <string.h>,
		 <sysLib.h>, <logLib.h>, <sockLib.h> 
01a,29jan94,ms   cleaned up for VxDemo.
*/

#include <socket.h>
#include <sockLib.h>
#include <stdlib.h>
#include <sysLib.h>
#include <logLib.h>
#include <errno.h>
#include <string.h> 
#include <stdio.h> 
#include "wdLib.h"
#include "in.h"		
#include "ioLib.h"	

LOCAL int	blastNum;
LOCAL int 	wdIntvl = 60;

LOCAL WDOG_ID	blastWd = NULL;
int	blasteeStop;
LOCAL void blastRate ();
 

/*****************************************************************************
 * blastee - server task for VxWorks target
 *
 * DESCRIPTION
 *
 *     This is a server program which communicates with client through a
 *     TCP socket. It allows to configure the maximum size of socket-level 
 *     receive buffer. It repeatedly reads a given size message from the client
 *     and reports the number of bytes read every minute. It stops receiving 
 *     the message when the global variable blasteeStop is set to 1. 
 *
 * EXAMPLE:
 *
 *     To run this blastee task from the VxWorks shell do as follows: 
 *     -> sp (blastee, 7000, 1000, 16000)
 *
 *     To stop blastee task from the VxWorks shell do as follows: 
 *     -> blasteeStop = 1 
 *
 */


void blastee 
    (
    int   port,  /* the port number to read from */
    int   size,  /* size of the meesage */
    int   blen   /* maximum size of socket-level receive buffer */
    )
    {
    struct sockaddr_in	serverAddr; /* server's address */
    struct sockaddr_in  clientAddr; /* client's address */
    char   *buffer;
    int	   sock;
    int    snew;
    int    len;

    buffer = (char *) malloc (size);

    if (buffer == NULL)
	{
	perror ("cannot allocate buffer of size ");
	exit (1);
	}

    /* Create watchdog timer */
    if (blastWd == NULL && (blastWd = wdCreate ()) == NULL)
	{
	perror ("cannot create blast watchdog");
	free (buffer);
	exit (1);
	}

    /* Start watchdog after a minute*/
    wdStart (blastWd, sysClkRateGet () * wdIntvl, (FUNCPTR) blastRate, 0);

    /* Zero out the sock_addr structures.
     * This MUST be done before the socket calls.
     */
    bzero ((char *) &serverAddr, sizeof (serverAddr));
    bzero ((char *) &clientAddr, sizeof (clientAddr));

   /* Open the socket. Use ARPA Internet address format and stream sockets. */
    sock = socket (AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
	{
	perror ("cannot open socket");
	free (buffer);
	exit (1);
	}

    /* Set up our internet address, and bind it so the client can connect. */
    serverAddr.sin_family	= AF_INET;
    serverAddr.sin_port	        = htons (port);

    if (bind (sock, (struct sockaddr *)&serverAddr, sizeof (serverAddr)) < 0)
	{
    	perror ("bind error");
	free (buffer);
	exit (1);
	}

    /* Listen, for the client to connect to us. */
    if (listen (sock, 2) < 0)
	{
    	perror ("listen failed");
	free (buffer);
	exit (1);
	}
    
    len = sizeof (clientAddr);

    snew = accept (sock, (struct sockaddr *) &clientAddr, &len);
    if (snew == ERROR)
        {
        printf ("accept failed");
        close (sock);
        exit (1);
        }

    blastNum = 0;

    /* maximum size of socket-level receive buffer */
    if (setsockopt (snew, SOL_SOCKET, SO_RCVBUF,(char *) &blen, sizeof (blen)) < 0)
	{
	perror ("setsockopt SO_SNDBUF failed");
	free (buffer);
	exit (1);
	}

    blasteeStop = FALSE;


    for (;;)
	{
	int numRead;

	if (blasteeStop == TRUE)
	    break;

	if ((numRead = read (snew, buffer, size)) < 0)
	    {
	    perror ("blastee read error");
	    break;
	    }
	
	blastNum += numRead;
	}


    wdCancel (blastWd);

    close (sock);
    close (snew);
    
    free (buffer);
    printf ("blastee end.\n");
    }

/*****************************************************************************
 * blastRate - watchdog routine executed every one minute to report number of
 *             bytes read   
 *
 */

LOCAL void blastRate ()
    {
    if (blastNum > 0)
	{
	logMsg ("%d bytes/sec\n", blastNum / wdIntvl,0, 0, 0, 0, 0);
	blastNum = 0;
	}
    else
	{
	logMsg ("No bytes read in the last 60 seconds.\n",0, 0, 0, 0, 0, 0);
	}
    wdStart (blastWd, sysClkRateGet () * wdIntvl, (FUNCPTR) blastRate, 0);
    }
