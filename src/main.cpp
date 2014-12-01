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
#include <fcntl.h>
#include <signal.h>

using namespace std;
using namespace boost;

const int MAX_ARGS = 100;
char CWD[1024];
string prompt;
int pid = 1;
bool isChild = false;

string cleanInput(const string&);
void getInput(const string&, string &, queue<string> &);
void statusChecker(queue<string>&, queue<string>&, int&, int&, char**&);
void buildArrays(char **&, const int &, queue<string> &);
void clearQueue(queue<string>&);
void clearArrays(char **&argv, int &commandCount);
void rshellExit();
void sigint_handler(int);
int my_exec(const char *prog, char *const args[]);

int main()
{
    if (signal(SIGINT, sigint_handler) == SIG_ERR)
    {
        perror("signal");
    }

    if (getcwd(CWD, sizeof(CWD)) == NULL)
    {
        perror("getcwd");
        prompt = "$ ";
    }
    
    else
    {
        prompt = CWD;
        prompt.append(" $ ");
    }
    // HW0 Extra Credit
   
    /* bool loginStatus = 0;
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
    } */
    
    // Obtain a line of input from the user then fix it, and tokenize it
    string input_line;
    queue<string> raw_commands;

    getInput(prompt, input_line, raw_commands);
    int commandCount = 0;
    
    // Connector status will be set by an int variable
    int status = 0;
    char **argv = NULL;
    queue<string> command_list;
    statusChecker(raw_commands, command_list, commandCount, status, argv);

    buildArrays(argv, commandCount, command_list);

    int *commandStatus = new int;
    int fd[2];
    int savestdin;

    if ((savestdin = dup(0)) == -1)
    {
        perror("dup");
        exit(1);
    }

    while(1)
    {
        if (pipe(fd) == -1)
        {
            perror("pipe");
            exit(1);
        }

        // FIX
        pid = fork();
        
        if (pid == -1)
        {
            perror("fork");
            exit(1);
        }

        else if (pid == 0) // This is the child process
        {
            isChild = true;

            // If we need to do input redirection
            if (status == 5)
            {
               if (raw_commands.empty())
               {
                   cerr << "Error: expected argument after '<'\n";
                   exit(1);
               }
                   
               else
               {
                   int in = open(raw_commands.front().c_str(), O_RDONLY);
                   if (in == -1)
                   {
                       perror("open");
                       exit(1);
                   }

                   if (dup2(in, 0) == -1)
                   {
                       perror("dup2");
                       exit(1);
                   }

                   // If piping follows or output redirection follows
                   if (!raw_commands.empty())
                       raw_commands.pop();
                   if (!raw_commands.empty() && raw_commands.front().compare("|") == 0)
                   {
                       status = 8;
                   }
                   if (!raw_commands.empty() && raw_commands.front().compare(">") == 0)
                   {
                       raw_commands.pop();
                       status = 6;
                   }
                   if (!raw_commands.empty() && raw_commands.front().compare(">>") == 0)
                   {
                       raw_commands.pop();
                       status = 7;
                   }
               }
            }

            // If we need to do output (1) redirection (create  or overwrite)
            if (status == 6)
            {
                if (raw_commands.empty())
                {
                    cerr << "Error: expected argument after '>'\n";
                    exit(1);
                }

                int out = open(raw_commands.front().c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
                if (out == -1)
                {
                    perror("open");
                    exit(1);
                }

                if (dup2(out, 1) == -1)
                {
                    perror("dup2");
                    exit(1);
                }
            }

            // If we need to do output (2) redirection (create or append)
            if (status == 7)
            {
                if (raw_commands.empty())
                {
                    cerr << "Error: expected argument after '>>'\n";
                    exit(1);
                }

                int out = open(raw_commands.front().c_str(), O_CREAT | O_WRONLY | O_APPEND, 0666);
                
                if (out == -1)
                {
                    perror("open");
                    exit(1);
                }

                if (dup2(out, 1) == -1)
                {
                    perror("dup2");
                    exit(1);
                }
            }

            // If we need to do piping
            if (status == 8)
            {
                if (raw_commands.empty())
                {
                    cerr << "Error: expected argument after '|'\n";
                    exit(1);
                }

                if (dup2(fd[1], 1) == -1)
                {
                    perror("dup2");
                    exit(1);
                }

                if (close(fd[0]) == -1)
                {
                    perror("close");
                    exit(1);
                }
            }

            if (my_exec(argv[0], argv) == -1)
            {
                perror("my_exec");
                exit(1);
            }
            /*if (execvp(argv[0], argv) == -1)
            {
                perror("execvp");
                exit(1);
            }*/

            exit(0);
        }

        else // This is the parent process
        {
            pid = wait(&commandStatus);

            if (pid == -1)
            {
                perror("wait");
            }

            // If status 1 (semicolon) is detected, add commands from raw_commands until next connector is reached, and start again

            if (status == 1)
            {
                clearArrays(argv, commandCount);
                statusChecker(raw_commands, command_list, commandCount, status, argv);
                buildArrays(argv, commandCount, command_list);

                continue;
            }

            // If status 2 (AND) is detected, the next command will only execute if the first succeeds.

            else if (status == 2)
            {
                if (WEXITSTATUS(commandStatus) == 1)
                {
                    clearQueue(raw_commands);
                    clearQueue(command_list);

                    clearArrays(argv, commandCount);

                    getInput(prompt, input_line, raw_commands);
                    statusChecker(raw_commands, command_list, commandCount, status, argv);
                    buildArrays(argv, commandCount, command_list);
                    
                    continue;
                }

                else
                {
                    clearArrays(argv, commandCount);
                    statusChecker(raw_commands, command_list, commandCount, status, argv);
                    buildArrays(argv, commandCount, command_list);

                    continue;
                }
            }

            // If status 3 (OR) is detected, the next command will only execute if the first fails.

            else if (status == 3)
            {
                if (WEXITSTATUS(commandStatus) == 1)
                {
                    clearArrays(argv, commandCount);
                    statusChecker(raw_commands, command_list, commandCount, status, argv);
                    buildArrays(argv, commandCount, command_list);

                    continue;
                }

                else
                {
                    clearQueue(raw_commands);
                    clearQueue(command_list);

                    clearArrays(argv, commandCount);

                    getInput(prompt, input_line, raw_commands);
                    statusChecker(raw_commands, command_list, commandCount, status, argv);
                    buildArrays(argv, commandCount, command_list);

                    continue;
                }
            }

            // If status 4 (#) is detected, the program will discard everything after it

            else if (status == 4)
            {
                clearQueue(raw_commands);
                clearQueue(command_list);

                clearArrays(argv, commandCount);

                getInput(prompt, input_line, raw_commands);
                statusChecker(raw_commands, command_list, commandCount, status, argv);
                buildArrays(argv, commandCount, command_list);

                continue;
            }

            // If status 5 (<) is detected, pop raw_commands
            else if (status == 5)
            {
                if (!raw_commands.empty())
                    raw_commands.pop();

                if (!raw_commands.empty() && raw_commands.front().compare("|") == 0)
                {
                    raw_commands.pop();
                    status = 8;
                }

                else if (!raw_commands.empty() && raw_commands.front().compare(">") == 0)
                {
                    raw_commands.pop();
                    raw_commands.pop();
                }

                else if (!raw_commands.empty() && raw_commands.front().compare(">>") == 0)
                {
                    raw_commands.pop();
                    raw_commands.pop();
                }

                else
                {
                    clearArrays(argv, commandCount);
                    statusChecker(raw_commands, command_list, commandCount, status, argv);
                    buildArrays(argv, commandCount, command_list);

                    continue;
                }
            }

            // If status 6 (>) is detected, pop raw_commands
            else if (status == 6)
            {
                if (!raw_commands.empty())
                    raw_commands.pop();

                clearArrays(argv, commandCount);
                statusChecker(raw_commands, command_list, commandCount, status, argv);
                buildArrays(argv, commandCount, command_list);

                continue;
            }

            // Do the same as status 6 basically
            else if (status == 7)
            {
                if (!raw_commands.empty())
                    raw_commands.pop();

                clearArrays(argv, commandCount);
                statusChecker(raw_commands, command_list, commandCount, status, argv);
                buildArrays(argv, commandCount, command_list);

                continue;
            }

            // Piping
            if (status == 8)
            {
                if (dup2(fd[0], 0))
                {
                    perror("dup2");
                    exit(1);
                }

                if (close(fd[1]))
                {
                    perror("close");
                    exit(1);
                }

                clearArrays(argv, commandCount);
                statusChecker(raw_commands, command_list, commandCount, status, argv);
                buildArrays(argv, commandCount, command_list);

                continue;
            }

            // This stuff will happen AFTER the connectors

            if (dup2(savestdin, 0) == -1)
            {
                perror("dup2");
                exit(1);
            }

            clearArrays(argv, commandCount);
            getInput(prompt, input_line, raw_commands);
            statusChecker(raw_commands, command_list, commandCount, status, argv);
            buildArrays(argv, commandCount, command_list);
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
            new_input += " ; ";

        else if (input[i] == '#')
            new_input += " # ";

        else if (input[i] == '|')
        {
            if (i+1 < input.size())
            {
                if (input[i+1] == '|')
                {
                    new_input += " || ";
                    continue;
                }
            }

            new_input += " | ";
        }

        else if (input[i] == '&')
        {
            if (i+1 < input.size())
            {
                if (input[i+1] == '&')
                    new_input += " && ";
            }
        }

        else if (input[i] == '<')
            new_input += " < ";

        else if (input[i] == '>')
        {
            if (i+1 < input.size())
            {
                if (input[i+1] == '>')
                {
                    new_input += " >> ";
                    i += 1;
                    continue;
                }

            }

            new_input += " > ";
        }

        else
        {
            new_input += input[i];
        }
    }

    return new_input;
}

void statusChecker(queue<string>& original, queue<string>& fixed, int& commandCount, int& status, char **&argv)
{
    // ALWAYS CHECK IF THE DAMN QUEUE IS EMPTY BEFORE TRYING TO ACCESS MEMORY
    if (original.empty())
    {
        status = 0;
        return;
    }

    if (original.front().compare("exit") == 0)
    {
        for (int i = 0; i < commandCount; i++)
        {
            delete [] argv[i];
            argv[i] = NULL;
        }

        delete [] argv;
        argv = NULL;

        rshellExit();
    }

    if (original.front().compare("cd") == 0)
    {
        original.pop();

        if (chdir(original.front().c_str()) == -1)
        {
            perror("chdir");
        }

        while (!original.empty())
        {
            original.pop();
        }

        status = 0;

        // Capture the working directory

        if (getcwd(CWD, sizeof(CWD)) == NULL)
        {
            perror("getcwd");
            prompt = "$ ";
        }

        else
        {
            prompt = CWD;
            prompt.append(" $ ");
        }

        return;
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

        // If < is found, we need to do input redirection
        else if (original.front().compare("<") == 0)
        {
            original.pop();
            status = 5;
            return;
        }

        // If > is found, we need to do ouput (1) redirection
        else if (original.front().compare(">") == 0)
        {
            original.pop();
            status = 6;
            return;
        }

        // If >> is found, we need to do ouput (2) redirection
        else if (original.front().compare(">>") == 0)
        {
            original.pop();
            status = 7;
            return;
        }

        // If | is found, we need to do piping
        else if (original.front().compare("|") == 0)
        {
            original.pop();
            status = 8;
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

void buildArrays(char **&argv, const int &commandCount, queue<string> &command_list)
{
    argv = new char *[commandCount + 1];

    for (int i = 0; i < commandCount; i++)
    {
        argv[i] = new char[MAX_ARGS];
        strcpy(argv[i], command_list.front().c_str());

        command_list.pop();
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
        argv[i] = NULL;
    }

    delete [] argv;
    argv = NULL;
    commandCount = 0;
}

void getInput(const string& prompt, string &input_line, queue<string> &raw_commands)
{

    cout << prompt;
    getline(cin, input_line);
    input_line = cleanInput(input_line);
    char_separator<char> sep(" ");
    tokenizer< char_separator<char> > tok(input_line, sep);

    for (tokenizer< char_separator<char> >::iterator it = tok.begin(); it != tok.end(); it++)
    {
        raw_commands.push(*it);
    }
}

void rshellExit()
{
    cout << "logout\n" << endl;
    cout << "[Process Completed]" << endl;
    exit(0); // Successful exit
}

void sigint_handler(int signum)
{
    if (isChild)
        kill(pid, SIGKILL);

    cout << endl;
}

int my_exec(const char *prog, char *const args[])
{
    string path = getenv("PATH");
    queue<string> raw_path;

    // Tokenize the path
    char_separator<char> sep(":");
    tokenizer< char_separator<char> > tok(path, sep);

    for (tokenizer< char_separator<char> >::iterator it = tok.begin(); it != tok.end(); it++)
    {
        raw_path.push(*it);
    }

    // Test possible locations for the program
    string current_path;

    while (!raw_path.empty())
    {
        current_path = raw_path.front();

        if (current_path.at(current_path.size() - 1) != '/')
            current_path.append("/");

        current_path.append(prog);

        if (execv(current_path.c_str(), args) == -1)
        {
            if (errno == ENOENT)
                raw_path.pop();
            else
                return -1;
        }

        if (raw_path.empty())
            return -1;
    }

    return 0;
}
