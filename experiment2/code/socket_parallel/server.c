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

int server_sock_listen, server_sock_data;

void release_socket(int signo)
{
	close(server_sock_listen);
	close(server_sock_data);
	exit(-1);
}

int main(int argc, char *argv[])
{
	signal(SIGINT, release_socket);

	struct sockaddr_in server_addr;
	char recv_msg[255];

	if (argc < 3)
	{
		printf("Usage: %s <ip> <port>", argv[0]);
	}

	/* 创建socket */
	server_sock_listen = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock_listen == -1)
	{
		perror("socket");
		return -1;
	}

	/* 指定服务器地址 */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
//	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY表示本机所有IP地址
	inet_aton(argv[1], &server_addr.sin_addr);
	memset(&server_addr.sin_zero, 0, sizeof(server_addr.sin_zero)); //零填充

	/* 绑定socket与地址 */
	long err = bind(server_sock_listen, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (err == -1)
	{
		perror("bind");
		goto release_listen;
	}

	/* 监听socket */
	err = listen(server_sock_listen, 5);
	if (err == -1)
	{
		perror("listen");
		goto release_listen;
	}

	struct sockaddr_in client_addr; //保存用户Socket信息
	memset(&client_addr, 0, sizeof(client_addr));
	socklen_t cli_len;
	cli_len = sizeof(client_addr);

	for (;;)
	{
		server_sock_data = accept(server_sock_listen, (struct sockaddr *)&client_addr, &cli_len);
		if (server_sock_data == -1)
		{
			perror("accept");
			goto release_listen;
		}

		pid_t pid = fork();
		if (pid < 0)
		{
			perror("fork");
		}
		else if (pid == 0)
		{
			printf("Connect client %s:%d\n",
				   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
			for (;;)
			{
				/* 接收并显示消息 */
				memset(recv_msg, 0, sizeof(recv_msg)); //接收数组置零
				err = recv(server_sock_data, recv_msg, sizeof(recv_msg), 0);
				if (err == -1)
				{
					perror("recv");
					goto release_data;
				}

				printf("Recv from %s:%d: %s",
					   inet_ntoa(client_addr.sin_addr),
					   ntohs(client_addr.sin_port), recv_msg);

				/* 发送消息 */
				err = send(server_sock_data, recv_msg, strlen(recv_msg), 0);
				if (err == -1)
				{
					perror("send");
					goto release_data;
				}
				printf("Sent to %s:%d: %s",
					   inet_ntoa(client_addr.sin_addr),
					   ntohs(client_addr.sin_port), recv_msg);

				if (strncmp(recv_msg, "bye", 3) == 0)
				{
					close(server_sock_data);
					close(server_sock_listen);
					return 0;
				}
			}
		}
		else
		{
//			close(server_sock_data);
			continue;
		}
	}

	close(server_sock_data);
	close(server_sock_listen);
	return 0;

release_data:
	/* 关闭数据socket */
	close(server_sock_data);
release_listen:
	/* 关闭监听socket */
	close(server_sock_listen);
	return -1;
}
