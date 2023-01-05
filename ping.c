#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
// ping packet size including icmp header
#define PING_PCKT_SIZE 64
// automatic port number
#define PORT_NO 0
#define TIMEOUT_RECEIVE 1000000
#define RCV_TO 1


#define IP4_HEADER_LEN 20
#define ICMP_HEADER_LEN 8


//calculate checksum
unsigned short calculate_checksum (unsigned short* buffer, int bytes); 
/*
1. get an arg = host to ping
*/

/*
2. send icmp-echo-request to host
*/

/*
3. recieved replay? go to step 2
*/

/*
3.1 for each replay: print pckt ip
                           pckt seq num
                           time from req to rep
*/

// command: make clean && make all && ./parta

// general struct for icmp packets
struct icmpPkt
{
    uint8_t type;
    uint8_t code;
    // u_int16_t checksum;
    uint16_t idprocces;
    u_int16_t seqnumber;

    double senddata;
    char msg[11];
};

double gettime()
{
    struct timeval timevalue;
    clock_gettime(&timevalue, NULL);
    return timevalue.tv_sec + ((double)timevalue.tv_usec) / 1000;
}


int sendechoRequest(int socket, struct sockaddr_in *address, int idprocces, int seqnumber)
{
    struct icmpPkt icmpEcho;
    bzero(&icmpEcho, sizeof(icmpEcho));

    icmpEcho.type = 8;
    icmpEcho.code = 0;
    icmpEcho.idprocces = htons(idprocces);
    icmpEcho.seqnumber = htons(seqnumber);

    int bytes = sendto(socket, &icmpEcho, sizeof(icmpEcho), 0, (struct sockaddr *)address, sizeof(*address));
    if (bytes == -1)
        return -1;

    return 0;
}

int receiveEchoReply(int socket, int idprocces)
{
    char buffer[1500];
    struct sockaddr_in peerAdress;
    int addressLength = sizeof(peerAdress);
    int bytes = recvfrom(socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&peerAdress, &addressLength);
    if (bytes == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return 0;
        }
        return -1;
    }

    // find the icmp packet in ip packet
    struct icmpPkt *icmpEcho = (struct icmpPkt *)(buffer + 20);
    // type check:
    if (icmpEcho->type != 0 || icmpEcho->code != 0)
    {
        return 0;
    }
    // id check:
    if (ntohs(icmpEcho->idprocces) != idprocces)
    {
        return 0;
    }
    // print ip, sequence number and time:
    printf("%s sequence number: %5.2fms\n", inet_ntoa(peerAdress.sin_addr), ntohs(icmpEcho->seqnumber), (gettime() - icmpEcho->senddata) * 1000);
    return 0;
}

