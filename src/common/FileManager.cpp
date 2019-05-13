#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int FileManager_createDir(char* name);
int FileManager_createFile(char* name);
int FileManager_openFile(char* name);
int FileManager_removeFile(char* name);
int FileManager_renameFile(char* oldname, char* newname);
int FileManager_getFileSize(int fd);
int FileManager_readFile(int fd, char* buffer);
int FileManager_writeFile(int fd, char* buffer);


