#include <iostream>
#include <boost/tokenizer.hpp>
#include <string>
#include <string.h>
#include <unistd.h>
#include <queue>

using namespace std;
using namespace boost;

string cleanInput(const string&);

int main()
{
    // Obtain a line of input from the user then fix it, and tokenize it
    // then save it in a Queue.
    string command_line;
    getline(cin, command_line);
    command_line = cleanInput(command_line);

    queue<string> command_list;
    char_separator<char> sep(" ");
    tokenizer< char_separator<char> > tok(command_line, sep);
    for (tokenizer< char_separator<char> >::iterator it = tok.begin(); it != tok.end(); it++)
    {
        command_list.push(*it);
    }
    
    // Now we're gonna start an infinite loop, and fork the program so we can run multiple commands.

    // Test to see if it correctly tokenized all items.

    cout << "You said: " << endl;
    for (int i = 0; i < command_list.size(); i++)
    {
        cout << command_list.front() << endl;
        command_list.pop();
    }
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
    
