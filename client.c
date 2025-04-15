#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

int sock;

void *recv_handler(void *arg) {
    char buffer[BUFFER_SIZE];
    while(1) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t len = recv(sock, buffer, BUFFER_SIZE, 0);
        if(len <= 0) {
            printf("服务端断开连接\n");
            break;
        }
        printf("收到回复: %s", buffer);
    }
    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "用法: %s <服务器IP> <端口>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    char *server_ip = argv[1];
    int port = atoi(argv[2]);

    // 创建套接字
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket创建失败");
        exit(EXIT_FAILURE);
    }

    // 配置服务端地址
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    // IP地址转换
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("无效的IP地址格式");
        exit(EXIT_FAILURE);
    }

    // 建立连接
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("连接失败");
        exit(EXIT_FAILURE);
    }
    printf("已连接到 %s:%d\n", server_ip, port);

    // 创建接收线程
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, recv_handler, NULL);

    // 主线程处理发送
    char message[BUFFER_SIZE];
    while(1) {
        fgets(message, BUFFER_SIZE, stdin);
        if(strcmp(message, "exit\n") == 0) break;
        send(sock, message, strlen(message), 0);
    }

    close(sock);
    return 0;
}
