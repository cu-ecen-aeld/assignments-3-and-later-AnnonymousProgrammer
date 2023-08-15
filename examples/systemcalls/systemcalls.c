#include "systemcalls.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
    int ret_val = system(cmd);
    return ret_val == 0;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
        printf("%s \n", command[i]);
    }
    command[count] = NULL;
    int pid = fork();
    if(pid == 0){
    	// child
    	execv(command[0], command);
    	exit(-1);
    }else if(pid != -1){
    	// father
    	int wstatus;
    	int wait_ret_val = waitpid(pid, &wstatus, 0);
    	if (wait_ret_val == -1){
    		return -1;
    	}else if(WIFEXITED(wstatus)){
    		return WEXITSTATUS(wstatus) == 0;
    	}else {
    		return -1;
    	}
    }else{
    	return false;
    }

    va_end(args);

    return true;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

/**


    int kidpid;
    int status;

    int fd = open(outputfile, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        abort();
    }

    switch (kidpid = fork()) {
        case -1:
            perror("fork");
            abort();
        case 0:
            if (dup2(fd, 1) < 0) {
                perror("dup2");
                abort();
            }
            close(fd);
            execvp(command[0], command);
            perror("execvp");
            abort();
        default:
            if (waitpid(kidpid, &status, 0) == -1) {
                return false;
            }
            else if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                return true;
            }
    }

    va_end(args);

    return true;

	if (dup2(outfile_desc, 1) < 0) {
                perror("dup2");
                abort();
            }
            close(outfile_desc);
            execvp(command[0], command);
            exit(-1);


**/





    int outfile_desc = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(outfile_desc == -1) {
    		exit(-1);
    }
    int pid = fork();
    if(pid == 0){
    	// child
    	if(dup2(outfile_desc, 1) < 0){
    		exit(-1);
    	}
    	close(outfile_desc);
    	execv(command[0], command);
    	exit(-1);
    }else if(pid != -1){
    	// father
    	int wstatus;
    	int wait_ret_val = waitpid(pid, &wstatus, 0);
    	if (wait_ret_val == -1){
    		return -1;
    	}else if(WIFEXITED(wstatus)){
    		return WEXITSTATUS(wstatus) == 0;
    	}else {
    		return -1;
    	}
    }else{
    	return false;
    }

    va_end(args);

    return true;
}

/**
int main(){

	bool ret = do_exec(3, "/usr/bin/test","-f","/bin/echo");
	if(ret){
		printf("success\n");
	}else{
		printf("fail\n");
	}
	return 0;
}
**/

