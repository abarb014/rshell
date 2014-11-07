Rshell
===

Licensing Information: READ LICENSE
---
Project source can be downloaded from https://github.com/abarb014/rshell
---

Author and Contributor List
---
Andrew Barboza

Report any bugs immediately to: abarb014@ucr.edu

File List
---
```
.:

Makefile

LICENSE

README.md

./src

./tests

```

```
/src:

main.cpp
```

```
/tests:

exec.script
```

To run this file
---
1. Clone the directory to your computer
2. `cd` into the `rshell` directory
3. Use `make` to compile the program
4. `cd` into the `bin` directory
5. Use `./rshell` to run the program. It will take over instead of the bash shell.

Features
---
* Can run many commands that the BASH shell can.
* Uses a combination of `fork()`, `execvp()`, and `wait()` to mimic the behaviour of the BASH shell.
* Supports connectors `&&`, `||`, and `;` to help the user chain commands together.
* Displays the users login name and host name, unless an error occurs. In the case of an error, your prompt will be `$ `.
* Has a custom built `exit` command in order to quit the program.

Known Bugs
---
* Typing multiple "&&" connectors with no spaces in between will not produce any errors.
* Typing multiple "||" connectors with no spaces in between will not produce any errors.
* Use of the "#" connector within an echo statements quoatation marks will still be removed from the output.
* Commands of the form `command#comment` will work, despite not being able to run in the original BASH shell.
* Currently, using connectors by themselves will send the program into an infinite loop.
* In ls.cpp, typing the -l command followed by a file parameter will not list the file information, instead it will give an error.
