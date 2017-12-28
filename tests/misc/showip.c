#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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

    char *hostname = argv[1];

    struct addrinfo hints;
    struct addrinfo *servinfo, *p;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status;

    if ((status = getaddrinfo(hostname, NULL, &hints, &servinfo)) != 0) {
        fprintf(stderr, "!! getaddrinfo error: %s !!\n", gai_strerror(status));
        exit(2);
    }

    fprintf(stdout, "--- IP addresses for '%s' ---\n", hostname);

    char ipstr[INET6_ADDRSTRLEN];

    for (p = servinfo; p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;

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

        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));

        fprintf(stdout, "  %s: %s\n", ipver, ipstr);
    }

    freeaddrinfo(servinfo);

    return 0;
}