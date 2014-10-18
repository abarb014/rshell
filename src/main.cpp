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

const int MAX_ARGS = 100;

string cleanInput(const string&);
void statusChecker(queue<string>&, queue<string>&, int&, int&);

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

    int commandCount = 0;
    
    // Further parse the input to stop at a connecter
    // Connector status will be set by an int variable
    int status = 0;
    queue<string> con_command_list;
    statusChecker(command_list, con_command_list, commandCount, status);

    // Dynamically allocate the arrays we need for our command list
  
    char **argv = new char *[commandCount + 1];
    for (int i = 0; i < commandCount; i++)
    {
        argv[i] = new char[MAX_ARGS];
        strcpy(argv[i], con_command_list.front().c_str());

        con_command_list.pop();
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

            // If status 1 (semicolon) is detected, add commands from command_list until next connector is reached, and start again
            if (status == 1)
            {
                // First clear the old arrays
                for (int i = 0; i < commandCount; i++)
                {
                    delete [] argv[i];
                }

                delete [] argv;
                commandCount = 0;
                
                // Reset status and build up command list again
                statusChecker(command_list, con_command_list, commandCount, status);
                
                // Build the new arrays and start the loop over again
                argv  = new char *[commandCount + 1];
                for (int i = 0; i < commandCount; i++)
                {
                    argv[i] = new char[MAX_ARGS];
                    strcpy(argv[i], con_command_list.front().c_str());
                    con_command_list.pop();
                }
                argv[commandCount] = '\0';

                continue;
            }

            // This stuff will happen AFTER the connectors
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

            commandCount = 0;
            statusChecker(command_list, con_command_list, commandCount, status);

            argv = new char *[commandCount + 1];
            for (int i = 0; i < commandCount; i++)
            {
                argv[i] = new char[MAX_ARGS];
                strcpy(argv[i], con_command_list.front().c_str());

                con_command_list.pop();
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

void statusChecker(queue<string>& original, queue<string>& fixed, int& commandCount, int& status)
{
    for (int i = 0, size = original.size(); i < size; i++)
    {
        if (original.front().compare(";") == 0)
        {
            original.pop();
            status = 1;
            return;
        }
        else
        {
            fixed.push(original.front());
            original.pop();
            commandCount += 1;
        }
    }

    // If it makes it throught the checker, set status back to 0
    status = 0;
}
