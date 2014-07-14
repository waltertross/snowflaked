#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <event.h>
#include <evhttp.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

const int LOW = 1;
const int HIGH = 500;

void send_command(int sd, char *command);

int main(int argc, char **argv) {
	char *ipaddress = "127.0.0.1";
	int port = 8002;
        int iterations = 1000;

	int c;
	while (1) {
		static struct option long_options[] = {
			{"ip",         required_argument, 0, 'i'},
			{"port",       required_argument, 0, 'p'},
			{"iterations", required_argument, 0, 'n'},
			{0, 0, 0, 0}
		};
		int option_index = 0;
		c = getopt_long(argc, argv, "i:p:n:", long_options, &option_index);
		if (c == -1) { break; }
		switch (c) {
			case 0:
				if (long_options[option_index].flag != 0) { break; }
				printf ("option %s", long_options[option_index].name);
				if (optarg) { printf(" with arg %s", optarg); }
				printf("\n");
				break;
			case 'i':
				ipaddress = optarg;
				break;
			case 'p':
				port = atoi(optarg);
				break;
			case 'n':
				iterations = atoi(optarg);
				break;
			case '?':
				/* getopt_long already printed an error message. */
				break;
			default:
				abort();
		}
	}

	struct hostent *hp;
	struct sockaddr_in pin;
	int sd;

	if ((hp = gethostbyname(ipaddress)) == 0) {
		perror("gethostbyname");
		exit(1);
	}

	memset(&pin, 0, sizeof(pin));
	pin.sin_family = AF_INET;
	pin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
	pin.sin_port = htons(port);

	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	if (connect(sd,(struct sockaddr *)  &pin, sizeof(pin)) == -1) {
		perror("connect");
		exit(1);
	}

	time_t seconds;
	time(&seconds);
	srand((unsigned int) seconds);

	int n = 0;
	int list[1000];
	while (n < 1000) {
		list[n] = rand() % (HIGH - LOW + 1) + LOW;
		n++;
	}

	n = 0;
        char msg[10];
        sprintf(msg, "GET\r\n");
	while (n < iterations) {
		printf("Sending command 'update %d 1' ... ", list[n]);
		send_command(sd, msg);
		n++;
	}
	send_command(sd, "INFO\r\n");

	close(sd);

	return 0;
}

void send_command(int sd, char *command) {
	if (send(sd, command, strlen(command), 0) == -1) {
		perror("send");
		exit(1);
	}
	char buf[300];
	int numbytes;
	if((numbytes = recv(sd, buf, 300-1, 0)) == -1) {
		perror("recv()");
		exit(1);
	}
	buf[numbytes] = '\0';
	printf("Client-Received: %s", buf);
}
