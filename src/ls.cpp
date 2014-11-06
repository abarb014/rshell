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
#include <stdio.h>

using namespace std;

// define Statements for simplification of the flag parsing

#define FLAG_a 1
#define FLAG_l 2
#define FLAG_R 4

// macros for colors

#define D_COLOR "\x1b[38;5;27m"
#define E_COLOR "\x1b[38;5;34m"
#define H_COLOR "\x1b[47m"
#define RESET_C "\x1b[0m"

vector<string> listDirectories(int flags, string initial)
{
    /* This function will print the contents of a single directory with either the -a or -l flag
     * however, it will not handle recursion. This will be used within another recursive function 
     * to handle the -R flag.
     */

    // All files and directories are stored and sorted in the files vector
    // Statbufs corresponding to the SORTED files are stored in info
   
    vector<string> files;
    vector<string> directories;
    vector<struct stat> info;

    struct stat statbuf;
    struct passwd *pwd;
    struct group *grp;

    string directory_path(initial);
    directory_path.append("/");
    string copy(directory_path);

    const char *dirName = initial.c_str();
    DIR *dirp = opendir(dirName);
    if (dirp == NULL)
    {
        perror("opendir");
        exit(1);
    }
    dirent *direntp;

    while ( (direntp = readdir(dirp)) )
    {
        files.push_back(direntp->d_name);
    }

    if (errno != 0)
    {
        perror("readdir");
        exit(1);
    }

    sort(files.begin(), files.end());
    
    for (unsigned i = 0; i < files.size(); i++)
    {
        directory_path.append(files.at(i));
        if ((stat(directory_path.c_str(), &statbuf)) == -1)
        {
            perror("stat");
            exit(1);
        }
        info.push_back(statbuf);

        if ((files.at(i) != ".") && (files.at(i) != "..") && (S_ISDIR(statbuf.st_mode)))
        {
            directories.push_back(directory_path);
        }

        directory_path = copy;
    }

    // Now branch off depending on the kind of printing we're doing

    // Only the 'a' flag is set, and not the 'l'
    if ((flags & FLAG_a) && !(flags & FLAG_l))
    {
        // Ouput formatting
        unsigned width = 80;
        unsigned longest = 0;

        for (unsigned i = 0; i < files.size(); i++)
        {
            if (files.at(i).size() > longest)
                longest = files.at(i).size();
        }

        longest += 4; // Minimum 4 spaces between longest entries.
        unsigned columns = width / longest;

        unsigned current = 0;

        // Ouput
        for (unsigned i = 0; i < files.size(); i++)
        {
            directory_path.append(files.at(i));
            if ((stat(directory_path.c_str(), &statbuf)) == -1)
            {
                perror("stat");
                exit(1);
            }

            if (files.at(i)[0] == '.')
            {
                cout << H_COLOR;
            }

            if (S_ISDIR(statbuf.st_mode))
            {
                cout << D_COLOR;
                cout << files.at(i) << "/";
                current = files.at(i).size() + 1;
            }
            else
            {
                if (statbuf.st_mode & S_IEXEC)
                {
                    cout << E_COLOR;
                }
                cout << files.at(i);
                current = files.at(i).size();
            }

            cout << RESET_C;
            cout << string(longest - current, ' ');
            columns--;
            if (columns == 0)
            {
                columns = width / longest;
                cout << endl;
            }

            directory_path = copy;
        }
    }

    // The 'l flag is set, and POSSIBLY the 'a' flag too
    else if (flags & FLAG_l)
    {
        // Calculate the number of blocks and print it first
        // Default ls blocks are 1024, so divide our number of 512 blocks by 2
        int blocks = 0;
        for (unsigned i = 0; i < files.size(); i++)
        {
            if (!(flags & FLAG_a) && files.at(i)[0] == '.')
                continue;
            else
                blocks += info.at(i).st_blocks;
        }
        cout << "total: " << blocks/2 << endl;

        for (unsigned i = 0; i < files.size(); i++)
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

                if (files.at(i)[0] == '.')
                    cout << H_COLOR;

                if (S_ISDIR(info.at(i).st_mode))
                {
                    cout << D_COLOR;
                    cout << files.at(i);
                    cout << '/';
                }

                else
                {
                    if (info.at(i).st_mode & S_IEXEC)
                        cout << E_COLOR;
                        cout << files.at(i);
                }

                cout << RESET_C;

                if (!(i + 1 == files.size()))
                    cout << endl;
            }
        }
    }


    // No flags :)
    else
    {
        // Ouput formatting
        unsigned width = 80;
        unsigned longest = 0;

        for (unsigned i = 0; i < files.size(); i++)
        {
            if (files.at(i).size() > longest)
                longest = files.at(i).size();
        }

        longest += 4; // Minimum 4 spaces between longest entries.
        unsigned columns = width / longest;

        unsigned current = 0;

        // Ouput begins
        for (unsigned i = 0; i < files.size(); i++)
        {
            if ( files.at(i)[0] == '.')
                continue;
            else
            {

                directory_path.append(files.at(i));
                if ((stat(directory_path.c_str(), &statbuf)) == -1)
                {
                    perror("stat");
                    exit(1);
                }

                if (S_ISDIR(statbuf.st_mode))
                {
                    cout << D_COLOR;
                    cout << files.at(i) << "/";
                    current = files.at(i).size() + 1;
                    cout << RESET_C;
                }

                else
                {
                    if (statbuf.st_mode & S_IEXEC)
                        cout << E_COLOR;

                    cout << files.at(i);
                    current = files.at(i).size();
                    cout << RESET_C;
                }

                cout << string(longest - current, ' ');
                columns--;
                if (columns == 0)
                {
                    columns = width / longest;
                    cout << endl;
                }

                directory_path = copy;
            }
        }
    }

    cout << endl;
    return directories;
}

void allDirectories(int flags, string initial)
{
    cout << initial << ":" << endl;

    vector<string> directories = listDirectories(flags,initial);

    cout << endl;

    for (unsigned i = 0; i < directories.size(); i++)
    {
        allDirectories(flags, directories.at(i));
    }

    return;
}

int main(int argc, char **argv)
{
    // Parse the arguments to get the flags out, and check for invalid flags
    int flags = 0;
    // Keep anything else that's typed as file 
    vector<string> files;

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

        else
        {
            files.push_back(argv[i]);
        }
    }

    sort(files.begin(), files.end());

    if (files.empty())
    {
        if (flags & FLAG_R)
        {
            allDirectories(flags, ".");
        }
        else
        {
            listDirectories(flags, ".");
        }
    }

    else
    {
        for (unsigned i = 0; i < files.size(); i++)
        {
            if (flags & FLAG_R)
            {
                allDirectories(flags, files.at(i));
            }
            else
            {
                if (files.size() > 1)
                    cout << files.at(i) << ":" << endl;
                listDirectories(flags, files.at(i));
            }

            if (!(i + 1 == files.size()))
                cout << endl;
        }
    }

    return(0);
}
