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
#include <algorithm>
#include <limits>
#include <openssl/sha.h>

using namespace std;

// Configuration
const int FTP_PORT = 2121;
const string PUBLIC_FOLDER = "./public";
const int MAX_CLIENTS = 10;
const size_t MAX_COMMAND_LENGTH = 512;
const size_t MAX_FILE_SIZE = 100 * 1024 * 1024; // 100MB
const unsigned int SESSION_TIMEOUT = 300; // 5 minutes in seconds

// Security constants
const vector<string> ALLOWED_COMMANDS = {"USER", "PASS", "QUIT", "LIST", "RETR", "SIZE"};
const string ALLOWED_CHARS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_.";

mutex logMutex;

struct ClientSession {
    int socket;
    string clientIP;
    bool authenticated;
    time_t lastActivity;
    string username;
};

// Secure logging function
void log(const string& message, const ClientSession* session = nullptr) {
    lock_guard<mutex> guard(logMutex);
    string logEntry = "[Server] ";
    if (session) {
        logEntry += "[" + session->clientIP + "] ";
        if (!session->username.empty()) {
            logEntry += "[" + session->username + "] ";
        }
    }
    logEntry += message;
    cout << logEntry << endl;
}

// Secure send function with error handling
bool sendResponse(ClientSession& session, const string& response) {
    if (send(session.socket, response.c_str(), response.size(), 0) < 0) {
        log("Failed to send response to client", &session);
        return false;
    }
    session.lastActivity = time(nullptr);
    return true;
}

// Validate filename security
bool isValidFilename(const string& filename) {
    if (filename.empty() || filename.length() > 255) return false;
    if (filename.find("..") != string::npos) return false;
    
    return all_of(filename.begin(), filename.end(), [](char c) {
        return ALLOWED_CHARS.find(c) != string::npos;
    });
}

// Secure path construction
string getSecurePath(const string& filename) {
    if (!isValidFilename(filename)) {
        throw runtime_error("Invalid filename");
    }
    return PUBLIC_FOLDER + "/" + filename;
}

// List files securely
string listFilesSecure() {
    DIR *dir;
    struct dirent *ent;
    stringstream fileList;
    
    if ((dir = opendir(PUBLIC_FOLDER.c_str())) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            string filename(ent->d_name);
            if (filename != "." && filename != ".." && isValidFilename(filename)) {
                fileList << filename << "\r\n";
            }
        }
        closedir(dir);
        return fileList.str();
    }
    return "550 Failed to list directory.\r\n";
}

// Get file size with security checks
long getSecureFileSize(const string& filename) {
    try {
        string filepath = getSecurePath(filename);
        struct stat stat_buf;
        
        if (stat(filepath.c_str(), &stat_buf) != 0) return -1;
        if (!S_ISREG(stat_buf.st_mode)) return -1; // Not a regular file
        if (stat_buf.st_size > MAX_FILE_SIZE) return -2; // File too large
        
        return stat_buf.st_size;
    } catch (...) {
        return -1;
    }
}

// Simple authentication (in real implementation, use proper password hashing)
bool authenticate(const string& user, const string& pass) {
    // This is just an example - store credentials securely in production
    return (user == "ftpuser" && pass == "2a10$N9qo8uLOickgx2ZMRZoMy..."); // Example hashed password
}

