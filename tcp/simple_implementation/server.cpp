#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fstream>
#include <ctime>
#include <sstream>

#define PORT 8080

using namespace std;

class TcpServer {
    private:
        int server_socket_;
        struct sockaddr_in address_;
        int port_;

        void HandleConnection() {
            int new_socket;
            socklen_t addrlen = sizeof(address_);
            
            if ((new_socket = accept(server_socket_, (struct sockaddr *)&address_, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
            }
            writeLog("Socket connection accepted");

            CommunicateWithClient(new_socket);
            close(new_socket);
        }

        void CommunicateWithClient(int socket) {
            char buffer[1024] = {0};
            int valread;
            while (true) {
                valread = read(socket, buffer, sizeof(buffer));
                if (valread == 0) {
                    cout << "Client disconnected" << endl;
                    writeLog("Client disconnected");
                    break;
                } else if (valread < 0) {
                    perror("read");
                    break;
                }
                else {
                    buffer[valread] = '\0';
                    writeLog("Message from client : " + string(buffer));
                }
                
                send(socket, buffer, valread, 0);
                cout << "Client: " << buffer << endl;
                buffer[valread] = '\0';
            }
        }

        void writeLog(const std::string& message) {
            ofstream logfile("server_log.txt", ios::app); // Open in append mode
            if (!logfile.is_open()) {
                cerr << "Error opening log file" << endl;
                return;
            }
            
            time_t now = time(0);
            tm *local_time = localtime(&now);
            char buffer[80];
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_time);
            string timestamp(buffer);
            logfile << timestamp << ": " << message << endl;
            logfile.close();
        }

    public:
        TcpServer(int port) : port_(port) {

            if ((server_socket_ = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
                perror("socket failed");
                writeLog("Socket falied hence terminating the code");
                exit(EXIT_FAILURE);
            }
            
            address_.sin_family = AF_INET;
            address_.sin_addr.s_addr = inet_addr("127.0.0.1");;
            address_.sin_port = htons(port_);

            if (bind(server_socket_, (struct sockaddr *)&address_, sizeof(address_)) < 0) {
                perror("bind failed");
                writeLog("Binding falied hence terminating the code");
                exit(EXIT_FAILURE);
            }
        }

        void Start() {
            if (listen(server_socket_, 3) < 0) {
                perror("listen");
                exit(EXIT_FAILURE);
            }
            writeLog("\n===========================================================================================================\n");
            writeLog("The server is up and listening");
            cout << "Server listening on port : " << port_ << endl;
            writeLog("Server listening on port : " + to_string(port_));
            while (true) {
                HandleConnection();
            }
        }
};

int main() {
    TcpServer server(PORT);
    server.Start();
    return 0;
}
