#Rshell

* Project source can be downloaded from https://github.com/abarb014/rshell

##Author and Contributor List

Andrew Barboza

* Report any bugs immediately to: abarb014@ucr.edu

##Instructions

1. Clone the directory to your computer
2. `cd` into the `rshell` directory
3. Use `make` to compile the program
4. Run `bin/rshell` to use the program. It will take over your default shell.
5. `bin/ls` and `bin/cp` can also be run withing the `rshell` program.

## Description

`rshell` is a basic shell program. It is based around BASH, and can use many of the same functionalities
 as the BASH. `rshell` is also capable of using connectors and i/o redirection. Some programs from BASH, such as
 `history` are currently not supported in `rshell`. However, a notable addition to `rshell` is the `cd` command, which allows the user to change directories while using the program. Along with this repository comes
 two additional programs that can be called from within `rshell`: `ls`, which will print the contents of a directory,
 and `cp`, which will copy one file to another. `ls` supports the `-alR` flags and outputs with color. `cp` will
 create a file if the given name if unique, however it will not overwite an existing file.

##Features

* Can run many commands that the BASH shell can.
* Uses a combination of `fork()`, `execvp()`, and `wait()` to mimic the behaviour of the BASH shell.
* Supports connectors `&&`, `||`, and `;` to help the user chain commands together.
* Displays the users login name and host name, unless an error occurs. In the case of an error, your prompt will be `$ `.
* Has a custom built `exit` command in order to quit the program.
* Supports the i/o redirectors `>`, `>>`, and `<`. If a chain of redirectors is used, the only the first argument
    after the redirector will be used.
* Redirections of the form [program] >/>> [file1] < [file2] will send output to file1, but the part after the `<` will do nothing. This is
    an invalid redirection.
* Redirections of the form [program1] > [file] | [program2] will not execute program2 properly because it does not recieve input. This is also
    an invalid redirection.
* Redirections of the form [program1] | [program2] < [file] will have the file take precedence as the standard input to program2.
* Supports piping, `|`.
* Includes a custom `ls` program that supports the `-alR` flags as well as color based on filetype.
* Includes a custom `cp` program that will copy files, but not overwrite existing ones.
* Fully functioning `cd` command that allows the user to change to a different directory. It only accepts two arguments, and will completely ignore anything after the second argument.

##Known Bugs

###rshell

* Typing multiple "&&" connectors with no spaces in between will not produce any errors.
* Typing multiple "||" connectors with no spaces in between will not produce any errors.
* Use of the "#" connector within an echo statements quoatation marks will still be removed from the output.
* Commands of the form `command#comment` will work, despite not being able to run in the original BASH shell.
* Currently, using connectors by themselves will send the program into an infinite loop.
* Using a chain of output redirectors ">" or ">>" will empty any file AFTER the first file; the first file will recieve
    the output of the command, but the rest of the files in the chain will be erased.
* There are two newlines printed instead of only one when rshell is called within rshell, and the ^C signal is sent.
* Color displaying is lost when changing directories.

###ls

* Typing the -l command followed by a file parameter will not list the file information, instead it will give an error.

###cp

* No known bugs.
