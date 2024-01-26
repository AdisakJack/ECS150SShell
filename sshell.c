#include <dirent.h>
#include <fcntl.h> 
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>




#define CMDLINE_MAX 512

void sytaxchecking(char *cmd){

        char coppy[CMDLINE_MAX];

        /* two variable extra to use strtok or strstr */
        char *token;
        char *check; 
        strcpy (coppy, cmd);   
        /* starts checking each syntax of comand */
        if (!strncmp(cmd, "exit",4)) {

                /* checking if some word behind exit */
                token = strtok(coppy," ");   
                token = strtok(NULL, " ");
                if (token != NULL) {
                    fprintf(stderr, "Error: too many process arguments\n");
                }

        }        
        else if (!strncmp(cmd, "ls",2)) {
                /* checking what type of words behind ls */
                token = strtok(coppy," ");
                token = strtok(NULL, " ");
                if (token != NULL) {
                        strcpy (coppy, cmd);
                       /* passing error when pipe is behind ls */
                        if(strstr(coppy, "|") == NULL)
                                fprintf(stderr, "Error: too many process arguments\n");
                }
        }

        else if (!strncmp(cmd, ">",1)) {
                /* > cannot be at the start */
                fprintf(stderr, "Error: missing command\n");
        }

        else if (!strncmp(cmd, "|",1)|| !strncmp(cmd, " |",2)) {    
                /* pipe cannot be at the first */
                fprintf(stderr, "Error: missing command\n");

        }

        /* get ready to use strstr */
        strcpy (coppy, cmd);     
        check = strstr(coppy, ">>");
        if (check!=NULL){

            /* if no wards are behind >> or >, it will be error */
            if(*(check +2) == '\0')
                fprintf(stderr, "Error: no output file\n");
            else if(strstr(check,"|")!=NULL)
                fprintf(stderr, "Error: mislocated output redirection\n");

        }

        strcpy (coppy, cmd);     
        check = strstr(coppy, ">");
        if (check!=NULL){
            if(*(check +1) == '\0')
                fprintf(stderr, "Error: no output file\n");

            else if(strstr(check,"|")!=NULL)
                fprintf(stderr, "Error: mislocated output redirection\n");

        }        

        strcpy (coppy, cmd);     
        check = strstr(coppy, "|");

        /* something should be behind of pipe to excute. */
        if (check!=NULL&&*(check +1) == '\0'){
            fprintf(stderr, "Error: missing command\n");

        }

}

/* exit command */
void Exit(char *cmd, int retval){

        fprintf(stderr, "Bye...\n+ completed '%s' [%d]\n", cmd, retval);
        exit(0);

}

/* cd command */
void Cd(char *cmd, int *retval){

        /* setting path for chdir */
        // printf("Hello cd received");
        char coppy[CMDLINE_MAX];
        strcpy (coppy, cmd);
        char *token;
        token = strtok(coppy," ");
        token = strtok(NULL," ");
        if(token != NULL)
        {
             if(chdir(token)!=0){
                *retval = 1;
               fprintf(stderr, "Error: cannot cd into directory '%s' [%d]\n", cmd, *retval);
                
                  }
        }
        else{
                 fprintf(stderr, "Error: missing argument for cd\n");
                *retval = 1;
        }
       
        fprintf(stderr, "\n+ completed '%s' [%d]\n", cmd, *retval);

}

/* pwd command */
void Pwd(char *cmd, int retval){
        // printf("Hello pwd received");
        char buff[CMDLINE_MAX];
        getcwd(buff, CMDLINE_MAX);
        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, retval);

}

void Sls(char *cmd, int retval){
        DIR *Opendirectory;
        struct dirent *directory;
        Opendirectory = opendir(".");
        /* if we cannot open directory, error happens */
        if (Opendirectory == NULL) {
                perror("Error: cannot open directory");
                return;

        }

        while ((directory = readdir(Opendirectory)) != NULL) {

                /* ignoring the file with . or ..*/
                if (strcmp(directory->d_name, ".") == 0 || strcmp(directory->d_name, "..") == 0)
                        continue;
                struct stat file_info;

                /* if it canot get information name, ignore it. */
                if (stat(directory->d_name, &file_info) != 0) {
                        continue;
                }

                fprintf(stdout, "%s (%" PRIu64 " bytes)\n", directory->d_name, (uint64_t)file_info.st_size);
        }

        closedir(Opendirectory);
        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, retval);
}

void Outputdirection(char *cmd, int retval) {
    // Save the original stdout file descriptor
    int original_stdout = dup(STDOUT_FILENO);

    /* get ready for using strstr */
    char coppy[CMDLINE_MAX];
    char *token;
    char *check;
    strcpy(coppy, cmd);
    check = strstr(coppy, "<<");

    /* if this file should be appended  */
    if (check != NULL) {
        /* finding location of <<, get the output file name, then write in the next line.  */
        strcpy(coppy, cmd);
        token = strstr(coppy, "<<");
        token += 3;

        // Redirect stdout to the specified file in "append" mode
        freopen(token, "a", stdout);

        // Close the redirected stdout
        fclose(stderr);
    } else {
        /* if this file should not be appended  */
        strcpy(coppy, cmd);
        token = strtok(cmd, "<");
        token = strtok(NULL, " ");

        // Open the file for writing (creating if it doesn't exist)
        int file = open(token, O_WRONLY | O_CREAT, 0666);
        if (file == -1) {
            perror("Error opening file");
            fprintf(stderr, "file open error");
        } else {
            // Redirect stdout to the specified file
            dup2(file, STDOUT_FILENO);
            close(file);
        }
    }

    // Restore the original stdout file descriptor
    dup2(original_stdout, STDOUT_FILENO);
    close(original_stdout);
}

