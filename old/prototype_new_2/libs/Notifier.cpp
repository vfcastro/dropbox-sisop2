#include "inotify.h"

class Notifier{
public:
    Notifier(){};
    char dirName[10] = "SyncDir";

    int notify(){
        inotifyFunc(dirName);
    }

private:
protected:
};