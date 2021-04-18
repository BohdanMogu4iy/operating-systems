#include <string.h>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

struct FileInfo{
    string name;
    mode_t mode;
    string uid;
    string gid;
    off_t size;
    time_t mtim;

   void Print(){

        cout << "---------------------------------------------------------\n";

        cout << " File name : " << name.c_str() << endl;

        cout << " Is file a folder : " << ((S_ISDIR(mode)) ? "true" : "--") << endl;

        cout << " Owner may read : " << ((mode & S_IRUSR) ? "true" : "--") << endl;

        cout << " Owner may rewrite : " << ((mode & S_IWUSR) ? "true" : "-") << endl;

        cout << " Owner may run : " << ((mode & S_IXUSR) ? "true" : "--") << endl;

        cout << " Group may read : " << ((mode & S_IRGRP) ? "true" : "--") << endl;

        cout << " Group may rewrite : " << ((mode & S_IWGRP) ? "true" : "--") << endl;

        cout << " Group may run : " << ((mode & S_IXGRP) ? "true" : "--") << endl;

        cout << " All the other may read : " << ((mode & S_IROTH) ? "true" : "--") << endl;

        cout << " All the other may rewrite : " << ((mode & S_IWOTH) ? "true" : "--") << endl;

        cout << " All the other may run : " << ((mode & S_IXOTH) ? "true" : "--") << endl;

        cout << " File user : " << uid.c_str() << endl;

        cout << " File group : "  << gid.c_str() << endl;

        cout << " File size : " << (int)size << endl;

        cout << " Change time : " <<  ctime (&mtim) << endl;

        

    }

};

void FindFiles(char dir[256], vector<FileInfo>& files)
{
    DIR *cdir;
    dirent *direntry;
    struct stat fileinfo;
    chdir(dir);
    cdir = opendir(dir);
    if(cdir!=NULL)
    {
        while((direntry = readdir(cdir)) != NULL)
        {
            if(lstat(direntry->d_name,&fileinfo)!=0){
                continue;
            }
            FileInfo tempDir;
            tempDir.name = direntry->d_name;
            tempDir.mode = fileinfo.st_mode;
            struct passwd *pw = getpwuid(fileinfo.st_uid);
            tempDir.uid = string(pw->pw_name);
            struct group  *gr = getgrgid(fileinfo.st_gid);
            tempDir.gid = string(gr->gr_name);
            tempDir.size = fileinfo.st_size;
            tempDir.mtim = fileinfo.st_mtime;

            files.push_back(tempDir);
        }
    }
}

void PrintAll(vector<FileInfo>& files){
    for (auto& i : files){
        i.Print();
    }
}

int main(int argc, char* argv[]){
    vector<FileInfo> names;
    char directory[256];

    cout << "Write path to directory" << endl;
    cin >> directory;

    FindFiles((char *)directory, names);
    PrintAll(names);

    return 0;
}
