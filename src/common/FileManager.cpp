#include <dirent.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <experimental/filesystem>
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


int FileManager_openDir(char* name) {
	DIR* dir = opendir(name);
	if(dir) {
		closedir(dir);
		return 1;
	}
	else
		return -1;
}

int FileManager_removeFile(std::string name){
}
int FileManager_renameFile(char* oldname, char* newname);

int FileManager_getFileSize(char* name) {
	struct stat st;
	if(stat(name,&st) == -1){
		std::cerr<<"FileManager_getFileSize("<<name<<") ERROR \n";
		return -1;
	}
	else
		return st.st_size;
}

int FileManager_readFile(int fd, char* buffer);
int FileManager_writeFile(int fd, char* buffer);
int FileManager_createFile(char* name);
