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
static void blastRate();

/*****************************************************************************
 * blastee - server process on UNIX host
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
 *
 */


int main (int argc, char *argv[])
{
    struct sockaddr_in	serverAddr; /* server's address */
    struct sockaddr_in  clientAddr; /* client's address */

    int	   sock;
    int    snew;
    socklen_t len;
    int	   size;
    int	   blen;
    int    on = 1;

    if (argc < 4) {
	printf ("usage: %s port size bufLen\n", argv [0]);
	exit (1);
    }

    size = atoi (argv [2]);
    blen = atoi (argv [3]); /* SO_RCVBUF */

    printf("%s recvsize:%d SO_RCVBUF:%d cycle:%d\n",argv[0], size, blen, wdIntvl);

    /* Associate SIGALARM signal with blastRate signal handler */
    signal (SIGALRM, blastRate);
    alarm (wdIntvl); /* Start signal handler after a minute */

    /* Zero out the sock_addr structures.
     * This MUST be done before the socket calls.
     */
    bzero (&serverAddr, sizeof (serverAddr));
    bzero (&clientAddr, sizeof (clientAddr));

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
    serverAddr.sin_port	= htons (atoi (argv [1]));

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


/*****************************************************************************
 * blastRate - signal handler routine executed every one minute which reports
 *             number of  bytes read
 *
 */

static void blastRate (void)
{
    if (blastNum > 0) {
	/*printf ("%ld bytes/sec (total %d)\n", blastNum / wdIntvl, blastNum);*/
	printf ("%.1f MB/sec (total %ld)\n", (blastNum/(1024*1024)) / (double)wdIntvl, blastNum);
	blastNum = 0;
    } else {
	printf ("No bytes read in the last %d seconds.\n",wdIntvl);
    }
    alarm (wdIntvl);
}

/* end of file */
