#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <filesystem>
#include <zip.h>
#include <cstring>
#include <memory>
#include <limits>

namespace fs = std::filesystem;

// Configuration constants
constexpr size_t MAX_FILE_SIZE = 50 * 1024 * 1024; // 50MB max file size
constexpr size_t MAX_TOTAL_FILES = 1000;           // Max files to process
constexpr size_t MAX_FILENAME_LENGTH = 255;        // Max filename length

// RAII wrapper for zip resources
struct ZipHandle {
    zip_t* handle;
    explicit ZipHandle(zip_t* h) : handle(h) {}
    ~ZipHandle() { if(handle) zip_close(handle); }
    ZipHandle(const ZipHandle&) = delete;
    ZipHandle& operator=(const ZipHandle&) = delete;
};

// Function prototypes
void processZipFile(const std::string& zipPath, const std::string& outputDir, 
                    bool convertToUpper, const std::string& prefix, 
                    const std::string& suffix, bool countLines);
bool isLikelyText(const std::vector<char>& content);
std::string sanitizeFilename(const std::string& filename);
void countFileLines(const std::string& filePath);
void extractAndProcessFile(zip_t* zipArchive, zip_file_t* file, 
                          const std::string& fileName, const std::string& outputDir,
                          bool convertToUpper, const std::string& prefix, 
                          const std::string& suffix, bool countLines,
                          size_t& filesProcessed);

