// Comprehensive stress test for socket-like API

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <arpa/inet.h>
#include <string.h>

#include <netinet/in.h>
#include <netdb.h>

#include "libzt.h"

int main()
{
	char *nwid = (char *)"e5cd7a9e1c0fd272";

	// Get ZeroTier core version
	char ver[ZT_VER_STR_LEN];
	zts_core_version(ver);
	printf("zts_core_version = %s\n", ver);
	
	// Get SDK version
	zts_sdk_version(ver);
	printf("zts_sdk_version = %s\n", ver);

	// Spawns a couple threads to support ZeroTier core, userspace network stack, and generates ID in ./zt
	zts_start("./zt"); 

	// Print the device/app ID (this is also the ID you'd see in ZeroTier Central)
	char id[ZT_ID_LEN + 1];
	zts_get_device_id(id);
	printf("id = %s\n", id);

	// Get the home path of this ZeroTier instance, where we store identity keys, conf files, etc
	char homePath[ZT_HOME_PATH_MAX_LEN+1];
	zts_get_homepath(homePath, ZT_HOME_PATH_MAX_LEN);
	printf("homePath = %s\n", homePath);

	// Wait for ZeroTier service to start
	while(!zts_running()) {
		printf("wating for service to start\n");
		sleep(1);
	}

	// Join a network
	zts_join(nwid);

	// Wait for ZeroTier service to issue an address to the device on the given network
	while(!zts_has_address("e5cd7a9e1c0fd272")) {
		printf("waiting for service to issue an address\n");
		sleep(1);
	}
	// Begin Socket API calls

		int err;
		int sockfd;
		int port = 7878;
		struct sockaddr_in addr;

		// socket()
		if((sockfd = zts_socket(AF_INET, SOCK_STREAM, 0)) < 0)
			printf("error creating ZeroTier socket");
		else
			printf("sockfd = %d\n", sockfd);

		// connect() IPv6
		if(false)
		{
			struct hostent *server = gethostbyname2("fde5:cd7a:9e1c:0fd2:7299:9367:5993:3b86",AF_INET6);
	    	struct sockaddr_in6 serv_addr;
			memset((char *) &serv_addr, 0, sizeof(serv_addr));
			serv_addr.sin6_flowinfo = 0;
			serv_addr.sin6_family = AF_INET6;
			memmove((char *) &serv_addr.sin6_addr.s6_addr, (char *) server->h_addr, server->h_length);
			serv_addr.sin6_port = htons( port );
			if((err = zts_connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0) {
				printf("error connecting to remote host (%d)\n", err);
				return -1;
			}
		}
		// connect() IPv4
		if(true)
		{
			addr.sin_addr.s_addr = inet_addr("10.9.9.20");
	    	addr.sin_family = AF_INET;
	    	addr.sin_port = htons( port );
			if((err = zts_connect(sockfd, (const struct sockaddr *)&addr, sizeof(addr))) < 0) {
				printf("error connecting to remote host (%d)\n", err);
				return -1;
			}

			zts_write(sockfd, "hello", 5);
			sleep(3);
			zts_close(sockfd);
		}
		// bind() ipv4
		if(false)
		{
			//addr.sin_addr.s_addr = INADDR_ANY; // TODO: Requires significant socket multiplexer work
			addr.sin_addr.s_addr = inet_addr("10.9.9.40");
	    	addr.sin_family = AF_INET;
	    	addr.sin_port = htons( port );
			if((err = zts_bind(sockfd, (const struct sockaddr *)&addr, sizeof(addr))) < 0) {
				printf("error binding to interface (%d)\n", err);
				return -1;
			}
			zts_listen(sockfd, 1);
			struct sockaddr_in client;
			int c = sizeof(struct sockaddr_in);
			
			int accept_fd = zts_accept(sockfd, (struct sockaddr *)&client, (socklen_t*)&c);
			
			printf("reading from buffer\n");
			char newbuf[32];
			memset(newbuf, 0, 32);
			read(accept_fd, newbuf, 20);
			printf("newbuf = %s\n", newbuf);
		}

	// End Socket API calls

	// Get the ipv4 address assigned for this network
	char addr_str[ZT_MAX_IPADDR_LEN];
	zts_get_ipv4_address(nwid, addr_str, ZT_MAX_IPADDR_LEN);
	printf("ipv4 = %s\n", addr_str);

	zts_get_ipv6_address(nwid, addr_str, ZT_MAX_IPADDR_LEN);
	printf("ipv6 = %s\n", addr_str);

	printf("peer_count = %lu\n", zts_get_peer_count());

	while(1) {
		sleep(1);
	}

	// Stop service, delete tap interfaces, and network stack
	zts_stop();
	return 0;
}