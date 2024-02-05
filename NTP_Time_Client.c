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

typedef struct ntpPacket
{
    unsigned char leapVersionMode;
    unsigned char stratum;
    unsigned char poll;
    unsigned char precision;
    unsigned long rootDelay;
    unsigned long rootDispersion;
    char rfid[4];
    unsigned long long refTimestamp;
    unsigned long long originalTimestamp;
    unsigned long long receiveTimestamp;
    unsigned long long transmitTimestamp;
} ntpPacket;



int main(int argc, char **argv)
{

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(0x0202, &wsaData))
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

    ntpPacket packet = {leap << 6 | version << 3 | mode, 0, 0, 0, 0, 0, {0, 0, 0, 0}, 0, now, 0, 0};

    int sentBytes = sendto(s, (char *)&packet, sizeof(packet), 0, (struct sockaddr *)&sin, sizeof(sin));
    printf("sent %d bytes via UDP \n", sentBytes);

    for (;;)
    {
        struct sockaddr_in sender_in;
        int senderInSize = sizeof(sender_in);
        int len = recvfrom(s, (char *)&packet, sizeof(packet), 0, (struct sockaddr *)&sender_in, &senderInSize);
        if (len > 0)
        {
            char addrAsString[64];
            inet_ntop(AF_INET, &sender_in.sin_addr, addrAsString, 64);
            printf("received %d bytes from %s:%d\n", len, addrAsString, ntohs(sender_in.sin_port));

            // converting from network byte order (which is big endian) to host byte order (which, usually, is little endian)
            packet.rootDelay = ntohl(packet.rootDelay);
            packet.rootDispersion = ntohl(packet.rootDispersion);



            time_t actualTime = ntohl(packet.receiveTimestamp);
            packet.refTimestamp = ntohll(packet.refTimestamp);
            packet.originalTimestamp = ntohll(packet.originalTimestamp);
            packet.receiveTimestamp = ntohll(packet.receiveTimestamp);
            packet.transmitTimestamp = ntohll(packet.transmitTimestamp);

            printf("Leap second: %d \nVersion: %d \nMode: %d\n", packet.leapVersionMode >> 6, (packet.leapVersionMode >> 3) & 0b111, packet.leapVersionMode & 0b111);
            printf("Stratum: %d \nPoll: %d \nPrecision: %d\n", packet.stratum, packet.poll, packet.precision);
            printf("Root delay: %lu\n", packet.rootDelay);
            printf("Root dispersion: %lu\n", packet.rootDispersion);
            printf("Ref ID: %c%c%c%c\n", packet.rfid[0], packet.rfid[1], packet.rfid[2], packet.rfid[3]);
            printf("Ref timestamp: %llu\n",      packet.refTimestamp);
            printf("Original timestamp: %llu\n", packet.originalTimestamp);
            printf("Received timestamp: %llu\n", packet.receiveTimestamp);
            printf("Transmit timestamp: %llu\n", packet.transmitTimestamp);

            actualTime -= SECONDS_IN_70_YEARS;

            char outstr[4096];
            struct tm* tmp;
            tmp = localtime(&actualTime);
            strftime(outstr, sizeof(outstr), "%A, %d %B %Y %T", tmp);
            printf("Result string is \"%s\"\n", outstr);

            break;
        }
    }

    return 0;
}

