#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <time.h>
#include <stdio.h>
#define SECONDS_IN_70_YEARS 2208988800

typedef struct ntp_packet
{
    unsigned char leap_version_mode;
    unsigned char stratum;
    unsigned char poll;
    unsigned char precision;
    unsigned long root_delay;
    unsigned long root_dispersion;
    char rfid[4];
    unsigned long long ref_timestamp;
    unsigned long long original_timestamp;
    unsigned long long receive_timestamp;
    unsigned long long transmit_timestamp;
} ntp_packet;



int main(int argc, char **argv)
{

#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(0x0202, &wsa_data))
    {
        printf("unable to initialize winsock2 \n");
        return -1;
    }
#endif

    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0)
    {
        printf("unable to initialize the UDP socket \n");
        return -1;
    }

    printf("socket %d created \n", s);

    struct sockaddr_in sin;
    inet_pton(AF_INET, "216.239.35.12", &sin.sin_addr); 
    sin.sin_family = AF_INET;
    sin.sin_port = htons(123); 

    unsigned char leap = 0;
    unsigned char version = 4;
    unsigned char mode = 3;

    time_t now = time(NULL);

    ntp_packet packet = {leap << 6 | version << 3 | mode, 0, 0, 0, 0, 0, {0, 0, 0, 0}, 0, now, 0, 0};

    int sent_bytes = sendto(s, (char *)&packet, sizeof(packet), 0, (struct sockaddr *)&sin, sizeof(sin));
    printf("sent %d bytes via UDP \n", sent_bytes);

    for (;;)
    {
        struct sockaddr_in sender_in;
        int sender_in_size = sizeof(sender_in);
        int len = recvfrom(s, (char *)&packet, sizeof(packet), 0, (struct sockaddr *)&sender_in, &sender_in_size);
        if (len > 0)
        {
            char addr_as_string[64];
            inet_ntop(AF_INET, &sender_in.sin_addr, addr_as_string, 64);
            printf("received %d bytes from %s:%d\n", len, addr_as_string, ntohs(sender_in.sin_port));

            // converting from network byte order (which is big endian) to host byte order (which, usually, is little endian)
            packet.root_delay = ntohl(packet.root_delay);
            packet.root_dispersion = ntohl(packet.root_dispersion);



            time_t actual_time = ntohl(packet.receive_timestamp);
            packet.ref_timestamp = ntohll(packet.ref_timestamp);
            packet.original_timestamp = ntohll(packet.original_timestamp);
            packet.receive_timestamp = ntohll(packet.receive_timestamp);
            packet.transmit_timestamp = ntohll(packet.transmit_timestamp);

            printf("Leap second: %d \nVersion: %d \nMode: %d\n", packet.leap_version_mode >> 6, (packet.leap_version_mode >> 3) & 0b111, packet.leap_version_mode & 0b111);
            printf("Stratum: %d \nPoll: %d \nPrecision: %d\n", packet.stratum, packet.poll, packet.precision);
            printf("Root delay: %lu\n", packet.root_delay);
            printf("Root dispersion: %lu\n", packet.root_dispersion);
            printf("Ref ID: %c%c%c%c\n", packet.rfid[0], packet.rfid[1], packet.rfid[2], packet.rfid[3]);
            // Here I'm cutting the fractionary part, to show only the seconds passed from 1/01/1900
            printf("Ref timestamp: %llu\n",      packet.ref_timestamp);
            printf("Original timestamp: %llu\n", packet.original_timestamp);
            printf("Received timestamp: %llu\n", packet.receive_timestamp);
            printf("Transmit timestamp: %llu\n", packet.transmit_timestamp);

            actual_time -= SECONDS_IN_70_YEARS;

            char outstr[4096];
            struct tm* tmp;
            tmp = localtime(&actual_time);
            strftime(outstr, sizeof(outstr), "%A, %d %B %Y %T", tmp);
            printf("Result string is \"%s\"\n", outstr);

            break;
        }
    }

    return 0;
}

