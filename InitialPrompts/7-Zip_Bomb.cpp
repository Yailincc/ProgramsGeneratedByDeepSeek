#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <filesystem>
#include <zip.h>
#include <cstring>

namespace fs = std::filesystem;

// Function prototypes
void processZipFile(const std::string& zipPath, const std::string& outputDir, 
                    bool convertToUpper, const std::string& prefix, 
                    const std::string& suffix, bool countLines);
std::string toUpperCase(const std::string& input);
void countFileLines(const std::string& filePath);
void extractAndProcessFile(zip_t* zipArchive, zip_file_t* file, 
                          const std::string& fileName, const std::string& outputPath,
                          bool convertToUpper, const std::string& prefix, 
                          const std::string& suffix, bool countLines);

int main() {
    std::cout << "Zip Archive File Manipulator (C++ Version)" << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    // Get input zip file path
    std::string zipPath;
    while (true) {
        std::cout << "Enter path to the zip file: ";
        std::getline(std::cin, zipPath);
        
        if (fs::exists(zipPath) {
            if (zipPath.size() >= 4 && zipPath.substr(zipPath.size() - 4) == ".zip") {
                break;
            }
            std::cout << "File is not a .zip archive. Please try again." << std::endl;
        } else {
            std::cout << "File not found. Please try again." << std::endl;
        }
    }

    // Get output directory
    std::string outputDir = fs::current_path().string() + "/processed_output";
    std::cout << "Enter output directory (default: " << outputDir << "): ";
    std::string userOutputDir;
    std::getline(std::cin, userOutputDir);
    if (!userOutputDir.empty()) {
        outputDir = userOutputDir;
    }

    // Create output directory if it doesn't exist
    fs::create_directories(outputDir);

    // Select manipulations
    bool convertToUpper = false;
    std::string prefix;
    std::string suffix;
    bool countLines = false;

    std::cout << "\nAvailable manipulations:" << std::endl;
    std::cout << "1. Convert text files to uppercase" << std::endl;
    std::cout << "2. Add prefix to filenames" << std::endl;
    std::cout << "3. Add suffix to filenames" << std::endl;
    std::cout << "4. Count lines in text files" << std::endl;

    std::cout << "Enter your choices (space-separated, e.g. 1 3): ";
    std::string choices;
    std::getline(std::cin, choices);

    std::istringstream iss(choices);
    std::string choice;
    while (iss >> choice) {
        if (choice == "1") {
            convertToUpper = true;
        } else if (choice == "2") {
            std::cout << "Enter prefix to add: ";
            std::getline(std::cin, prefix);
        } else if (choice == "3") {
            std::cout << "Enter suffix to add: ";
            std::getline(std::cin, suffix);
        } else if (choice == "4") {
            countLines = true;
        }
    }

    // Process the zip file
    processZipFile(zipPath, outputDir, convertToUpper, prefix, suffix, countLines);

    std::cout << "Processing complete. Results saved in: " << outputDir << std::endl;
    return 0;
}

void processZipFile(const std::string& zipPath, const std::string& outputDir, 
                   bool convertToUpper, const std::string& prefix, 
                   const std::string& suffix, bool countLines) {
    int err = 0;
    zip_t* zipArchive = zip_open(zipPath.c_str(), ZIP_RDONLY, &err);
    if (!zipArchive) {
        std::cerr << "Failed to open zip file: " << zipPath << std::endl;
        return;
    }

    zip_int64_t numEntries = zip_get_num_entries(zipArchive, 0);
    for (zip_int64_t i = 0; i < numEntries; i++) {
        const char* name = zip_get_name(zipArchive, i, 0);
        if (!name) {
            std::cerr << "Error getting name for entry " << i << std::endl;
            continue;
        }

        // Skip directory entries
        if (name[strlen(name) - 1] == '/') {
            continue;
        }

        zip_file_t* file = zip_fopen_index(zipArchive, i, 0);
        if (!file) {
            std::cerr << "Error opening file " << name << " in zip archive" << std::endl;
            continue;
        }

        // Create output path
        std::string fileName = name;
        std::string outputPath = outputDir + "/" + fileName;

        // Create directories if needed
        fs::path filePath(outputPath);
        fs::create_directories(filePath.parent_path());

        extractAndProcessFile(zipArchive, file, fileName, outputPath, 
                            convertToUpper, prefix, suffix, countLines);

        zip_fclose(file);
    }

    zip_close(zipArchive);
}

void extractAndProcessFile(zip_t* zipArchive, zip_file_t* file, 
                         const std::string& fileName, const std::string& outputPath,
                         bool convertToUpper, const std::string& prefix, 
                         const std::string& suffix, bool countLines) {
    // Get file stats
    zip_stat_t stats;
    if (zip_stat_index(zipArchive, zip_file_get_index(file, 0), 0, &stats) != 0) {
        std::cerr << "Error getting stats for file " << fileName << std::endl;
        return;
    }

    // Read file contents
    std::vector<char> contents(stats.size);
    zip_int64_t bytesRead = zip_fread(file, contents.data(), stats.size);
    if (bytesRead < 0) {
        std::cerr << "Error reading file " << fileName << std::endl;
        return;
    }

    // Determine final output path with prefix/suffix
    fs::path originalPath(outputPath);
    std::string newFilename = originalPath.filename().string();
    
    if (!prefix.empty()) {
        newFilename = prefix + newFilename;
    }
    
    if (!suffix.empty()) {
        size_t dotPos = newFilename.find_last_of('.');
        if (dotPos != std::string::npos) {
            newFilename.insert(dotPos, suffix);
        } else {
            newFilename += suffix;
        }
    }
    
    fs::path finalPath = originalPath.parent_path() / newFilename;

    // Check if we need to process as text (for uppercase conversion and line counting)
    bool isTextFile = false;
    std::string extension = originalPath.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    if (extension == ".txt" || extension == ".csv" || extension == ".xml" || 
        extension == ".json" || extension == ".cpp" || extension == ".h" || 
        extension == ".java" || extension == ".py") {
        isTextFile = true;
    }

    // Write file with possible modifications
    std::ofstream outFile(finalPath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error creating output file " << finalPath << std::endl;
        return;
    }

    if (convertToUpper && isTextFile) {
        std::string textContent(contents.begin(), contents.end());
        std::string upperContent = toUpperCase(textContent);
        outFile.write(upperContent.c_str(), upperContent.size());
        std::cout << "Converted to uppercase: " << fileName << std::endl;
    } else {
        outFile.write(contents.data(), contents.size());
    }

    outFile.close();

    // Count lines if requested
    if (countLines && isTextFile) {
        countFileLines(finalPath.string());
    }

    // Print prefix/suffix messages if applied
    if (!prefix.empty()) {
        std::cout << "Added prefix '" << prefix << "': " << fileName << std::endl;
    }
    if (!suffix.empty()) {
        std::cout << "Added suffix '" << suffix << "': " << fileName << std::endl;
    }
}

std::string toUpperCase(const std::string& input) {
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(), 
                  [](unsigned char c) { return std::toupper(c); });
    return result;
}

void countFileLines(const std::string& filePath) {
    std::ifstream inFile(filePath);
    if (!inFile) {
        std::cerr << "Error opening file for line counting: " << filePath << std::endl;
        return;
    }

    int lineCount = 0;
    std::string line;
    while (std::getline(inFile, line)) {
        lineCount++;
    }

    std::cout << "Line count for " << filePath << ": " << lineCount << std::endl;
}
