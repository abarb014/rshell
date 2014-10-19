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
#include <sys/param.h>

using namespace std;
using namespace boost;

const int MAX_ARGS = 100;

string cleanInput(const string&);
void getInput(const string&, string &, queue<string> &);
void statusChecker(queue<string>&, queue<string>&, int&, int&);
void buildArrays(char **&, const int &, queue<string> &);
void clearQueue(queue<string>&);
void clearArrays(char **&argv, int &commandCount);
void rshellExit();

int main()
{
    // Set up the login and hostname and a failsafe in case either one doesn't work.
    bool loginStatus = 0;
    char host[MAXHOSTNAMELEN];
    if (gethostname(host, sizeof(host)) == -1)
    {
        perror("gethostname");
        loginStatus = 1;
    }
    char *login = getlogin();
    if (login == NULL)
    {
        perror("getlogin");
        loginStatus = 1;
    }
    
    string prompt;
    if (loginStatus == 1)
    {
        prompt = "$: ";
    }
    else
    {
        prompt.append(login);
        prompt.append("@");
        prompt.append(host);
        prompt.append(" $ ");
    }
    
    // Obtain a line of input from the user then fix it, and tokenize it
    // then save it in a Queue.
    string command_line;
    queue<string> command_list;

    getInput(prompt, command_line, command_list);
    int commandCount = 0;
    
    // Further parse the input to stop at a connecter
    // Connector status will be set by an int variable
    int status = 0;
    queue<string> con_command_list;
    statusChecker(command_list, con_command_list, commandCount, status);

    // Dynamically allocate the arrays we need for our command list
  
    char **argv;
    buildArrays(argv, commandCount, con_command_list);

    int *commandStatus = new int;

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
                exit(1); // To indicate error
            }
            exit(0); // A successful exit
        }
        else // This is the parent process
        {
            pid = wait(&commandStatus);
            //cout << WEXITSTATUS(commandStatus) << endl;

            if (pid == -1)
            {
                perror("wait");
            }

            // If status 1 (semicolon) is detected, add commands from command_list until next connector is reached, and start again
            if (status == 1)
            {
                // First clear the old arrays
                clearArrays(argv, commandCount);
                
                // Reset status and build up command list again
                statusChecker(command_list, con_command_list, commandCount, status);
                
                // Build the new arrays and start the loop over again
                buildArrays(argv, commandCount, con_command_list);

                continue;
            }

            // If status 2 (AND) is detected, the next command will only execute if the first succeeds.
            else if (status == 2)
            {
                if (WEXITSTATUS(commandStatus) == 1)
                {
                    // If the first command failed, clear the queues
                    clearQueue(command_list);
                    clearQueue(con_command_list);

                    // Call delete on the old arrays
                    clearArrays(argv, commandCount);

                    // Get new input and parse it
                    getInput(prompt, command_line, command_list);

                    // Check for more connectors
                    statusChecker(command_list, con_command_list, commandCount, status);

                    // Build the new command arrays
                    buildArrays(argv, commandCount, con_command_list);
                    
                    continue;
                }
                else
                {
                    // If it works, get the next command and test it too
                    clearArrays(argv, commandCount);

                    statusChecker(command_list, con_command_list, commandCount, status);

                    buildArrays(argv, commandCount, con_command_list);

                    continue;
                }
            }

            // If status 3 (OR) is detected, the next command will only execute if the first fails.
            else if (status == 3)
            {
                if (WEXITSTATUS(commandStatus) == 1)
                {
                    clearArrays(argv, commandCount);

                    statusChecker(command_list, con_command_list, commandCount, status);

                    buildArrays(argv, commandCount, con_command_list);

                    continue;
                }
                else
                {
                    clearQueue(command_list);
                    clearQueue(con_command_list);

                    clearArrays(argv, commandCount);

                    getInput(prompt, command_line, command_list);

                    statusChecker(command_list, con_command_list, commandCount, status);

                    buildArrays(argv, commandCount, con_command_list);

                    continue;
                }
            }

            // If status 4 (#) is detected, the program will discard everything after it
            else if (status == 4)
            {
                clearQueue(command_list);
                clearQueue(con_command_list);

                clearArrays(argv, commandCount);

                getInput(prompt, command_line, command_list);

                statusChecker(command_list, con_command_list, commandCount, status);

                buildArrays(argv, commandCount, con_command_list);

                continue;
            }

            // This stuff will happen AFTER the connectors
            clearArrays(argv, commandCount);

            getInput(prompt, command_line, command_list);

            statusChecker(command_list, con_command_list, commandCount, status);

            buildArrays(argv, commandCount, con_command_list);
        }
    }

    return 0;
}

string cleanInput(const string& input)
{
    string new_input;

    for (unsigned i = 0; i < input.size(); i++)
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
    if (original.front().compare("exit") == 0)
    {
        rshellExit();
    }

    for (int i = 0, size = original.size(); i < size; i++)
    {
        // If a semicolon is found, anything after it will be executed
        if (original.front().compare(";") == 0)
        {
            original.pop();
            status = 1;
            return;
        }
        // If && is found, only execute the next command if it succeeds
        else if (original.front().compare("&&") == 0)
        {
            original.pop();
            status = 2;
            return;
        }
        // If || is found, only execute the next command if the first fails
        else if (original.front().compare("||") == 0)
        {
            original.pop();
            status = 3;
            return;
        }
        // If # is found, everything after it will be discarded
        else if (original.front().compare("#") == 0)
        {
            original.pop();
            status = 4;
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

void buildArrays(char **&argv, const int &commandCount, queue<string> &con_command_list)
{
    argv = new char *[commandCount + 1];
    for (int i = 0; i < commandCount; i++)
    {
        argv[i] = new char[MAX_ARGS];
        strcpy(argv[i], con_command_list.front().c_str());

        con_command_list.pop();
    }

    argv[commandCount] = '\0';
}

void clearQueue(queue<string> &byebye)
{
    while (!byebye.empty())
    {
        byebye.pop();
    }
}

void clearArrays(char **&argv, int &commandCount)
{
    for (int i = 0; i < commandCount; i++)
    {
        delete [] argv[i];
    }

    delete [] argv;
    commandCount = 0;
}

void getInput(const string& prompt, string &command_line, queue<string> &command_list)
{

    cout << prompt;
    getline(cin, command_line);
    command_line = cleanInput(command_line);
    char_separator<char> sep(" ");
    tokenizer< char_separator<char> > tok(command_line, sep);
    for (tokenizer< char_separator<char> >::iterator it = tok.begin(); it != tok.end(); it++)
    {
        command_list.push(*it);
    }
}

void rshellExit()
{
    cout << "logout\n" << endl;
    cout << "[Process Completed]" << endl;
    exit(1); // Successful exit
}
