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

struct depend {
   	command_t tree;
	char ** input;
	char ** output;
	Node * to;
	Node * from; 
	int status;
};

struct depend *  peekD(Node * s)
{
	return s->data;
}

/*void commandTraverse( struct command * p)
{
  if (p != NULL)
  {
      if (p->type == SUBSHELL_COMMAND)
      {
         commandTraverse(p->u.subshell_command);
      }
      else if (p->type != SIMPLE_COMMAND)
      {
         commandTraverse(p->u.command[0]);
      }

      if (p -> type == SIMPLE_COMMAND) 
      {
	char **w = p->u.word;
	while (*++w) { 
	 	if ((*w)[0] != '-') {
		   Node * tmp = (Node *)malloc(sizeof(Node));
		   tmp -> data = (char *)malloc(sizeof(*w));
		   strcpy(tmp->data, *w);
		   dep->input = push(dep->input, tmp);
		}
	}
  	if (p->input) {
		   Node * tmp = (Node *)malloc(sizeof(Node));
		   tmp -> data = (char *)malloc(sizeof(*w));
		   strcpy(tmp->data, *w);
		   dep->input = push(dep->input, tmp);
	}
  	if (p->output) {
		   Node * tmp = (Node *)malloc(sizeof(Node));
		   tmp -> data = (char *)malloc(sizeof(*w));
		   strcpy(tmp->data, *w);
		   dep->output = push(dep->output, tmp);
	}
      }

      if (p->type != SIMPLE_COMMAND)
      {
         commandTraverse(p->u.command[1]);
      }
  }
}*/
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

void deleteEdge (struct Node ** s, command_t cmd)
{
   	Node * it, * jt;
	command_t tmp;
	
	if (*s == NULL) return;
	if (peek(*s) == cmd) {
	   *s = pop(*s, &tmp);
	   return;
	}

	for (it = *s; it->next != NULL; it = it -> next) {
		if (peek(it->next) == cmd) {
			jt = it->next;
			it->next = jt->next;		
		}
	}
}


void execute_parallel_command (struct Node * s)
{
   	command_t command;
	Node * stack = NULL;
	Node * stream = s;
	Node * dependForest = NULL;
	struct depend * dep = NULL;
	while ((stream)) {
	   	command = (stream)->data;
		dep = (struct depend *)malloc(sizeof(struct depend));
		dep->tree = command;
		dep->status = 0;
		dep->input = (char **)malloc(sizeof(char *)*50);
		dep->output = (char **)malloc(sizeof(char *)*50);
		char ** intmp, ** outtmp;
		intmp = dep->input;
		outtmp = dep->output;
		//printf("%d\n", command->type);

		//Traverse the command tree
		stack = push(stack, command);
		while (stack != NULL) {
			struct command * com;
			stack = pop(stack, &com);
		        if (com->type == SIMPLE_COMMAND) {
			   char **w = com->u.word;
			   //printf("%s\n", *w);
			   while (*++w) { 
			      if ((**w) != '-') {
				      *(dep->input) = (char *)malloc(sizeof(char)*120);
				      strcpy(*(dep->input), *w);
				      dep->input++;
			      }
			   }	
			   if (com->input) {
				  *(dep->input) = (char *)malloc(sizeof(char)*120);
				  strcpy(*(dep->input), com->input);
				  dep->input++;
			   }
			   if (com->output) {
				  *(dep->output) = (char *)malloc(sizeof(char)*120);
				  strcpy(*(dep->output), com->output);
				  dep->output++;
			   }
			} else if (com->type == SUBSHELL_COMMAND) {
				stack = push(stack, com->u.subshell_command);	
			} else {
	 			stack = push(stack, com->u.command[0]);
				stack = push(stack, com->u.command[1]);
			}	
		}

		*(dep->input) = NULL;
		*(dep->output) = NULL;
		dep->input = intmp;
		dep->output = outtmp;

		dependForest = push(dependForest, dep);
		stream = (stream) -> next;		
	}
	
	Node * it = dependForest;
	for (; it != NULL; it = it->next) {
		char ** k;
		char ** h;
		/*if (*k)printf("%s\n", *k);
		while(*++k) {
			printf("%s\n", *k);
		}
		//printf("%s\n",*(peekD(it)->input));
		*/
		Node * jt = dependForest;
		for (; jt != it && jt != NULL; jt = jt->next) {
			 h = peekD(jt)->output;
			 k = peekD(it)->input;
			 for (; *h != NULL; h++) {
				for (; *k != NULL; k++) {
					if (strcmp(*k, *h) == 0) {
						peekD(jt)->to = push(peekD(jt)->to, peekD(it)->tree);
						peekD(it)->from = push(peekD(it)->from, peekD(jt)->tree);  
					}	
				}
			 }
		}
	}

        int flag = 1;
	while (flag) {
	   	flag = 0;
		Node * stack = NULL, * tmp, * pidStack = NULL;
		command_t cmd;
		Node * jt, * kt;

		// Find all runnable processes, push them to the stack
		// Set status of the processed to be runned
		for (it = dependForest; it != NULL; it = it -> next) {
			if (peekD(it)->from == NULL && peekD(it)->status == 0) {
				flag = 1;
				peekD(it)->status = 1;
				stack = push(stack, peekD(it)->tree);
			}
		}

		// Run all the runnable processes in parallel
		for (kt = stack; kt != NULL; kt = kt -> next) {
		   	pid_t * child = (pid_t *) malloc(sizeof(pid_t));
			*child = fork();
			if (*child == 0) {
				execute_command(peek(kt), 1);
				exit(0);
			} else {
			   	pidStack = push(pidStack, child);
				//waitpid(child, 0, 0);
		 	}
		}

		// Wait for all the running command
		while (pidStack != NULL) {
		   	pid_t * child;
			pidStack = pop(pidStack, (command_t *)&(child));
			waitpid(*((pid_t *)child), 0, 0);
		}

		// Pop the stack. Cut out all the related edge
		while (stack != NULL) {
			stack = pop(stack, &cmd);
			for (jt = dependForest; jt != NULL; jt = jt->next) {
				deleteEdge(&(peekD(jt)->from), cmd);
			}
		}
		//break;
		
	}
}
