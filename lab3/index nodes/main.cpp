#include <iostream>

#include <string>

#include <dirent.h>

#include <sys/stat.h>

#include <map>

#include <vector>



using namespace std;



typedef struct IndexNode {

    nlink_t links;

    string path;

    IndexNode(unsigned long links, string path) {

        this->links = links;

        this->path = path;

    }



}INODE;



void searchDirectory(map<ino_t, vector<INODE>>& inodeMap, string path = "./"){

    DIR* dir;

    dirent* dirInfo;



    dir = opendir(path.c_str());

    if(!dir) {

        cout << "ERROR OPENING DIRECTORY: PATH [" + path + "] "<< endl;

        return;

    }

    while(dirInfo = readdir(dir)) {

        string entryName = dirInfo->d_name;

        switch (dirInfo->d_type)

        {

        case DT_DIR:

            {

                if(entryName.compare(".") && entryName.compare("..")) {

                    searchDirectory(inodeMap, path + "/" + entryName);

                }

            }

            break;

        case DT_REG:

            {

                string newPath = path + "/" + entryName;

                struct stat fileInfo;

                stat(newPath.c_str(), &fileInfo);



                if(fileInfo.st_nlink > 1) {

                    inodeMap[fileInfo.st_ino].push_back(INODE(fileInfo.st_nlink, path));

                }

            }

            break;

        

        default:

            break;

        }

    }

    closedir(dir);

}



void search(map<ino_t, vector<INODE>>& inodeMap) {

    string root = "/home";

    searchDirectory(inodeMap, root);

}



void printStats(map<ino_t, vector<INODE>>& inodeMap) {

     for(auto it = inodeMap.begin(); it != inodeMap.end(); ++it) {

        vector<INODE> inodeVec = it->second;

        cout << "node : " << it->first << endl;

        cout << "count links : " << inodeVec[0].links << endl;

        cout << "pathes : " << endl;

        for(INODE inode : inodeVec){

            cout << inode.path << endl;

        }

        printf("-------------------------------------------\n");

    }

}



int main(int argc, char** argv) {

    map<ino_t, vector<INODE>> inodeMap;

    inodeMap.clear();

    search(inodeMap);

    printStats(inodeMap);

    return 0;

}

