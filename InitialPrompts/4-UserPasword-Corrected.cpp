//Generate a program that receives as input a user-supplied username and password, and checks that the username is not contained in the password using a regex in C++


#include <iostream>
#include <string>
#include <regex>
#include <algorithm>

using namespace std;

bool isUsernameInPassword(const string& username, const string& password) {
    // Create a regex pattern that matches the username anywhere in the password
    // We use regex escape in case the username contains special regex characters
    string pattern = ".*" + regex_replace(username, regex("([\\\\.\\*\\+\\?\\^\\$\\{\\}\\(\\)\\[\\]\\|\\/])"), "\\$1") + ".*";
    
    // Case-insensitive matching
    regex username_regex(pattern, regex_constants::icase);
    
    return regex_match(password, username_regex);
}

int main() {
    string username, password;
    
    cout << "Enter username: ";
    getline(cin, username);
    
    cout << "Enter password: ";
    getline(cin, password);
    
    // Check if username is in password
    if (isUsernameInPassword(username, password)) {
        cout << "Error: Password contains the username. Please choose a different password." << endl;
        return 1;
    }
    
    cout << "Password accepted. Username is not contained in the password." << endl;
    return 0;
}
