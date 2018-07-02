#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>
using namespace std;

vector<string> getFiles(string cate_dir)
{
    vector<string> files;
    DIR *dir;
    struct dirent *ptr;

    if ((dir=opendir(cate_dir.c_str())) == NULL)
    {
    cout<< ("Open dir error...") <<endl;
            exit(1);
    }

    while ((ptr=readdir(dir)) != NULL)
    {
        if (ptr->d_type == 10)
        {
        files.push_back(ptr->d_name);
        }
        else
            continue;
    }
    closedir(dir);
    sort(files.begin(), files.end());
    return files;

}

string get_device_name (string path)
{
    string filename = path +"name";
    ifstream device_name {filename};
    string line;
    std::getline(device_name, line);
    return line;
}

string get_orbbec_camera ()
{
    string videoFilePath = "/sys/class/video4linux/";
//    string imuFilePath = "/sys/class/tty/";
    string Orbbec_name = "ORBBEC";

    vector<string> files = getFiles(videoFilePath);
    for (int i = 0; i< files.size(); i++)
    {
        string videoFileName = videoFilePath + files[i] + "/";
        string video_device_name = get_device_name(videoFileName);

        if (video_device_name.find(Orbbec_name) != string::npos)
        {
            return "/dev/"+files[i];
        }
    }

}