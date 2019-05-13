int FileManager_createDir(char* name);
int FileManager_openDir(char* name);
int FileManager_createFile(char* name);
int FileManager_openFile(char* name);
int FileManager_removeFile(char* name);
int FileManager_renameFile(char* oldname, char* newname);
int FileManager_getFileSize(char* name);
int FileManager_readFile(int fd, char* buffer);
int FileManager_writeFile(int fd, char* buffer);


