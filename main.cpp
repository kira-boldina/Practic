#include <sys/mount.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h> 
#include <arpa/inet.h>

using namespace std;
int hostname_to_ip(char *hostname, char *ip);

int main()
{
    string src = "\\\\desktop-xxxxxxxxxxxxxx\\smb";
    string dst = "/mnt/sm";
    string fstype = "cifs";
    char ip[100];
    char hostname[] = "desktop-xxxxxxxxxxxx";
    hostname_to_ip(hostname, ip);

    printf("%s\n", ip);
    string ips(ip);
    string all_string = "ip=" + ips + ",username=smbguest,password=xxxxxxx";

    printf("src: %s\n", src.c_str());
    printf("all_string: %s\n", all_string.c_str());

    if (-1 == mount(src.c_str(), dst.c_str(), fstype.c_str(), 0, all_string.c_str()))
    {
        printf("mount failed with error: %s\n", strerror(errno));
    }
    else
        printf("mount success!\n");

    if (umount2(dst.c_str(), MNT_FORCE) < 0)
    {
        printf("unmount failed with error: %s\n", strerror(errno));
    }
    else
        printf("unmount success!\n");

    return 0;
}

int hostname_to_ip(char *hostname, char *ip)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_in *h;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; 
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(hostname, "http", &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }


    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        h = (struct sockaddr_in *)p->ai_addr;
        strcpy(ip, inet_ntoa(h->sin_addr));
    }

    freeaddrinfo(servinfo); // all done with this structure
    return 0;
}
