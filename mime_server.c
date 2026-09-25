#define _GNU_SOURCE
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

const char *get_mime_type(const char *path)
{
    const char *dot = strrchr(path, '.');
    if (!dot)
        return "text/plain";

    if (strcmp(dot, ".html") == 0)
        return "text/html; charset=UTF-8";
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".js") == 0)
        return "text/js";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "img/jpeg";

    return "application/octet-stream";
}

int main()
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, "8080", &hints, &res);

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0)
    {
        perror("Socket Not Found!");
        exit(-1);
    }
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed!");
    }
    int bind_result = bind(sockfd, res->ai_addr, res->ai_addrlen);
    if (bind_result < 0)
    {
        perror("Bind Error");
    }
    freeaddrinfo(res);

    listen(sockfd, 20);

    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);

    while (1)
    {
        int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
        if (new_fd < 0)
        {
            perror("Accepting Connection Failed.");
            continue;
        }
        int max_len = 1000;
        char http_req[max_len];
        memset(http_req, 0, max_len);
        int request_recv = recv(new_fd, http_req, max_len, 0);
        if (request_recv > 0)
        {
            printf("--- Incoming Request ---\n%s\n");

            char method[10], raw_path[256];
            sscanf(http_req, "%s %s", method, raw_path);

            const char *filename = raw_path + 1;
            if (strlen(filename) == 0)
            {
                filename = "index.html";
            }

            FILE *file = fopen(filename, "rb");
            if (file == NULL)
            {
                char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
                send(new_fd, not_found, strlen(not_found), 0);
            }
            else
            {
                fseek(file, 0, SEEK_END);
                long fsize = ftell(file);
                fseek(file, 0, SEEK_SET);

                const char *mime_type = get_mime_type(filename);

                char header[256];
                int header_len = snprintf(header, sizeof(header),
                                         "HTTP/1.1 200 OK\r\n"
                                         "Content-Type: %s\r\n"
                                         "Content-Length: %ld\r\n"
                                         "Connection: close\r\n"
                                         "\r\n",
                                         mime_type, fsize);
                send(new_fd,header,header_len,0);
                char chunk[4096];
                size_t bytes_read;
                while((bytes_read = fread(chunk,1,sizeof(chunk),file)) > 0){
                    send(new_fd,chunk,bytes_read,0);
                }
                fclose(file);
            }
        }
        close(new_fd);
    }
    close(sockfd);
    return 0;
}