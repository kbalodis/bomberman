#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

void
sigchild_handler(int s) 
{
    int cur_error = errno;

    while (waitpid(-1, NULL, WNHANG) > 0)
    ;

    errno = cur_error;
}

void*
get_sockaddr_in(struct sockaddr *sa)
{
    return (sa->sa_family == AF_INET)
        ? &(((struct sockaddr_in*)sa)->sin_addr)
        : &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void 
print_usage(FILE *out) 
{
    fprintf(out, "server [port] [backlog] [-v]\n");
    fprintf(out, "    port - Port number to listen on.\n");
    fprintf(out, "    backlog - Number of clients that can connect.\n");
    fprintf(out, "    v (optional) - Verbose output.\n");
}

int main(int argc, char *argv[])
{
    if (argc < 3 || argc > 4) {
        print_usage(stderr);
        exit(1);
    }

    char *port = argv[1];
    char *backlog = argv[2];

    int v = 0;

    // Parse optional arguments
    char arg;
    while ((arg = getopt(argc, argv, "v")) != EOF)
    {
        switch (c)
        {
            case 'v':
                v = 1;
                break;
            case '?':
                fprintf(stderr, "Invalid command line option: %c!\n", optopt);
                print_usage(stderr);
                exit(1);
        }
    }

    struct addrinfo hints;
    struct addrinfo *servinfo, *p;
                                        
    memset(&hints, 0, sizeof(hints));   // Zero out memory to make sure that getaddrinfo() initializes servinfo correctly.

    hints.ai_family = AF_UNSPEC;        // IPv4 or IPv6, doesn't matter.
    hints.ai_socktype = SOCK_STREAM;    // Use the TCP protocol (UDP is SOCK_DGRAM).
    hints.ai_flags = AI_PASSIVE;        // Since we're the server, fill the IP adress of the host we're currently on.

    int status;

    if ((status = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        if (v)
            fprintf(stderr, "!! getaddrinfo error: %s !!\n", gai_strerror(status));

        exit(2);
    }

    int sockfd;
    int newfd;

    int yes = 1;

    for (p = servinfo; p != NULL; p = p->ai_next) {     // Iterate over the resulting linked list, try to bind to the first available one.
        if ((sockfd = socket(->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            if (v)
                fprintf(stdout, " * socket() failed\n");

            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            if (v)
                fprintf(stdout, " * setsockopt() failed\n");
            
            exit(2);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            if (v) {
                fprintf(stdout, " * bind() failed\n");
                fprintf(stdout, " * closing socket %d\n", sockfd);
            }
            
            close(sockfd);
            continue;
        }

        break;
    }

    freeaddrinfo(serinfo);

    if (!p) {
        fprintf(stderr, "!! server failed to bind !!\n");
        exit(2);
    }

    if (listen(sockfd, backlog) == -1) {
        fprintf(stderr, "!! listen() failed !!\n");
        exit(2);
    }

    struct sigaction sa;

    sa.sa_handler = sigchild_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        fprintf(stderr, "!! sigaction() failed !!\n");
        exit(2);
    }

    fprintf(stdout, "Waiting for connections...\n");

    struct sockaddr_storage client_addr;
    socklen_t sin_size;

    char ipstr[INET6_ADDRSTRLEN];

    while (1) {
        sin_size = sizeof(client_addr);
        newfd = accept(sockfd, (struct sockaddr*)&client_addr, &sin_size);

        if (newfd == -1) {
            if (v)
                fprintf(stdout, " * accept() failed\n");
            
            continue;
        }

        inet_ntop(
            client_addr.ss_family, 
            get_sockaddr_in((struct sockaddr*)&client_addr)),
            ipstr, 
            sizeof(ipstr));
        
        fprintf(stdout, "Received connection '%s'\n", ipstr);

        if (!fork()) {
            close(sockfd);

            if (send(newfd, "Hello world!", 13, 0) == -1) {
                fprintf(stderr, "!! send() failed !!\n");
            }

            close(newfd);
            exit(0);
        }

        close(newfd);
    }

    return 0;
}