void Piping(char *cmd, int retval) {
    int pipe_fd[2];
    pid_t child_pid;
    pipe(pipe_fd);
    child_pid = fork();

    if (child_pid == 0) {  // Child
        dup2(pipe_fd[0], STDIN_FILENO);

    } else {  // Parent

        dup2(pipe_fd[1], STDOUT_FILENO);
        wait(NULL);
        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, retval);
    }
}

int fork_exec_wait(char cmd[CMDLINE_MAX]) {
  int status = -1;
  pid_t pid;
  // Tokenize the command and arguments
    char *args[CMDLINE_MAX];
    char *token = strtok(cmd, " ");
    int i = 0;

    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }

    args[i] = NULL;  // Null-terminate the array

//   char *args[] = {cmd, NULL};
  pid = fork();
  if (pid == 0) {
    /* Child */
    execvp(args[0], args);
    perror("execvp");
    exit(1);

  } else if (pid > 0) {
    /* Parent */
    waitpid(pid, &status, 0);
   // printf("Child returned %d\n", WEXITSTATUS(status));
  } else {
    perror("fork");
    exit(1);
  }
  return WEXITSTATUS(status);
}

int main(void)
{
    char cmd[CMDLINE_MAX];
    while (1) {

            char *nl;
            int retval;
            char checkingRdirectAndPiping[CMDLINE_MAX];
        //     cmd[0] = '\0';
            /* Print prompt */
            printf("sshell$ ");
            fflush(stdout);


          if (fgets(cmd, CMDLINE_MAX, stdin) == NULL) {
            // Check for errors or end-of-file
            perror("Error reading command");
            exit(EXIT_FAILURE);
        }
            /* Get command line */
            fgets(cmd, CMDLINE_MAX, stdin);

        //  printf("finally command is now %s", cmd);




            /* Print command line if stdin is not provided by terminal */
            if (!isatty(STDIN_FILENO)) {

                    printf("%s", cmd);
                    fflush(stdout);

            }

            /* Remove trailing newline from command line */
            nl = strchr(cmd, '\n');
            if (nl)

                    *nl = '\0';

            /* Builtin command */
            sytaxchecking(cmd);
            if (!strcmp(cmd, "exit")) {
                    Exit(cmd, retval);
            }

            else if (!strcmp(cmd, "pwd")){
                    Pwd(cmd,retval);
            }

            else if (!strncmp(cmd, "cd",2)) {
                    Cd(cmd, &retval);
            }

            else if (!strcmp(cmd, "sls")){
                    Sls(cmd, retval);
            }
            else{
                  /* Regular command */
            retval = fork_exec_wait(cmd);
            strcpy (checkingRdirectAndPiping, cmd);   
            }
                    

            if (strtok(checkingRdirectAndPiping,"<")!=NULL){
                    Outputdirection(cmd, retval);
            }
         
            strcpy (checkingRdirectAndPiping, cmd);
            if(strtok(checkingRdirectAndPiping,"|")!=NULL){

                    Piping(cmd, retval);

            }

    }

    return EXIT_SUCCESS;

}

/*visited websites
https://stackoverflow.com/questions/1442116/how-to-get-the-date-and-time-values-in-a-c-program
https://www.studytonight.com/c/programs/misc/display-current-date-and-time#:~:text=C%20Program%20to%20Display%20the,hh%3Amm%3Ass%20yyyy.
https://stackoverflow.com/questions/298510/how-to-get-the-current-directory-in-a-c-program
https://stackoverflow.com/questions/67251953/implement-cd-command-in-c
https://iq.opengenus.org/implementing-cd-command-in-c/
https://stackoverflow.com/questions/19467865/how-to-use-redirection-in-c-for-file-input
https://hand-over.tistory.com/39
https://m.blog.naver.com/sharonichoya/220490957748
https://www.youtube.com/watch?v=jnJL6ppn26Q  makefile example.
https://blockdmask.tistory.com/389 strchr example.
https://m.blog.naver.com/sharonichoya/220490957748 strtok() example.
https://k-story.tistory.com/243 opendir(), closedir(), readdir()
https://bodamnury.tistory.com/38 struct stat
https://pubs.opengroup.org/onlinepubs/7908799/xsh/dirent.h.html d_name() for dirent 
https://www.ibm.com/docs/ko/aix/7.3?topic=files-stath-file st_size for bytes
https://www.cs.utexas.edu/~theksong/posts/2020-08-30-using-dup2-to-redirect-output/ dup2()
https://badayak.com/entry/C%EC%96%B8%EC%96%B4-%ED%8C%8C%EC%9D%BC-%EC%97%B4%EA%B8%B0-%ED%95%A8%EC%88%98-open open()
https://zoosso.tistory.com/930 freeopen()
https://m.blog.naver.com/neakoo35/30133294306 detail of freeopen()
https://velog.io/@hamys96/pipex2-%ED%8C%8C%EC%9D%B4%ED%94%84-%EA%B5%AC%ED%98%84 pipe(); pid1 = fork();
https://nomad-programmer.tistory.com/110 concept for pipe.
*/


