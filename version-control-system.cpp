#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <functional>
#include <vector>
#include <unordered_set>

bool InitializingRepository(const std::string& repositoryPath) {
    std::string command = "mkdir " + repositoryPath;
    int result = system(command.c_str());

    if (result == 0) {
        std::cout << "Repository initialized in: " << repositoryPath << std::endl;
        return true;
    } else {
        std::cerr << "Error: Repository initialization failed.\n";
        return false;
    }
}

bool AddFile(const std::string& repositoryPath, const std::string& fileName) {
    std::ofstream file(repositoryPath + "/" + fileName);

    if (file.is_open()) {
        file << "This is a simple content for " << fileName << std::endl;
        file.close();
        std::cout << "File added to repository: " << fileName << std::endl;
        return true;
    } else {
        std::cerr << "Error: Unable to create file " << fileName << std::endl;
        return false;
    }
}

std::string CalculateFileHash(const std::string& filePath) {
    std::ifstream file(filePath);

    if (file.is_open()) {
        std::ostringstream content;
        content << file.rdbuf();
        file.close();

        // Simple hash using std::hash
        std::hash<std::string> hasher;
        return std::to_string(hasher(content.str()));
    } else {
        std::cerr << "Error: Unable to open file " << filePath << " for hashing.\n";
        return "";
    }
}

void DisplayTrackedFiles(const std::string& repositoryPath) {
    std::cout << "Tracked files in the repository:\n";
    std::string command = "ls " + repositoryPath;
    system(command.c_str());
}

bool CommitChanges(const std::string& repositoryPath, const std::string& fileName) {
    std::string filePath = repositoryPath + "/" + fileName;
    std::ifstream file(filePath);

    if (file.is_open()) {

        std::string newContent = "Updated content for " + fileName;
        file.close();

        std::ofstream newFile(filePath);
        if (newFile.is_open()) {
            newFile << newContent << std::endl;
            newFile.close();

            std::string newHash = CalculateFileHash(filePath);

            std::ofstream logFile(repositoryPath + "/commit_log.txt", std::ios::app);
            if (logFile.is_open()) {
                auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                logFile << "[" << std::put_time(std::localtime(&now), "%F %T") << "] "
                        << "Commit: Updated " << fileName << " (Hash: " << newHash << ")\n";
                logFile.close();
            } else {
                std::cerr << "Error: Unable to open commit log file.\n";
            }

            std::cout << "Changes committed for file: " << fileName << std::endl;
            return true;
        } else {
            std::cerr << "Error: Unable to commit changes for file " << fileName << std::endl;
            return false;
        }
    } else {
        std::cerr << "Error: Unable to open file " << fileName << std::endl;
        return false;
    }
}

void ViewCommitLog(const std::string& repositoryPath) {
    std::string logFilePath = repositoryPath + "/commit_log.txt";
    std::ifstream logFile(logFilePath);

    if (logFile.is_open()) {
        std::cout << "Commit log:\n";
        std::cout << logFile.rdbuf();
        logFile.close();
    } else {
        std::cerr << "Error: Unable to open commit log file.\n";
    }
}

