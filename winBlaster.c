/* winBlaster.c - blasts another system with the given message size */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
01a,29jan94,ms   cleaned up for VxDemo.
*/

/*****************************************************************************
 * blaster - client program for MS WINDOWS host
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
 *     To run this blaster program from your WINDOWS host do the following at
 *     a DOS prompt: 
 *     C:\> blaster  <target name>  7000  1000  16000
 *
 *
 */

#include <sys/types.h>
#include <winsock.h>				/* <sys/socket.h> */
#include <stdio.h>
#include <string.h>
#include <io.h>

void main 
    (
    int		argc,
    char **	argv
    )
    {
    struct sockaddr_in	sin;
    int    sock;
    int    ix = 0;
    char   *buffer; 
    int	   blen; 		/* maximum size of socket-level send buffer */
    int    size; 		/* size of the message to be sent */
    struct hostent  *hp;

    WORD wVersionRequested; 
    WSADATA wsaData; 
    int err; 
	
    /* initialize winsock.dll */
    wVersionRequested = MAKEWORD(1, 1); 
    err = WSAStartup(wVersionRequested, &wsaData); 
 
    if (err != 0) 
        /* Tell the user that we couldn't find a useable */ 
        /* winsock.dll.     */ 

        return; 
 
    /* Confirm that the Windows Sockets DLL supports 1.1.*/ 
    /* Note that if the DLL supports versions greater */ 
    /* than 1.1 in addition to 1.1, it will still return */ 
    /* 1.1 in wVersion since that is the version we */ 
    /* requested. */ 
 
    if ( LOBYTE( wsaData.wVersion ) != 1 || 
		    HIBYTE( wsaData.wVersion ) != 1 ) 
        { 
	/* Tell the user that we couldn't find a useable */ 
	/* winsock.dll. */ 

        perror ("Unable to initialize WinSock Version 1.1\n");
        WSACleanup(); 
        return; 
	} 

    /* The Windows Sockets DLL is acceptable. Proceed. */ 

    
    if (argc < 5)
	{
	printf ("usage: %s targetname port size bufLen\n", argv [0]);
	exit (1);
	}

    memset(&sin, 0, sizeof(sin));	

    /* Create a TCP socket */

    sock = socket (PF_INET, SOCK_STREAM, 0);

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

    /* Fill in the hostent structure */

    if (hp != 0)
        memcpy (&sin.sin_addr, hp->h_addr, hp->h_length);

    sin.sin_family 	= PF_INET;
    sin.sin_port 	= htons ((short) atoi (argv [2]));
    size		= atoi (argv [3]);
    blen = atoi (argv [4]);

    if ((buffer = (char *) malloc (size)) == NULL)
	{
	perror ("cannot allocate buffer of size ");
	exit (1);
	}

    /* Set the size of the socket send and receive buffers to blen */
 
    if (setsockopt (sock, SOL_SOCKET, SO_SNDBUF,
		    (char *) &blen, sizeof (blen)) < 0)
	{
	perror ("setsockopt SO_SNDBUF failed");
	exit (1);
	}

    if (setsockopt (sock, SOL_SOCKET, SO_RCVBUF,
                    (char*) &blen, sizeof (blen)) < 0)
        {
        perror ("setsockopt SO_RCVBUF failed");
        exit (1);
        }

    /* Connect to the server */

    if (connect (sock, (SOCKADDR *) &sin, sizeof (sin)) < 0)
	{
	perror ("connect");
    	printf ("connect failed: host %s port %d\n", inet_ntoa (sin.sin_addr),
		ntohs (sin.sin_port));
	exit (1);
	}
   
    /* Send a data buffer of length size to the server repeatedly */ 
    for (;;)
	{
	int y;
	if ((y = send(sock, buffer, size, 0)) < 0)
	    {
	    perror ("blaster write error");
	    break;
	    }
	}
    
    close (sock);
    
    free (buffer);
    printf ("blaster exit.\n");
    }

