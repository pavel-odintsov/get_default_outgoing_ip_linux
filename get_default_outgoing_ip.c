#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Retrieves default outgoing IPv4 address using Linux sockets trick
int get_default_outgoing_ipv4_address(uint32_t* ipv4_address) {
    // Create UDP socket as only it works well for our case
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (client_socket < 0) {
        warn("cannot create IPv4 socket");
        return 1;
    }

    // We will "attempt" to connect to Google DNS
    // Do not worry. There are no connection expected to happen
    unsigned short remote_port_dns = 53;

    // We use IP address from special block TEST-NET-3 which is reserved only for documentation use
    // https://datatracker.ietf.org/doc/html/rfc5737
    char* remote_host = "203.0.113.1";

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(remote_port_dns);

    int pton_result = inet_pton(AF_INET, remote_host, &serv_addr.sin_addr);

    if (pton_result <= 0) {
        warn("IPv4 inet_pton failed for %s", remote_host);
        close(client_socket);
        return 1;
    }

    // Connect. As we deal with UDP there are no actual connection will happen in this moment
    // Linux kernel will just populate peer information and will populate outgoing IP address which is useful for us

    int connect_result = connect(client_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    if (connect_result != 0) {
        warn("IPv4 cannot connect");
        close(client_socket);
        return 1;
    }

    // Retrieve default outgoing IP address
    struct sockaddr_in peer_socket;
    memset(&peer_socket, 0, sizeof(peer_socket));

    socklen_t address_length = sizeof(peer_socket);
    int getsockname_result = getsockname(client_socket, (struct sockaddr*) &peer_socket, &address_length);

    if (getsockname_result != 0) {
        warn("IPv4 getsockname failed");
        // Some error happened
        close(client_socket);
        return 1;
    }

    // Retrieve IPv4 address
    memcpy(ipv4_address, &peer_socket.sin_addr, sizeof(uint32_t));

    // Close socket as we do not need it anymore
    close(client_socket);

    return 0;
}


// Retrieves default outgoing IPv6 address using Linux sockets trick
int get_default_outgoing_ipv6_address(struct in6_addr* ipv6_address) {
    // Create UDP socket as only it works well for our case
    int client_socket = socket(AF_INET6, SOCK_DGRAM, 0);

    if (client_socket < 0) {
        warn("cannot create IPv6 socket");
        return 1;
    }

    // We will "attempt" to connect to Google DNS
    // Do not worry. There are no connection expected to happen
    unsigned short remote_port_dns = 53;

    // We use IPv6 address reserved for documentation
    // https://datatracker.ietf.org/doc/html/rfc3849
    char* remote_host = "2001:db8::1";

    struct sockaddr_in6 serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_port   = htons(remote_port_dns);

    int pton_result = inet_pton(AF_INET6, remote_host, &serv_addr.sin6_addr);

    if (pton_result <= 0) {
        warn("IPv6 inet_pton failed for %s", remote_host);

        close(client_socket);
        return 1;
    }

    // Connect. As we deal with UDP there are no actual connection will happen in this moment
    // Linux kernel will just populate peer information and will populate outgoing IP address which is useful for us

    int connect_result = connect(client_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    if (connect_result != 0) {
        warn("IPv6 connect failed");
        close(client_socket);
        return 1;
    }

    // Retrieve default outgoing IP address
    struct sockaddr_in6 peer_socket;
    memset(&peer_socket, 0, sizeof(peer_socket));

    socklen_t address_length = sizeof(peer_socket);
    int getsockname_result = getsockname(client_socket, (struct sockaddr*) &peer_socket, &address_length);

    if (getsockname_result != 0) {
        warn("IPv6 getsockname failed");

        // Some error happened
        close(client_socket);
        return 1;
    }

    // Retrieve IPv6 address
    memcpy(ipv6_address, &peer_socket.sin6_addr, sizeof(struct in6_addr));

    // Close socket as we do not need it anymore
    close(client_socket);

    return 0;
}

int main(int argc, char **argv) {
    uint32_t ipv4_address = 0;
    int retval = EXIT_SUCCESS;

    int get_ipv4_result = get_default_outgoing_ipv4_address(&ipv4_address);

    if (get_ipv4_result == 0) {
        char ipv4_address_as_string[256];

        if (inet_ntop(AF_INET, &ipv4_address, ipv4_address_as_string, 256) == NULL) {
            printf("Successfully retrieved default outgoing IPv4 address but failed to print it\n");
            retval = EXIT_FAILURE;
        } else {
            printf("Successfully retrieved default outgoing IPv4 address: %s\n", ipv4_address_as_string);
        }
    } else {
        warnx("cannot retrieve outgoing IPv4 address");
        retval = EXIT_FAILURE;
    }

    struct in6_addr ipv6_address;
    memset(&ipv6_address, 0, sizeof(struct in6_addr));

    int get_ipv6_result = get_default_outgoing_ipv6_address(&ipv6_address);

    if (get_ipv6_result == 0) {
        char ipv6_address_as_string[256];

        if (inet_ntop(AF_INET6, &ipv6_address, ipv6_address_as_string, 256) == NULL) {
            printf("Successfully retrieved default outgoing IPv6 address but failed to print it\n");
            retval = EXIT_FAILURE;
        } else {
            printf("Successfully retrieved default outgoing IPv6 address: %s\n", ipv6_address_as_string);
        }
    } else {
        warnx("cannot retrieve outgoing IPv6 address");
        retval = EXIT_FAILURE;
    }
    return retval;
}
