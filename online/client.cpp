#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main(int argc, char *argv[])
{
    char ip[] = "127.0.0.1";
    char port[] = "8888";

    struct in_addr addr;
    int ip_num = inet_aton(ip, &addr);
    int port_num = htons(atoi(port));

    struct sockaddr_in socket_addr;
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_addr = addr;
    socket_addr.sin_port = port_num;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    int connect_ret = connect(sockfd, (struct sockaddr *)&socket_addr, sizeof(socket_addr));
    if (connect_ret == -1)
    {
        perror("connect");
        return -1;
    }

    int epollfd = epoll_create(1);

    struct epoll_event event;
    event.events = EPOLLIN;

    event.data.fd = STDIN_FILENO;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &event);

    event.data.fd = sockfd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);

    std::cout << "Enter 1 or 2 (recommend or query): ";
    std::flush(std::cout);
    while (1)
    {
        struct epoll_event events[2];
        int num = epoll_wait(epollfd, events, 2, -1);
        for (int i = 0; i < num; i++)
        {
            int fd = events[i].data.fd;
            if (fd == STDIN_FILENO)
            {
                int query_id;
                std::string msg;

                std::cin >> query_id;
                std::cin.ignore(); // 忽略输入缓冲区中的换行符
                std::cout << "Enter msg: ";
                std::getline(std::cin, msg);

                // 构建JSON字符串
                json json_message;
                json_message["query_id"] = query_id;
                json_message["msg"] = msg;

                std::string json_str = json_message.dump() + "\n";

                write(sockfd, json_str.c_str(), json_str.length());
            }
            else if (fd == sockfd)
            {
                int msglen = 0;
                int ret = recv(sockfd, &msglen, sizeof(int), MSG_WAITALL);
                if (ret == -1)
                {
                    perror("recv");
                    goto end;
                }
                else if (ret == 0)
                {
                    std::cout << "Server closed the connection" << std::endl;
                    goto end;
                }
                char buf[msglen + 1];
                recv(sockfd, buf, msglen, MSG_WAITALL);
                buf[msglen] = '\0';
                // 直接输出
                std::cout << "\nServer response:\n"
                          << buf << "\n\n";
                std::cout << "Enter 1 or 2 (recommend or query): ";
                std::flush(std::cout);
            }
        }
    }

end:
    close(sockfd);
    return 0;
}