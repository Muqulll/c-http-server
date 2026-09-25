#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

void serve_html(int client_fd)
{
    FILE *file = fopen("base_server_test.html", "r");
    if (file == NULL)
    {
        // Send a basic 404 if the file is missing
        char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        send(client_fd, not_found, strlen(not_found), 0);
        return;
    }

    // 1. Calculate file size for the Content-Length header
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 2. Send HTTP Response Headers first
    char header[256];
    int header_len = snprintf(header, sizeof(header),
                              "HTTP/1.1 200 OK\r\n"
                              "Content-Type: text/html; charset=UTF-8\r\n"
                              "Content-Length: %ld\r\n"
                              "Connection: close\r\n"
                              "\r\n",
                              file_size);

    send(client_fd, header, header_len, 0);

    char chunk[4096];
    size_t bytes_read;

    while ((bytes_read = fread(chunk, 1, sizeof(chunk), file)) > 0)
    {
        send(client_fd, chunk, bytes_read, 0);
    }
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
        perror("get socket first");
        exit(-1);
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setpocket failed...");
    }

    int bind_result = bind(sockfd, res->ai_addr, res->ai_addrlen);
    if (bind_result < 0)
    {
        perror("Bind Error");
        exit(-1);
    }
    freeaddrinfo(res);
    
    printf("bind result is: %d\n", bind_result);

    int listen_result = listen(sockfd, 20);
    if (listen_result < 0)
    {
        perror("Cannot Listen");
        exit(-1);
    }

    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);

    while (1)
    {
        int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

        int max_len = 1000;
        char http_request[max_len];
        memset(http_request, 0, max_len);
        int request_recieved = recv(new_fd, http_request, max_len, 0);
        puts(http_request);

        if (strstr(http_request, "GET /") != NULL)
        {
            printf("The User asked for the HomePage.\n");
            serve_html(new_fd);
        }
        close(new_fd);
    }
    return 0;
}