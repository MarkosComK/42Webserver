#include <iostream>
#include <cstring>
#include <stdexcept>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static void die(const char *msg){
    std::perror(msg);
    std:exit(1);
}

static std::string itoa_int(int n){
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%d", n);
    return std::string(buf);
}

static bool reacv_until_end(int client_fd, std::string &out){
    const size_t MAX_HEADER = 8192; //8kb
    char buf[1024];

    while (true)
    {
        std::memset(buf, 0, sizeof(buf));
        int n = recv(client_fd, buf, sizeof(buf), 0);
        if (n < 0)
            return false; //error
        if (n == 0)
            return false; //client closed        
        out.append(buf, n);
        //protecao requests gigantes;
        if (out.size() > MAX_HEADER)
            return false;
        
        //final 
        if (out.find("\r\n\r\n") != std::string::npos)
            return true;
    }
}

static void handle_client(int client_fd){
    //recv() (blocking)
    std::string request;
    if (!reacv_until_end(client_fd, request)){
        std::cout << "Failed Reading Headers" << std::endl;
        return;
    }
    
    std::cout << "Full Headers Received" << std::endl;
    std::cout << request << std::endl;

    //Send Fixed
    const std::string body = "Hello! I'm reading...\n";
    const std::string ans = 
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: " + itoa_int((int)body.size()) + "\r\n"
        "Connection: closed\r\n"
        "\r\n" + body;
    
    if (send(client_fd, ans.c_str(), ans.size(), 0) < 0)
        die("send");
}

int main(int argc, char **argv){
    int port = 8080;
    if (argc >= 2)
        port = atoi(argv[1]);

    //socket
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) 
        die("socket");

    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        die("setsockopt");
    //bind
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) < 0)
        die("bind");
    
    //listen
    if (listen(listen_fd, 128) < 0)
        die("listen");
    
    std::cout << "Listening on port " << port << "..." << std::endl;

    // accept (blocking)
    while (true)
    {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(listen_fd, (sockaddr*)&client_addr, &client_len);
        if (client_fd < 0)
            die("accept");
        
        std::cout << "Client connected from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << std::endl;
        handle_client(client_fd);
        close(client_fd);
        std::cout << "Client done (closed)\n";
    }
    
    close(listen_fd);
    return 0;
}