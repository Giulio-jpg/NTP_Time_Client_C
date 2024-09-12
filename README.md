# About

It's a simple program written in C, which asks the current time to google NTP server time4.google.com

NTP packet consists of this structure:


``` c
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
```

To show the current date I took the receivetimestamp:

``` C
time_t currentTime = ntohl(packet.receiveTimestamp);
```

I used ntohl because I only need the first 32 bits, I can afford to cut the others (I still kept the complete data in another variable)

NTP server clock starts from 1900, but our clock starts from 1970, so I had to subtract the seconds there are in 70 years from our currentTime

``` C
#define SECONDS_IN_70_YEARS 2208988800
```


``` C
currentTime -= SECONDS_IN_70_YEARS;

char outstr[4096];
struct tm* tmp;
tmp = localtime(&currentTime);
strftime(outstr, sizeof(outstr), "%A, %d %B %Y %T", tmp);
printf("The date is \"%s\"\n", outstr);
```

Once the program has been executed, the result will be:

![image](https://github.com/user-attachments/assets/fac10a16-09d5-4ea9-8638-7a67360cf7a9)

