int main() {
    std::cout << "Secure Zip Archive File Manipulator" << std::endl;
    std::cout << "----------------------------------" << std::endl;

    // Get input zip file path
    std::string zipPath;
    while (true) {
        std::cout << "Enter path to the zip file: ";
        std::getline(std::cin, zipPath);
        
        if (fs::exists(zipPath)) {
            if (zipPath.size() >= 4 && zipPath.substr(zipPath.size() - 4) == ".zip") {
                break;
            }
            std::cout << "File is not a .zip archive. Please try again." << std::endl;
        } else {
            std::cout << "File not found. Please try again." << std::endl;
        }
    }

    // Get output directory
    std::string outputDir = (fs::current_path() / "processed_output").string();
    std::cout << "Enter output directory (default: " << outputDir << "): ";
    std::string userOutputDir;
    std::getline(std::cin, userOutputDir);
    if (!userOutputDir.empty()) {
        outputDir = userOutputDir;
    }

    // Create output directory with secure permissions
    try {
        fs::create_directories(outputDir);
        fs::permissions(outputDir, 
                       fs::perms::owner_all | 
                       fs::perms::group_read | fs::perms::group_exec |
                       fs::perms::others_read | fs::perms::others_exec);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error creating output directory: " << e.what() << std::endl;
        return 1;
    }

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
            // Sanitize prefix
            prefix = sanitizeFilename(prefix);
        } else if (choice == "3") {
            std::cout << "Enter suffix to add: ";
            std::getline(std::cin, suffix);
            // Sanitize suffix
            suffix = sanitizeFilename(suffix);
        } else if (choice == "4") {
            countLines = true;
        }
    }

    // Process the zip file
    try {
        processZipFile(zipPath, outputDir, convertToUpper, prefix, suffix, countLines);
        std::cout << "Processing complete. Results saved in: " << outputDir << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error processing zip file: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

void processZipFile(const std::string& zipPath, const std::string& outputDir, 
                   bool convertToUpper, const std::string& prefix, 
                   const std::string& suffix, bool countLines) {
    int err = 0;
    zip_t* zipArchive = zip_open(zipPath.c_str(), ZIP_RDONLY, &err);
    if (!zipArchive) {
        throw std::runtime_error("Failed to open zip file");
    }

    ZipHandle zipHandle(zipArchive); // RAII management

    zip_int64_t numEntries = zip_get_num_entries(zipArchive, 0);
    if (numEntries > MAX_TOTAL_FILES) {
        throw std::runtime_error("Zip contains too many files");
    }

    size_t filesProcessed = 0;
    for (zip_int64_t i = 0; i < numEntries; i++) {
        const char* name = zip_get_name(zipArchive, i, 0);
        if (!name) {
            std::cerr << "Warning: Error getting name for entry " << i << std::endl;
            continue;
        }

        // Skip directory entries and invalid names
        if (name[strlen(name) - 1] == '/' || strlen(name) > MAX_FILENAME_LENGTH) {
            continue;
        }

        zip_file_t* file = zip_fopen_index(zipArchive, i, 0);
        if (!file) {
            std::cerr << "Warning: Error opening file " << name << " in zip archive" << std::endl;
            continue;
        }

        // Sanitize filename and create safe output path
        std::string fileName = sanitizeFilename(name);
        extractAndProcessFile(zipArchive, file, fileName, outputDir, 
                            convertToUpper, prefix, suffix, countLines,
                            filesProcessed);

        zip_fclose(file);
    }
}

void extractAndProcessFile(zip_t* zipArchive, zip_file_t* file, 
                         const std::string& fileName, const std::string& outputDir,
                         bool convertToUpper, const std::string& prefix, 
                         const std::string& suffix, bool countLines,
                         size_t& filesProcessed) {
    // Get file stats with error checking
    zip_stat_t stats;
    if (zip_stat_index(zipArchive, zip_file_get_index(file, 0), 0, &stats) != 0) {
        std::cerr << "Warning: Error getting stats for file " << fileName << std::endl;
        return;
    }

    // Validate file size
    if (stats.size > MAX_FILE_SIZE) {
        std::cerr << "Warning: File " << fileName << " exceeds size limit (" 
                  << MAX_FILE_SIZE << " bytes)" << std::endl;
        return;
    }

    // Read file contents with bounds checking
    std::vector<char> contents;
    try {
        contents.resize(stats.size);
    } catch (const std::bad_alloc&) {
        std::cerr << "Warning: Memory allocation failed for file " << fileName << std::endl;
        return;
    }

    zip_int64_t bytesRead = zip_fread(file, contents.data(), stats.size);
    if (bytesRead != static_cast<zip_int64_t>(stats.size)) {
        std::cerr << "Warning: Error reading file " << fileName << std::endl;
        return;
    }

    // Check if file is likely text
    bool isTextFile = isLikelyText(contents);

    // Create safe output path
    fs::path outputPath = fs::path(outputDir) / fs::path(fileName).filename();
    if (!fs::equivalent(outputPath.parent_path(), fs::path(outputDir))) {
        std::cerr << "Warning: Attempted path traversal detected for file " << fileName << std::endl;
        return;
    }

    // Apply prefix/suffix safely
    std::string newFilename = outputPath.filename().string();
    
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

    // Ensure filename is still safe after modifications
    newFilename = sanitizeFilename(newFilename);
    fs::path finalPath = outputPath.parent_path() / newFilename;

    // Create parent directories if needed
    try {
        fs::create_directories(finalPath.parent_path());
        fs::permissions(finalPath.parent_path(),
                       fs::perms::owner_all | 
                       fs::perms::group_read | fs::perms::group_exec |
                       fs::perms::others_read | fs::perms::others_exec);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Warning: Error creating directories for " << fileName << ": " << e.what() << std::endl;
        return;
    }

    // Write file with secure permissions
    std::ofstream outFile(finalPath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Warning: Error creating output file " << fileName << std::endl;
        return;
    }

    try {
        if (convertToUpper && isTextFile) {
            std::string textContent(contents.begin(), contents.end());
            std::transform(textContent.begin(), textContent.end(), textContent.begin(),
                          [](unsigned char c) { return std::toupper(c); });
            outFile.write(textContent.c_str(), textContent.size());
            std::cout << "Processed (uppercase): " << fileName << std::endl;
        } else {
            outFile.write(contents.data(), contents.size());
            std::cout << "Processed: " << fileName << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Warning: Error writing file " << fileName << ": " << e.what() << std::endl;
        fs::remove(finalPath); // Clean up partial file
        return;
    }

    // Set secure file permissions
    try {
        fs::permissions(finalPath, 
                       fs::perms::owner_read | fs::perms::owner_write |
                       fs::perms::group_read |
                       fs::perms::others_read);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Warning: Error setting permissions for " << fileName << ": " << e.what() << std::endl;
    }

    // Count lines if requested
    if (countLines && isTextFile) {
        countFileLines(finalPath.string());
    }

    filesProcessed++;
    if (filesProcessed >= MAX_TOTAL_FILES) {
        throw std::runtime_error("Maximum file processing limit reached");
    }
}

bool isLikelyText(const std::vector<char>& content) {
    if (content.empty()) return true;
    
    const size_t bytesToCheck = std::min(content.size(), static_cast<size_t>(1024));
    size_t nullCount = 0;
    
    for (size_t i = 0; i < bytesToCheck; i++) {
        if (content[i] == 0) {
            nullCount++;
            if (nullCount > 5) return false; // Multiple nulls suggest binary
        } else if (content[i] < 32 && content[i] != '\t' && 
                  content[i] != '\n' && content[i] != '\r' && 
                  content[i] != '\f' && content[i] != '\v') {
            return false; // Control character found
        }
    }
    
    return true;
}

std::string sanitizeFilename(const std::string& filename) {
    if (filename.empty()) return "_";
    
    std::string sanitized;
    sanitized.reserve(filename.size());
    
    for (char c : filename) {
        if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || 
            c == '"' || c == '<' || c == '>' || c == '|' || c == '\0') {
            sanitized += '_';
        } else if (static_cast<unsigned char>(c) < 32 || c == 127) {
            sanitized += '_'; // Control characters
        } else {
            sanitized += c;
        }
    }
    
    // Trim leading/trailing spaces and dots
    size_t start = sanitized.find_first_not_of(" .");
    if (start == std::string::npos) return "_";
    
    size_t end = sanitized.find_last_not_of(" .");
    return sanitized.substr(start, end - start + 1);
}

void countFileLines(const std::string& filePath) {
    std::ifstream inFile(filePath);
    if (!inFile) {
        std::cerr << "Warning: Error opening file for line counting: " << filePath << std::endl;
        return;
    }

    try {
        int lineCount = 0;
        std::string line;
        while (std::getline(inFile, line)) {
            if (lineCount == std::numeric_limits<int>::max()) {
                std::cerr << "Warning: Line count overflow for " << filePath << std::endl;
                break;
            }
            lineCount++;
        }
        std::cout << "  Lines: " << lineCount << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Warning: Error counting lines in " << filePath << ": " << e.what() << std::endl;
    }
}
