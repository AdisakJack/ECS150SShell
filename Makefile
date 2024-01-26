sshell: sshell.o

	gcc -O2 -Wall -Wextra -Werror -o sshell sshell.o

sshell.o: 

	gcc -c sshell.c

clean:

	rm -f sshell sshell.o


