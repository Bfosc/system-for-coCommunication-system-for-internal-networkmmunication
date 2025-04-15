#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define MAX_CONN 5
#define BUFFER_SIZE 1024

int client_sock;

void *recv_handler(void *arg) {
    char buffer[BUFFER_SIZE];
    while(1) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t len = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if(len <= 0) {
            printf("客户端断开连接\n");
            break;
        }
        printf("收到消息: %s", buffer);
    }
    close(client_sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "用法: %s <监听端口>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_fd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int port = atoi(argv[1]);

    // 创建TCP套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket创建失败");
        exit(EXIT_FAILURE);
    }

    // 设置端口复用
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 配置服务端地址
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    // 绑定套接字
    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("绑定失败");
        exit(EXIT_FAILURE);
    }

    // 开始监听
    if (listen(server_fd, MAX_CONN) == -1) {
        perror("监听失败");
        exit(EXIT_FAILURE);
    }
    printf("服务端已启动，监听端口：%d\n", port);

    // 接受客户端连接
    if ((client_sock = accept(server_fd, (struct sockaddr *)&cli_addr, &cli_len)) == -1) {
        perror("接受连接失败");
        exit(EXIT_FAILURE);
    }
    printf("客户端已连接: %s:%d\n", 
           inet_ntoa(cli_addr.sin_addr), 
           ntohs(cli_addr.sin_port));

    // 创建接收线程
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, recv_handler, NULL);

    // 主线程处理发送
    char message[BUFFER_SIZE];
    while(1) {
        fgets(message, BUFFER_SIZE, stdin);
        if(strcmp(message, "exit\n") == 0) break;
        send(client_sock, message, strlen(message), 0);
    }

    close(server_fd);
    return 0;
}