// Handle client connection securely
void handleClientSecure(ClientSession session) {
    char buffer[MAX_COMMAND_LENGTH + 1] = {0};
    
    sendResponse(session, "220 Welcome to Secure FTP Server\r\n");
    
    while (true) {
        // Check session timeout
        if (difftime(time(nullptr), session.lastActivity) > SESSION_TIMEOUT) {
            sendResponse(session, "421 Session timeout\r\n");
            log("Session timed out", &session);
            break;
        }
        
        memset(buffer, 0, sizeof(buffer));
        int valread = recv(session.socket, buffer, MAX_COMMAND_LENGTH, 0);
        
        if (valread <= 0) {
            log(valread == 0 ? "Client disconnected" : "Read error", &session);
            break;
        }
        
        string command(buffer);
        command = command.substr(0, command.find('\r')); // Remove CRLF
        
        // Basic command validation
        if (command.length() > MAX_COMMAND_LENGTH) {
            sendResponse(session, "500 Command too long\r\n");
            continue;
        }
        
        string cmd = command.substr(0, command.find(' '));
        transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
        
        // Check if command is allowed
        if (find(ALLOWED_COMMANDS.begin(), ALLOWED_COMMANDS.end(), cmd) == ALLOWED_COMMANDS.end()) {
            sendResponse(session, "500 Unknown command\r\n");
            continue;
        }
        
        log("Command: " + command, &session);
        
        if (cmd == "QUIT") {
            sendResponse(session, "221 Goodbye\r\n");
            break;
        }
        else if (cmd == "USER") {
            if (command.length() > 5) {
                session.username = command.substr(5);
                sendResponse(session, "331 Password required\r\n");
            } else {
                sendResponse(session, "501 Syntax error in parameters\r\n");
            }
        }
        else if (cmd == "PASS") {
            if (session.username.empty()) {
                sendResponse(session, "503 Login with USER first\r\n");
            } else if (command.length() > 5) {
                string password = command.substr(5);
                if (authenticate(session.username, password)) {
                    session.authenticated = true;
                    sendResponse(session, "230 Login successful\r\n");
                    log("User authenticated", &session);
                } else {
                    sendResponse(session, "530 Login incorrect\r\n");
                    log("Failed authentication attempt", &session);
                }
            } else {
                sendResponse(session, "501 Syntax error in parameters\r\n");
            }
        }
        else if (!session.authenticated) {
            sendResponse(session, "530 Not logged in\r\n");
        }
        else if (cmd == "LIST") {
            string files = listFilesSecure();
            sendResponse(session, "150 Here comes the directory listing\r\n");
            sendResponse(session, files);
            sendResponse(session, "226 Directory send OK\r\n");
        }
        else if (cmd == "RETR") {
            if (command.length() > 5) {
                string filename = command.substr(5);
                try {
                    string filepath = getSecurePath(filename);
                    long fileSize = getSecureFileSize(filename);
                    
                    if (fileSize == -1) {
                        sendResponse(session, "550 File not found or access denied\r\n");
                    } else if (fileSize == -2) {
                        sendResponse(session, "552 Requested file size exceeds limit\r\n");
                    } else {
                        ifstream file(filepath, ios::binary);
                        if (file) {
                            sendResponse(session, "150 Opening BINARY mode data connection for " + filename + "\r\n");
                            
                            // Stream file in chunks to avoid memory issues
                            vector<char> buffer(1024 * 1024); // 1MB buffer
                            while (file.read(buffer.data(), buffer.size())) {
                                if (send(session.socket, buffer.data(), file.gcount(), 0) < 0) {
                                    throw runtime_error("Send failed");
                                }
                            }
                            // Send remaining data
                            if (file.gcount() > 0) {
                                send(session.socket, buffer.data(), file.gcount(), 0);
                            }
                            
                            sendResponse(session, "226 Transfer complete\r\n");
                        } else {
                            sendResponse(session, "550 Could not open file\r\n");
                        }
                    }
                } catch (const exception& e) {
                    sendResponse(session, "550 Access denied\r\n");
                    log(string("File access denied: ") + e.what(), &session);
                }
            } else {
                sendResponse(session, "501 Syntax error in parameters\r\n");
            }
        }
        else if (cmd == "SIZE") {
            if (command.length() > 5) {
                string filename = command.substr(5);
                try {
                    long size = getSecureFileSize(filename);
                    if (size >= 0) {
                        sendResponse(session, "213 " + to_string(size) + "\r\n");
                    } else if (size == -2) {
                        sendResponse(session, "552 File too large\r\n");
                    } else {
                        sendResponse(session, "550 File not found or access denied\r\n");
                    }
                } catch (...) {
                    sendResponse(session, "550 Access denied\r\n");
                }
            } else {
                sendResponse(session, "501 Syntax error in parameters\r\n");
            }
        }
    }
    
    close(session.socket);
    log("Connection closed", &session);
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    // Create public folder with secure permissions
    mkdir(PUBLIC_FOLDER.c_str(), 0755); // Restrictive permissions
    
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
    
    // Set receive timeout
    struct timeval tv;
    tv.tv_sec = 30;
    tv.tv_usec = 0;
    setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    
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
    
    log("Secure FTP Server started on port " + to_string(FTP_PORT));
    log("Sharing files from: " + PUBLIC_FOLDER);
    
    vector<thread> clientThreads;
    
    while (true) {
        // Accept new connection
        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
            perror("Accept failed");
            continue;
        }
        
        ClientSession session;
        session.socket = clientSocket;
        session.clientIP = inet_ntoa(clientAddr.sin_addr);
        session.authenticated = false;
        session.lastActivity = time(nullptr);
        
        log("New connection", &session);
        
        // Handle client in a new thread
        clientThreads.emplace_back(handleClientSecure, session);
        
        // Clean up finished threads
        for (auto it = clientThreads.begin(); it != clientThreads.end(); ) {
            if (it->joinable()) {
                it->join();
                it = clientThreads.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    close(serverSocket);
    return 0;
}
