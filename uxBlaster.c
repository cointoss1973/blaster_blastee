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
 * blaster - client program for UNIX host
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
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>


main (argc, argv)
    int		argc;
    char	*argv [];
    {
    struct sockaddr_in	sin;
    int    sock;
    int    ix = 0;
    char   *buffer; 
    int	   blen; /* maximum size of socket-level send buffer */
    int    size; /* size of the message to be sent */
    struct hostent  *hp;
    struct hostent  *gethostbyname ();

    if (argc < 5)
	{
	printf ("usage: %s targetname port size bufLen\n", argv [0]);
	exit (1);
	}

    bzero (&sin, sizeof (sin));

    sock = socket (AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
        {
	perror ("cannot open socket");
        exit (1);
        }

    hp = gethostbyname (argv[1]);
    if (hp == 0 && (sin.sin_addr.s_addr = inet_addr (argv [1])) == -1)
	{
	fprintf (stderr, "%s: unkown host\n", argv [1]);
	exit (2);
	}

    if (hp != 0)
        bcopy (hp->h_addr, &sin.sin_addr, hp->h_length);

    sin.sin_family 	= AF_INET;
    sin.sin_port 	= htons (atoi (argv [2]));
    size		= atoi (argv [3]);

    blen = atoi (argv [4]);


    if ((buffer = (char *) malloc (size)) == NULL)
	{
	perror ("cannot allocate buffer of size ");
	exit (1);
	}

    if (setsockopt (sock, SOL_SOCKET, SO_RCVBUF, (char*) &blen, sizeof (blen)) < 0)
	{
	perror ("setsockopt SO_RCVBUF failed");
	exit (1);
	}

    if (connect (sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
	{
	perror ("connect");
    	printf ("connect failed: host %s port %d\n", inet_ntoa (sin.sin_addr),
		ntohs (sin.sin_port));

	exit (1);
	}
    
    for (;;)
	{
	int y;
	if ((y = write(sock, buffer, size)) < 0)
	    {
	    perror ("blaster write error");
	    break;
	    }
	}
    
    close (sock);
    
    free (buffer);
    printf ("blaster exit.\n");
    }

