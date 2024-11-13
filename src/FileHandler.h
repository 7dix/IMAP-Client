// FileHandler.h

#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <string>
#include <vector>
#include <map>

class FileHandler {
public:
    /**
     *  @brief Construct a new FileHandler object.
     * 
     * @param mailFolder Path to the folder containing email file structure.
     */
    FileHandler(std::string mailFolder);

    /**
     * @brief Saves the given message content to a file with a specified ID.
     *
     * Saves the message content to a file in the output directory.
     *
     * @param message_content The content of the message to be saved.
     * @param id The identifier used to generate the filename.
     * @param account The account name.
     * @param mailbox The mailbox name.
     * @throws FileException if the file cannot be opened or written to.
     */
    void saveMessage(const std::string &message_content, int id, std::string &account, std::string &mailbox);

    /**
     * @brief Checks if the message with the given ID has already been downloaded.
     * 
     * If only the header of the message has been downloaded, the message is not considered downloaded.
     * 
     * @param id The ID of the message to check.
     * @param account The account name.
     * @param mailbox The mailbox name.
     * @return int 
     * Returns: 0 if the message has not been downloaded, 1 if it has been downloaded, 2 if only headers were downloaded, and -1 on error.   
     */
    int isMessageAlreadyDownloaded(int id, std::string &account, std::string &mailbox);

private:
    std::string path; ///< Path to the folder containing email file structure.

    /**
     * @brief Creates directories in the given path.
     *
     * Creates directories in the given path if they do not already exist.
     *
     * @param fullPath The full path to create directories in.
     * @throws FileException if the directories cannot be created.
     */
    void createDirectories(const std::string& fullPath);
};

#endif // FILEHANDLER_H