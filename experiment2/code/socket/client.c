#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <error.h>
#include <stdlib.h>
#include <signal.h>

char recv_msg[255];
char send_msg[255];

int client_sock;

void release_socket(int signo)
{
	close(client_sock);
}

int main(int argc, char *argv[])
{
	signal(SIGINT, release_socket);
	struct sockaddr_in server_addr;

	if (argc != 3)
	{
		printf("Usage: %s <IP> <PORT>", argv[0]);
		return -1;
	}

	/* 创建socket */
	client_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (client_sock < 0)
	{
		perror("socket");
		return -1;
	}

	/* 指定服务器地址 */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	inet_aton(argv[1], &server_addr.sin_addr);
	memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero)); //零填充

	/* 连接服务器 */
	long err = connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (err < 0)
	{
		perror("connect");
		goto release;
	}

	for (;;)
	{
		fgets(send_msg, 255, stdin);

		/* 发送消息 */
		printf("Send: %s", send_msg);
		err = send(client_sock, send_msg, strlen(send_msg), 0);
		if (err < 0)
		{
			perror("send");
			goto release;
		}

		/* 接收并显示消息 */
		memset(recv_msg, 0, sizeof(recv_msg)); //接收数组置零
		err = recv(client_sock, recv_msg, sizeof(recv_msg), 0);
		if (err < 0)
		{
			perror("recv");
			goto release;
		}

		printf("Recv: %s", recv_msg);

		if (strncmp(recv_msg, "bye", 3) == 0)
		{
			break;
		}
	}

	return 0;
release:
	/* 关闭socket */
	close(client_sock);

	return -1;
}
