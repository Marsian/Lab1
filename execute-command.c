// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */
void deQuotion (char ** arg) {
	int i;
	int j;
	for ( i = 0; arg[i] != NULL; i++) {
		if (arg[i][0] == '\"') {
			for (j = 0; arg[i][j] != '\0'; j++) {
			   arg[i][j] = arg[i][j+1];
			}
			j = j - 2;
			arg[i][j] = '\0';
		}
	}   
}   

int
command_status (command_t c)
{
  return c->status;
}

void
execute_command (command_t c, int time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
	pid_t	child;
	int status;
	int fd[2];
	command_t cmd = c;

	switch (cmd->type) {
	case SIMPLE_COMMAND: {
		child = fork();
		if (child == 0) {
		        //handle redirects
			int fd_in;
			int fd_out;
			if (cmd -> input) {
			   	//cmd < file
				if ((fd_in = open(cmd->input, O_RDONLY, 0666)) == -1)
				   error(1, 0, "cannot open input file!");
				if ((dup2(fd_in, 0)) == -1)
				   error(1, 0, "cannot do input redirect!");
				printf("%s\n", cmd->input);
			}

			if (cmd -> output) {
			   	//cmd > file
				if ((fd_out = open(cmd->output, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
				   error(1, 0, "cannot open output file!");
				if ((dup2(fd_out, 1)) == -1)
				   error(1, 0, "cannot do output redirect!");
			}
			//handle execution
			deQuotion(cmd->u.word);
			execvp (cmd->u.word[0], cmd->u.word);
			error(1, 0, "cannot execute command!");
		
		} else if (child > 0) {
			//wait for the child process
			waitpid(child, &status, 0);
			//harvest the execution status
			cmd->status = status;
		} else
			error (1, 0, "cannot fork!");
		break;
	}
	case AND_COMMAND: {
		execute_command (cmd->u.command[0], time_travel);
		if (cmd->u.command[0] -> status == 0) {
		   //run the second commnad cmd2
		   execute_command (cmd->u.command[1], time_travel);
		   //set the status of the AND command
		   cmd->status = cmd->u.command[1]->status;
		} else {
		   //do not run cmd2
		   //set the status of the AND command
		   cmd->status = cmd->u.command[0]->status;
		}
		break;
	}
	case OR_COMMAND: {
		execute_command (cmd->u.command[0], time_travel);
		if (cmd->u.command[0] -> status != 0) {
		   //run the second commnad cmd2
		   execute_command (cmd->u.command[1], time_travel);
		   //set the status of the AND command
		   cmd->status = cmd->u.command[1]->status;
		} else {
		   //do not run cmd2
		   //set the status of the AND command
		   cmd->status = cmd->u.command[0]->status;
		}
		break;
	}

	case PIPE_COMMAND: {
	 	if ( pipe(fd) == -1)
		   error ( 1, 0, "cannot create pipe!");
		child = fork();
		if (child == 0) {
		   	// child writes to the pipe
			// cmd1
			close (fd[0]);
			if ( dup2(fd[1], STDOUT_FILENO) == -1)
			   error(1, 0, "cannot redirect output!");
			execute_command(cmd->u.command[0], time_travel);
			close (fd[1]);
			exit(cmd->u.command[0]->status);  
		}  else if (child > 0) {
		   	// parent read the pipe
			waitpid( child, &status, 0);
			cmd->u.command[0]->status = status;
			close(fd[1]);
			if ( dup2(fd[0], STDIN_FILENO) == -1)
			   error(1, 0, "cannot redirect input!");
			execute_command( cmd->u.command[1], time_travel);
			close(fd[0]);
			cmd->status = cmd->u.command[1]->status;
		} else
			error(1, 0, "cannot fork!");

		break;
	}   	
	
	case SEQUENCE_COMMAND: {
		child = fork();
		if (child == 0) {
			execute_command(cmd->u.command[0], time_travel);
		} else if (child > 0) {
			execute_command(cmd->u.command[1], time_travel);
		} else
			error(1, 0, "cannot fork!");
		break;
	}
	case SUBSHELL_COMMAND: {
		execute_command(cmd->u.subshell_command, time_travel);
		break;
	}
	default: abort();
	}
}
