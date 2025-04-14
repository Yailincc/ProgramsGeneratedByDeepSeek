#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <thread>
#include <mutex>

using namespace std;

mutex logMutex;

// Configuration
const int FTP_PORT = 2121; // Using alternative FTP port
const string PUBLIC_FOLDER = "./public"; // Folder to share
const int MAX_CLIENTS = 10;

// Logging function with thread safety
void log(const string& message) {
    lock_guard<mutex> guard(logMutex);
    cout << "[Server] " << message << endl;
}

// Helper function to send data to client
void sendResponse(int clientSocket, const string& response) {
    send(clientSocket, response.c_str(), response.size(), 0);
}

// List files in the public directory
string listFiles() {
    DIR *dir;
    struct dirent *ent;
    stringstream fileList;
    
    if ((dir = opendir(PUBLIC_FOLDER.c_str())) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            if (string(ent->d_name) != "." && string(ent->d_name) != "..") {
                fileList << ent->d_name << "\r\n";
            }
        }
        closedir(dir);
        return fileList.str();
    } else {
        return "550 Failed to list directory.\r\n";
    }
}

// Check if file exists in public folder
bool fileExists(const string& filename) {
    string filepath = PUBLIC_FOLDER + "/" + filename;
    ifstream file(filepath);
    return file.good();
}

// Get file size
long getFileSize(const string& filename) {
    string filepath = PUBLIC_FOLDER + "/" + filename;
    struct stat stat_buf;
    int rc = stat(filepath.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

// Handle client connection
void handleClient(int clientSocket) {
    char buffer[1024] = {0};
    
    // Send welcome message
    sendResponse(clientSocket, "220 Welcome to Simple FTP Server\r\n");
    
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(clientSocket, buffer, 1024);
        
        if (valread <= 0) {
            log("Client disconnected");
            break;
        }
        
        string command(buffer);
        command = command.substr(0, command.find('\r')); // Remove CRLF
        
        log("Received command: " + command);
        
        if (command == "QUIT") {
            sendResponse(clientSocket, "221 Goodbye\r\n");
            break;
        }
        else if (command == "LIST") {
            string files = listFiles();
            sendResponse(clientSocket, "150 Here comes the directory listing\r\n");
            sendResponse(clientSocket, files);
            sendResponse(clientSocket, "226 Directory send OK\r\n");
        }
        else if (command.substr(0, 4) == "RETR") {
            string filename = command.substr(5);
            if (fileExists(filename)) {
                string filepath = PUBLIC_FOLDER + "/" + filename;
                ifstream file(filepath, ios::binary);
                
                sendResponse(clientSocket, "150 Opening BINARY mode data connection for " + filename + "\r\n");
                
                // In a real FTP server, we'd establish a data connection here
                // For simplicity, we'll just send the file directly
                string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
                sendResponse(clientSocket, content);
                sendResponse(clientSocket, "226 Transfer complete\r\n");
            } else {
                sendResponse(clientSocket, "550 File not found\r\n");
            }
        }
        else if (command.substr(0, 4) == "SIZE") {
            string filename = command.substr(5);
            if (fileExists(filename)) {
                long size = getFileSize(filename);
                sendResponse(clientSocket, "213 " + to_string(size) + "\r\n");
            } else {
                sendResponse(clientSocket, "550 File not found\r\n");
            }
        }
        else {
            sendResponse(clientSocket, "502 Command not implemented\r\n");
        }
    }
    
    close(clientSocket);
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    // Create public folder if it doesn't exist
    mkdir(PUBLIC_FOLDER.c_str(), 0777);
    
    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }
    
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(FTP_PORT);
    
    // Bind socket
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen for connections
    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    log("FTP Server started on port " + to_string(FTP_PORT));
    log("Sharing files from: " + PUBLIC_FOLDER);
    
    vector<thread> clientThreads;
    
    while (true) {
        // Accept new connection
        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
            perror("Accept failed");
            continue;
        }
        
        log("New connection from " + string(inet_ntoa(clientAddr.sin_addr)));
        
        // Handle client in a new thread
        clientThreads.emplace_back(handleClient, clientSocket);
    }
    
    // Wait for all client threads to finish (though in this simple server, we never get here)
    for (auto& t : clientThreads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    close(serverSocket);
    return 0;
}
