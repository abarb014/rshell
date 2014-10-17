#include <iostream>
#include <boost/tokenizer.hpp>
#include <string>
#include <string.h>
#include <unistd.h>
#include <queue>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

using namespace std;
using namespace boost;

const int MAX_ARGS = 20;

string cleanInput(const string&);

int main()
{
    // Obtain a line of input from the user then fix it, and tokenize it
    // then save it in a Queue.
    string command_line;

    cout << "$ ";

    getline(cin, command_line);
    command_line = cleanInput(command_line);

    queue<string> command_list;
    char_separator<char> sep(" ");
    tokenizer< char_separator<char> > tok(command_line, sep);
    for (tokenizer< char_separator<char> >::iterator it = tok.begin(); it != tok.end(); it++)
    {
        command_list.push(*it);
    }

    int commandCount = command_list.size();

    // Dynamically allocate the arrays we need for our command list
  
    char **argv = new char *[commandCount + 1];
    for (int i = 0; i < commandCount; i++)
    {
        argv[i] = new char[MAX_ARGS];
        strcpy(argv[i], command_list.front().c_str());

        command_list.pop();
    }
    argv[commandCount] = '\0';
    
    // Fork and execute the commands!
    while(1)
    {
        int pid = fork();
        
        if (pid == -1) // This is the error checker
        {
            perror("fork");
        }
        else if (pid == 0) // This is the child process
        {
            if (execvp(argv[0], argv) == -1)
            {
                perror("execvp");
            }
            exit(1);
        }
        else // This is the parent process
        {
            pid = wait(NULL);

            if (pid == -1)
            {
                perror("wait");
            }

            // clear the previous arrays
            for (int i = 0; i < commandCount; i++)
            {
                delete [] argv[i];
            }

            delete [] argv;

            // Get new input, fix it, and tokenize it.
            cout << "$ ";
            getline(cin, command_line);
            command_line = cleanInput(command_line);
            tokenizer< char_separator<char> > tok(command_line, sep);
            for (tokenizer< char_separator<char> >::iterator it = tok.begin(); it != tok.end(); it++)
            {
                command_list.push(*it);
            }

            commandCount = command_list.size();

            argv = new char *[commandCount + 1];
            for (int i = 0; i < commandCount; i++)
            {
                argv[i] = new char[MAX_ARGS];
                strcpy(argv[i], command_list.front().c_str());

                command_list.pop();
            }

            argv[commandCount] = '\0';
        }
    }

    return 0;
}

string cleanInput(const string& input)
{
    string new_input;

    for (int i = 0; i < input.size(); i++)
    {
        if (input[i] == ';')
        {
            new_input += " ; ";
        }
        else
        {
            new_input += input[i];
        }
    }

    return new_input;
}
