#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <iostream>
#include <stdlib.h>

using namespace std;

// define Statements for simplification of the flag parsing

#define FLAG_a 1
#define FLAG_l 2
#define FLAG_R 4

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

    // This is here solely for testing purposes, remove when finished with the feature
    if (flags & FLAG_a)
        cout << "You used the \'a\' option!" << endl;
    if (flags & FLAG_l)
        cout << "You used the \'l\' option!" << endl;
    if (flags & FLAG_R)
        cout << "You used the \'R\' option!" << endl;

    return(0);

    char *dirName = ".";
    DIR *dirp = opendir(dirName);
    dirent *direntp;
    while ((direntp = readdir(dirp)))
        cout << direntp->d_name << endl;  // use stat here to find attributes of file
    closedir(dirp);
}
