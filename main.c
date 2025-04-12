#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define SERVER "raw.githubusercontent.com"
#define FILE_PATH "/jeepyq/bj/main/depression_tips.txt"
#define PORT "443"  // HTTPS uses port 443
void fetchFileFromGitHub() {
    int sock;
    struct addrinfo hints, *res;
    char request[512], response[4096];

    // Initialize OpenSSL
    SSL_library_init();
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        perror("SSL_CTX creation failed");
        return;
    }

    // Resolve GitHub's raw URL
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(SERVER, PORT, &hints, &res) != 0) {
        perror("getaddrinfo failed");
        return;
    }

    // Create socket
    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        perror("Socket creation failed");
        return;
    }

    // Connect to GitHub
    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        perror("Connection failed");
        return;
    }

    freeaddrinfo(res);  // No longer needed

    // Set up SSL
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);
    if (SSL_connect(ssl) != 1) {
        perror("SSL connection failed");
        return;
    }

    // Create HTTPS GET request
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n"
             "\r\n",
             FILE_PATH, SERVER);

    // Send request over SSL
    SSL_write(ssl, request, strlen(request));

    printf("THE ONLY SOLUTION OF YOUR DEPRESSION: \n");
    
    // Read response
    int bytes_received;
    int header_end = 0;  // Flag to detect header end
    while ((bytes_received = SSL_read(ssl, response, sizeof(response) - 1)) > 0) {
        response[bytes_received] = '\0';  // Null-terminate response
        
        // Find the end of HTTP headers (first occurrence of "\r\n\r\n")
        if (!header_end) {
            char *body_start = strstr(response, "\r\n\r\n");
            if (body_start) {
                header_end = 1;
                body_start += 4;  // Move past "\r\n\r\n"
                printf("%s", body_start);
            }
        } else {
            printf("%s", response);
        }
    }

    // Cleanup
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sock);
}

int main() {
    fetchFileFromGitHub();
    return 0;
}
