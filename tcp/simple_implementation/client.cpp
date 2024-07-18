#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fstream>
#include <ctime>
#include <sstream>

#define SERVER_PORT 8080

using namespace std;

class TcpClient {
public:
  TcpClient(const string& server_address) : server_address_(server_address) {}

  bool Connect() {
    // Create a socket
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ == -1) {
      perror("socket creation failed");
      return false;
    }

    // Configure server address
    servaddr_.sin_family = AF_INET;
    servaddr_.sin_port = htons(SERVER_PORT);
    servaddr_.sin_addr.s_addr = inet_addr(server_address_.c_str());

    // Connect to the server
    if (connect(sockfd_, (struct sockaddr*)&servaddr_, sizeof(servaddr_)) < 0) {
      perror("connection failed");
      return false;
    }

    writeLog("Connected to server");
    return true;
  }

  void SendMessage(const string& message) {
    send(sockfd_, message.c_str(), message.length(), 0);
  }

  string ReceiveMessage() {
    char buffer[1024] = {0};
    int valread = read(sockfd_, buffer, sizeof(buffer));

    if (valread == 0) {
      cout << "Server disconnected" << endl;
      writeLog("Server disconnected");
      return "";
    } else if (valread < 0) {
      perror("read");
      return "";
    }

    return string(buffer);
  }

  void Close() {
    close(sockfd_);
  }

private:
  void writeLog(const string& message) {
    ofstream logfile("client_log.txt", ios::app); // Open in append mode
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

  int sockfd_;
  struct sockaddr_in servaddr_;
  string server_address_;
};

int main() {
  string server_address = "127.0.0.1"; // Replace with server IP if needed

  TcpClient client(server_address);
  if (!client.Connect()) {
    cerr << "Failed to connect to server" << endl;
    return 1;
  }

  while (true) {
    string message;
    cout << "Enter message (or 'quit' to exit): ";
    getline(cin, message);

    client.SendMessage(message);
    if (message == "quit") {
      break;
    }

    string response = client.ReceiveMessage();
    if (response.empty()) {
      break;
    }
    cout << "Server: " << response << endl;
  }
  client.Close();
  return 0;
}
