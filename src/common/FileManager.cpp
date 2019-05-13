#include <cerrno>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../../include/common/FileManager.h"

int FileManager_createDir(char* name) {
    // Creating a directory 
    if (mkdir(name, 0777) == -1) {
		if(errno != EEXIST) {
	        std::cerr << "FileManager_createDir(): ERROR " << std::strerror(errno) << std::endl; 
    	    return -1;
		}
	}
	else
		return 1;
}

int FileManager_createFile(char* name);
int FileManager_openFile(char* name);
int FileManager_removeFile(char* name);
int FileManager_renameFile(char* oldname, char* newname);
int FileManager_getFileSize(int fd);
int FileManager_readFile(int fd, char* buffer);
int FileManager_writeFile(int fd, char* buffer);