int ping(const char *ip)
{

    printf("start ping\n");
    // save destination address
    struct sockaddr_in address;
    bzero(&address, sizeof(address));

    // set port = 0, fill in address
    address.sin_family = AF_INET;
    address.sin_port = 0;

    if (inet_aton(ip, (struct in_addr *)&address.sin_addr.s_addr) == 0)
    {
        return -1;
    }

    // create raw socket for icmp
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == -1)
    {
        return -1;
    }

    printf("create raw socket\n");
    // int ttl = 64;
    // if (setsockopt(socket, SOL_IP, IP_TTL,  &ttl, sizeof(ttl)) != 0)
    // {
    //     printf("\nSetting socket options to TTL failed!\n");
    //     return ;

    // } else {
    //     printf("\nSocket set to TTL..\n");
    // }
    // printf("set ttl\n");

    // socket timeout
    struct timeval timevalue;
    timevalue.tv_sec = 0;
    timevalue.tv_usec = TIMEOUT_RECEIVE;
    int ret = setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timevalue, sizeof(timevalue));
    if (ret == -1)
    {
        return -1;
    }
    double nexttimestamp = gettime();
    int idprocess = getpid();
    int sequencenumber = 1;
    printf("set timeout\n");

    while (1)
    {
        printf("in while loop\n");
        if (gettime() >= nexttimestamp)
        {
            ret = sendechoRequest(socket, &address, idprocess, sequencenumber);
            if (ret == -1)
            {
                perror("failed with simple send.  L\n");
            }
            nexttimestamp += 1;
            sequencenumber += 1;
        }
        ret = receiveEchoReply(socket, idprocess);
        if (ret == -1)
        {
            perror("failed with simple receive.  L\n");
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    struct icmp icmphdr;
    char dataMessege[IP_MAXPACKET] = "This is the ping!\n";
    int dataLength = strlen(dataMessege) + 1;

    //icmp header///
    //msg type
    icmphdr.icmp_type = ICMP_ECHO;
    icmphdr.icmp_code = 0;
    icmphdr.icmp_id = 18;
    icmphdr.icmp_cksum = 0;
     // Combine the packet
    char packet[IP_MAXPACKET];

    // Next, ICMP header
    memcpy((packet), &icmphdr, ICMP_HEADERLEN);

    // After ICMP header, add the ICMP data.
    memcpy(packet + ICMP_HDRLEN, data, datalen);

    // Calculate the ICMP header checksum
    icmphdr.icmp_cksum = calculate_checksum((unsigned short *)(packet), ICMP_HDRLEN + datalen);
    memcpy((packet), &icmphdr, ICMP_HDRLEN);

    struct sockaddr_in dest_in;
    memset(&dest_in, 0, sizeof(struct sockaddr_in));
    dest_in.sin_family = AF_INET;

    // The port is irrelant for Networking and therefore was zeroed.
    // dest_in.sin_addr.s_addr = iphdr.ip_dst.s_addr;
    dest_in.sin_addr.s_addr = inet_addr(DESTINATION_IP);
    // inet_pton(AF_INET, DESTINATION_IP, &(dest_in.sin_addr.s_addr));

    // Create raw socket for IP-RAW (make IP-header by yourself)
    int sock = -1;
    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
    {
        fprintf(stderr, "socket() failed with error: %d", errno);
        fprintf(stderr, "To create a raw socket, the process needs to be run by Admin/root user.\n\n");
        return -1;
    }

    struct timeval start, end;
    gettimeofday(&start, 0);

    // Send the packet using sendto() for sending datagrams.
    int bytes_sent = sendto(sock, packet, ICMP_HDRLEN + datalen, 0, (struct sockaddr *)&dest_in, sizeof(dest_in));
    if (bytes_sent == -1)
    {
        fprintf(stderr, "sendto() failed with error: %d", errno);
        return -1;
    }
    printf("Successfuly sent one packet : ICMP HEADER : %d bytes, data length : %d , icmp header : %d \n", bytes_sent, datalen, ICMP_HDRLEN);

    // Get the ping response
    bzero(packet, IP_MAXPACKET);
    socklen_t len = sizeof(dest_in);
    ssize_t bytes_received = -1;
    while ((bytes_received = recvfrom(sock, packet, sizeof(packet), 0, (struct sockaddr *)&dest_in, &len)))
    {
        if (bytes_received > 0)
        {
            // Check the IP header
            struct iphdr *iphdr = (struct iphdr *)packet;
            struct icmphdr *icmphdr = (struct icmphdr *)(packet + (iphdr->ihl * 4));
            // printf("%ld bytes from %s\n", bytes_received, inet_ntoa(dest_in.sin_addr));
            // icmphdr->type

            printf("Successfuly received one packet with %d bytes : data length : %d , icmp header : %d , ip header : %d \n", bytes_received, datalen, ICMP_HDRLEN, IP4_HDRLEN);

            break;
        }
    }

    gettimeofday(&end, 0);

    char reply[IP_MAXPACKET];
    memcpy(reply, packet + ICMP_HDRLEN + IP4_HDRLEN, datalen);
    // printf("ICMP reply: %s \n", reply);

    float milliseconds = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
    unsigned long microseconds = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec);
    printf("\nRTT: %f milliseconds (%ld microseconds)\n", milliseconds, microseconds);

    // Close the raw socket descriptor.
    close(sock);

    return 0;
}

// Compute checksum (RFC 1071).
unsigned short calculate_checksum(unsigned short *paddress, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = paddress;
    unsigned short answer = 0;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1)
    {
        *((unsigned char *)&answer) = *((unsigned char *)w);
        sum += answer;
    }

    // add back carry outs from top 16 bits to low 16 bits
    sum = (sum >> 16) + (sum & 0xffff); // add hi 16 to low 16
    sum += (sum >> 16);                 // add carry
    answer = ~sum;                      // truncate to 16 bits

    return answer;
    printf("fml\n");

    ping(argv[1]);
    return 0;
}
