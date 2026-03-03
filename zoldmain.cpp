
#include <cstring>
#include <cstdlib>


static std::string itoa_int(int n){
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%d", n);
    return std::string(buf);
}

struct ClientState
{
    std::string in;          // buffer de leitura
    std::string out;         // buffer de escrita (resposta)
    size_t      out_sent;    // quantos bytes já enviámos de out
    bool        headers_done;

    ClientState() : out_sent(0), headers_done(false) {}
};

static std::string build_response_200_text(const std::string& body)
{
    return
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: " + itoa_int((int)body.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" + body;
}

static void close_client(std::vector<pollfd> &pfds, std::map<int, ClientState> &clients, size_t id){
    int fd = pfds[id].fd;
    close(fd);
    clients.erase(fd);
    pfds.erase(pfds.begin() + id);
}

int main(int argc, char **argv){

    while (true)
    {
        int ret = poll(&pfds[0], pfds.size(), -1);
        if (ret < 0){
            if (errno == EINTR)
                continue;
            die("poll");
        }

        //events
        for (size_t i = 0; i < pfds.size(); ++i){
            if (pfds[i].revents == 0)
                continue;
            //errors / hangups
            if (pfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)){
                if (pfds[i].fd == listen_fd)
                    die("Listen socket error!");
                close_client(pfds, clients, i);
                --i;
                continue;
            }
            //Accept New Clients
            if (pfds[i].fd == listen_fd && (pfds[i].revents & POLLIN)){
                while (true){
                    sockaddr_in caddr;
                    socklen_t clen = sizeof(caddr);
                    int cfd = accept(listen_fd, (sockaddr*)&caddr, &clen);
                    if (cfd < 0){
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            break;
                        die("accept");
                    }
                    set_nonblocking(cfd);
                    pollfd p ;
                    p.fd = cfd;
                    p.events = POLLIN;
                    p.revents = 0;
                    pfds.push_back(p);
                    clients[cfd] = ClientState();
                    std::cout << "Client connected from " << inet_ntoa(caddr.sin_addr) << ":" << ntohs(caddr.sin_port) << " | fd = " << cfd << std::endl;
                }
                continue;
            }

            //Client Read
            if (pfds[i].revents & POLLIN){
                int cfd = pfds[i].fd;
                ClientState &st = clients[cfd];

                char buf[2048];
                while (true){
                    int n = recv(cfd, buf, sizeof(buf), 0);
                    if (n > 0){
                        st.in.append(buf, n);
                        if (!st.headers_done && st.in.find("\r\n\r\n") != std::string::npos){
                            st.headers_done = true;

                            //Ainda sem parsing, fixo
                            const std::string body = "Hello! poll + non-blocking OK\n";
                            st.out = build_response_200_text(body);
                            st.out_sent = 0;

                            //mudar interesse para escrita
                            pfds[i].events = POLLOUT;
                            break; //já temos resposta pronta
                        }

                        //limitar headers para evitar infinito
                        if (st.in.size() > 8192){
                            //fecha por simplicidade e envia 431/413
                            close_client(pfds, clients, i);
                            --i;
                            break;
                        }
                    } else if (n == 0) {
                        //cliente fechou
                        close_client(pfds, clients, i);
                        --i;
                        break;
                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            break; //sem mais dados agora
                        close_client(pfds, clients, i);
                        --i;
                        break;
                    }
                }
                continue;
            }

            // client -> write
            if (pfds[i].revents & POLLOUT){
                int cfd = pfds[i].fd;
                ClientState &st = clients[cfd];

                while (st.out_sent < st.out.size())
                {
                    const char *data = st.out.c_str() + st.out_sent;
                    size_t left = st.out.size() - st.out_sent;

                    int n = send(cfd, data, left, 0);
                    if (n > 0)
                        st.out_sent += (size_t)n;
                    else if (n < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            break; //tenta depois
                        close_client(pfds, clients, i);
                        --i;
                        break;
                    }
                }
                //terminou de enviar -> fechar http 1.0
                if (i < pfds.size() && st.out_sent >= st.out.size())
                {
                    close_client(pfds, clients, i);
                    --i;
                }
            }
        }

        // limpar revents
        for (size_t i = 0; i < pfds.size(); i++)
            pfds[i].revents = 0;
    }
    close(listen_fd);
    return 0;
}