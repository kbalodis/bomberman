#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// showip performs DNS lookup of the hostname and prints the IP adresses of the remote server

void 
print_usage(void) 
{
    fprintf(stderr, "showip {hostname}\n");
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        print_usage();
        exit(1);
    }

    char *hostname = argv[1];           // E.g. www.google.com.

    // *servinfo will hold the result of getaddrinfo call.
    // *p iterator for reading the linked list addrinfo.
    // *servinfo is the head node in the linked list, getarrdinfo() call initializes the linked list of addrinfo structs.
    struct addrinfo hints;
    struct addrinfo *servinfo, *p;
                                        
    memset(&hints, 0, sizeof(hints));   // Zero out memory to make sure that getaddrinfo() initializes servinfo correctly.

    hints.ai_family = AF_UNSPEC;        // IPv4 or IPv6, doesn't matter.
    hints.ai_socktype = SOCK_STREAM;    // Use the TCP protocol (UDP is SOCK_DGRAM).

    int status;

    if ((status = getaddrinfo(hostname, NULL, &hints, &servinfo)) != 0) {           // Second arg is (char*) port number.
        fprintf(stderr, "!! getaddrinfo error: %s !!\n", gai_strerror(status));
        exit(2);
    }

    fprintf(stdout, "--- IP addresses for '%s' ---\n", hostname);

    char ipstr[INET6_ADDRSTRLEN];                       // Long enough to hold IPv6 results.

    for (p = servinfo; p != NULL; p = p->ai_next) {     // Iterate over the resulting linked list.
        void *addr;
        char *ipver;

        // Cast required to set the pointer in the correct location.
        if (p->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in*)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        }
        else {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6*)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));    // Convert addr to a string.

        fprintf(stdout, "  %s: %s\n", ipver, ipstr);            // Print the ip.
    }

    freeaddrinfo(servinfo);

    return 0;
}