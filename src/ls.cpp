#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string>
#include <algorithm>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <ctime>

using namespace std;

// define Statements for simplification of the flag parsing

#define FLAG_a 1
#define FLAG_l 2
#define FLAG_R 4


void listDirectories(int flags)
{
    /* This function will print the contents of a single directory with either the -a or -l flag
     * however, it will not handle recursion. This will be used within another recursive function 
     * to handle the -R flag.
     */

    // All files and directories are stored and sorted in the files vector
    // Statbufs corresponding to the SORTED files are stored in info
   
    vector<string> files;
    vector<struct stat> info;

    struct stat statbuf;
    struct passwd *pwd;
    struct group *grp;

    char *dirName = ".";
    DIR *dirp = opendir(dirName);
    dirent *direntp;

    while ( (direntp = readdir(dirp)) )
    {
        files.push_back(direntp->d_name);
    }

    sort(files.begin(), files.end());
    
    for (int i = 0; i < files.size(); i++)
    {
        stat(files.at(i).c_str(), &statbuf);
        info.push_back(statbuf);
    }

    // Now branch off depending on the kind of printing we're doing

    if ((flags & FLAG_a) && !(flags & FLAG_l))
    {
        for (int i = 0; i < files.size(); i++)
        {
            cout << files.at(i) << "\t";
        }
    }

    else if (flags & FLAG_l)
    {
        // Calculate the number of blocks and print it first
        // Default ls blocks are 1024, so divide our number of 512 blocks by 2
        int blocks = 0;
        for (int i = 0; i < files.size(); i++)
        {
            if (!(flags & FLAG_a) && files.at(i)[0] == '.')
                continue;
            else
                blocks += info.at(i).st_blocks;
        }
        cout << "total: " << blocks/2 << endl;

        for (int i = 0; i < files.size(); i++)
        {
            if (files.at(i)[0] == '.' && !(flags & FLAG_a))
                continue;
            else
            {
                // File Type (Directory, Link, or File?)
                if (S_ISDIR(info.at(i).st_mode))
                    cout << 'd';
                else if (S_ISLNK(info.at(i).st_mode))
                    cout << 'l';
                else
                    cout << '-';

                // Owner Permissions
                if (info.at(i).st_mode & S_IRUSR)
                    cout << 'r';
                else cout << '-';
                if (info.at(i).st_mode & S_IWUSR)
                    cout << 'w';
                else cout << '-';
                if (info.at(i).st_mode & S_IXUSR)
                    cout << 'x';
                else cout << '-';

                // Group Permissions
                if (info.at(i).st_mode & S_IRGRP)
                    cout << 'r';
                else cout << '-';
                if (info.at(i).st_mode & S_IWGRP)
                    cout << 'w';
                else cout << '-';
                if (info.at(i).st_mode & S_IXGRP)
                    cout << 'x';
                else cout << '-';

                // Other Users Permissions
                if (info.at(i).st_mode & S_IROTH)
                    cout << 'r';
                else cout << '-';
                if (info.at(i).st_mode & S_IWOTH)
                    cout << 'w';
                else cout << '-';
                if (info.at(i).st_mode & S_IXOTH)
                    cout << 'x';
                else cout << '-';

                // inode number
                cout << " " << info.at(i).st_nlink << " ";

                // User ID
                pwd = getpwuid(info.at(i).st_uid);
                cout << pwd->pw_name << " ";

                // Group ID
                grp = getgrgid(info.at(i).st_gid);
                cout << grp->gr_name << " ";

                // Size in Bytes
                cout << info.at(i).st_size << " ";

                // Time of last modification
                time_t time = info.at(i).st_mtime;
                string time_s = ctime(&time);
                cout << time_s.substr(4, 12) << " ";

                cout << files.at(i); 

                if (S_ISDIR(info.at(i).st_mode))
                    cout << '/';

                if (!(i + 1 == files.size()))
                    cout << endl;
            }
        }
    }


    else
    {
        for (int i = 0; i < files.size(); i++)
        {
            if ( files.at(i)[0] == '.')
                continue;
            else
                cout << files.at(i) << "\t";

        }
    }

    cout << endl;
    return;
}

int main(int argc, char **argv)
{
    // Parse the arguments to get the flags out, and check for invalid flags
    int flags = 0;

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            for (int j = 1; argv[i][j] != 0; j++)
            {
                if (argv[i][j] == 'a')
                    flags |= FLAG_a;
                else if (argv[i][j] == 'l')
                    flags |= FLAG_l;
                else if (argv[i][j] == 'R')
                    flags |= FLAG_R;
                else
                {
                    // Program will exit when given invalid flags
                    cerr << "ls: invalid option -- \'" << argv[i][j] << "\'\n";
                    exit(1);
                }
            }
        }
    }

    listDirectories(flags);

    return(0);

    char *dirName = ".";
    DIR *dirp = opendir(dirName);
    dirent *direntp;
    while ((direntp = readdir(dirp)))
        cout << direntp->d_name << endl;  // use stat here to find attributes of file
    closedir(dirp);
}