bool RevertFile(const std::string& repositoryPath, const std::string& fileName) {
    std::string logFilePath = repositoryPath + "/commit_log.txt";
    std::ifstream logFile(logFilePath);
    std::vector<std::string> commits;

    if (logFile.is_open()) {
        std::string line;
        while (std::getline(logFile, line)) {
            commits.push_back(line);
        }
        logFile.close();
    } else {
        std::cerr << "Error: Unable to open commit log file.\n";
        return false;
    }

    if (commits.size() < 2) {
        std::cerr << "Error: No previous versions available for file " << fileName << std::endl;
        return false;
    }

    std::string previousCommit = commits[commits.size() - 2];
    std::istringstream iss(previousCommit);
    std::string timestamp, action, file;

    iss >> timestamp >> action >> file;

    if (action != "Commit:" || file != "Updated" + fileName) {
        std::cerr << "Error: Unable to revert file " << fileName << ". Previous version not found in commit log.\n";
        return false;
    }


    std::string previousHash = "";
    for (size_t i = 0; i < commits.size() - 1; ++i) {
        std::istringstream commitStream(commits[i]);
        std::string commitTimestamp, commitAction, commitFile, commitHash;

        commitStream >> commitTimestamp >> commitAction >> commitFile >> commitHash;

        if (commitAction == "Commit:" && commitFile == "Updated" + fileName) {
            previousHash = commitHash;
            break;
        }
    }

    if (previousHash.empty()) {
        std::cerr << "Error: Unable to find hash of the previous version for file " << fileName << std::endl;
        return false;
    }

    std::string filePath = repositoryPath + "/" + fileName;
    std::string currentHash = CalculateFileHash(filePath);

    if (currentHash == previousHash) {
        std::ofstream revertedFile(filePath);
        if (revertedFile.is_open()) {
            revertedFile << "This is a simple content for " << fileName << std::endl;
            revertedFile.close();
            std::cout << "File reverted to the previous version.\n";
            return true;
        } else {
            std::cerr << "Error: Unable to revert file " << fileName << std::endl;
            return false;
        }
    } else {
        std::cerr << "Error: Unable to revert file " << fileName << ". Hash mismatch.\n";
        return false;
    }
}

bool CheckIntegrity(const std::string& repositoryPath) {
    std::string logFilePath = repositoryPath + "/commit_log.txt";
    std::ifstream logFile(logFilePath);

    if (logFile.is_open()) {
        std::vector<std::string> commits;
        std::unordered_set<std::string> commitSet;

        std::string line;
        while (std::getline(logFile, line)) {
            commits.push_back(line);
            commitSet.insert(line);
        }
        logFile.close();

       
        if (commits.size() != commitSet.size()) {
            std::cerr << "Error: Duplicate records found in the commit log.\n";
            return false;
        }

        std::hash<std::string> hasher;
        size_t checksum = 0;
        for (const auto& commit : commits) {
            checksum ^= hasher(commit);
        }

        if (checksum == 0) {
            std::cout << "Integrity check passed. No changes in commit log contents.\n";
            return true;
        } else {
            std::cerr << "Error: Integrity check failed. Commit log contents have changed.\n";
            return false;
        }
    } else {
        std::cerr << "Error: Unable to open commit log file for integrity check.\n";
        return false;
    }
}

int main() {
    std::string repositoryPath;
    std::cout << "Enter the name of the repository: ";
    std::cin >> repositoryPath;

    if (InitializingRepository(repositoryPath)) {
        std::cout << "Additional initialization steps completed.\n";
    }

    std::string newFileName;
    std::cout << "Enter the name of the file to add: ";
    std::cin >> newFileName;

    if (AddFile(repositoryPath, newFileName)) {
        std::cout << "File addition completed.\n";
    } else {
        std::cerr << "File addition failed.\n";
        return 1;
    }

    DisplayTrackedFiles(repositoryPath);

    std::string fileToCommit;
    std::cout << "Enter the name of the file to commit changes: ";
    std::cin >> fileToCommit;

    if (CommitChanges(repositoryPath, fileToCommit)) {
        std::cout << "Changes committed successfully.\n";
    } else {
        std::cerr << "Error: Unable to commit changes.\n";
        return 1;
    }


    ViewCommitLog(repositoryPath);


    std::string fileToRevert;
    std::cout << "Enter the name of the file to revert: ";
    std::cin >> fileToRevert;

    if (RevertFile(repositoryPath, fileToRevert)) {
        std::cout << "File reverted successfully.\n";
    } else {
        std::cerr << "Error: Unable to revert file.\n";
        return 1; 
    }

    
    if (CheckIntegrity(repositoryPath)) {
        std::cout << "Commit log integrity check passed.\n";
    } else {
        std::cerr << "Error: Commit log integrity check failed.\n";
        return 1;
    }

    return 0;
}