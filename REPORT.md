# Project 1 - Simple Shell

## Overview

In this project, we were tasked with creating a simple shell.
The shell needed to be able to execute user-supplied commands with optional 
arguments, have a selection of builtin commands exit, pwd, and cd, redirection
 of the standard output to files, and the composition of commands via piping.

## SyntaxChecking

Syntax checking is a function that makes sure that the commands that is input 
is in the valid format for the built in commands and piping. For example, if we
call the **ls** command, the '`syntaxchecking` function first checks to see if 
the input is one of the builtin commands that the project required (ie, **cd**,
**ls**, **pwd**, etc), then `syntaxchecking` will then check to see if the 
additional arguments for **ls** is being used properly. In the case of **ls**
we can see if the pipe `|` is behind **ls**. We made a syntax checking function
and called it at towards the beginning of `main` in order to catch any errors
regarding the builtin functions before we even run them.

## Exit

`Exit` is fairly straight forward and just exits when we exit shell when we
call it.

## Cd

For `cd` we decided to make a copy of **cmd** so we do not alter **cmd** in 
any way. Then we split the copy of **cmd** using `" "` as a delimiter,
putting it into `tokens`. `tokens` is basically a list of strings, it lets
us keep the various arguments for any command. We used `strtok` as we felt
that it would be easier to implement and process the string compared to 
parsing and putting the strings inside a struct or other list. Then when
we want the next argument or flag we can use `strtok(NULL," ");`. For example,
if the command is "cd test", `strtok(coppy," ");` first gives is **cd** while
`strtok(NULL," ");` gives us "test". When we then use the `chdir` function, the
argument used is "test".

## Pwd

For **pwd** we simply call the `getcwd` function in order to get the path of
the working directory. We then put the path into `buff` where the size is 512,
the max size of the command line.

## Sls

For `Sls` we first try to see if we can even open the directory by seeing if
the directory is **NULL** ifthe directory is **NULL**, we send an error. If we
can open the directory, we use `readdir` and we use `strcmp` to ignore files
with **.** or **..**. If we cannot get the information name, we also ignore
that file. Else, we print the file and its file size. We then make sure to
close the directory to prevent any errors.

## Outputdirection

For `Outputdirection` we are basically making sure we know which direction to
call arguments whenever we see a **>** symbol. The first **for** loop is used
to get rid of any hanging newlines, instead replacing them with a space. Then
we tokenize the copy of cmd into `step`. While `step` is not empty we do
the various terminal commands found in `step`. We have a special case to handle
the **echo** command. We check the next argument after **echo** to see if the
original `report` had a newline, if it does, we add it back to `step`. We also
check to see if we need to append the data to the file by using `strcmp` to
check for **>>**. If we do find **>>**, we find the location of **<<**, get the
output file name, then write it in next line. If the file does not need to be
appended, we use `dup2` to redirect the file descriptor into the correct file
output.

## Piping

For piping we use `dup2` in order to chain the output of one command as the 
input of another command. We create an array of file descriptors of size two,
then we use `fork` so that the child uses `pipe_fd[0]` as the old file 
descriptor and `STDIN_FILENO` as the new file descriptor. The parent process
uses `pipe_fd[1]` as the old file descriptor and `STDOUT_FILENO` as the new
file descriptor.

## fork_exec_wait

We used `fork_exec_wait` in order to replace the `system` function. The basis
of the code was modified from lecture 03 *syscalls* from the *fork_exec_wait.c*
example code. However, the exec function `execv()` was replaced with `execvp()`
since we needed to automatically search programs in the **$PATH**. The output
of `fork_exec_wait()` was also an into which was the `WEXITSTATUS(status)` in
order to check to see if the command was actually in **$PATH**.

## main

`main()` was modified from the skeleton code that was provided to us. However,
before we call `fork_exec_wait` we first check to see if the cmd is valid for
any of our built in functions. We then call those functions instead of 
`fork_exec_wait`. Then we create a copy of cmd so as to not alter it and check
to see if we need to call `Outputdirection` or `Piping`.

## Sources

Most of our sources were used to figure out how to implement the various
commands.

## Testing

Most of our testing was done via the autograder as well as manually running
the code on the CSIF. A GDB debugging was used to find some bugs that happened
with the fork processes.
