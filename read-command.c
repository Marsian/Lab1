// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Node
{
  void * data;
  struct Node *next;
};

struct Node * pop (struct Node * head, void ** data)
{
  if (head == NULL) 
  {
      *data = NULL;
      return NULL;
  }
  else
  {
      struct Node * temp = head;
      *data = head->data;
      head = head->next;
      free(temp);
      return head;
  }
}

struct Node * push (struct Node * head, void * data)
{
  struct Node * temp;
  temp = ( struct Node *)malloc(sizeof (struct Node));
  
  if (temp == NULL)
  {
      exit(0);
  }

  temp -> data = data;
  temp -> next = head;
  head = temp;
  return head;
}

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */
typedef struct Node * command_stream;


command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  struct Node * stack;
  stack = NULL;
  int input;
  char c; 
  char tmp[120] = "\0";
  char comb[2];
  command_t temp;
  char * string;

  struct command * tree = NULL;
  struct Node * oStack = NULL;
  struct Node * dStack = NULL; 
  struct Node *  forest = NULL;

  input = get_next_byte (get_next_byte_argument);
  while ( input != EOF )
  {
      c = (char)input;
      printf("%c", c);
      switch (c)
      {
      case '(': 
         //printf("%s\n", tmp); 
         strcpy(tmp,"\0");
         break;
      case '>': 
         //printf("%s\n", tmp); 
         strcpy(tmp,"\0");
         break;
      case '<': 
         //printf("%s\n", tmp); 
         strcpy(tmp,"\0");
         break;
      case '|': 
         //printf("%s\n", tmp); 
         strcpy(tmp,"\0");
         break;
      case '&': 
         //printf("%s\n", tmp); 
         temp = ( struct command *)malloc(sizeof (struct command));
             
         temp->type = SIMPLE_COMMAND;
         temp->status = 1;
         temp->input = 0;
         temp->output = 0;
         string = malloc(sizeof (tmp));
         strcpy(string, tmp);
         temp->u.word = &string;
         strcpy(tmp, "\0");
         dStack = push(dStack, temp);
         input = get_next_byte (get_next_byte_argument);
         if ( input != '&')
         {
            exit(1);
            printf("& error\n");
         }
         else
         {
             temp = ( struct command *)malloc(sizeof (struct command));
             
             temp->type = AND_COMMAND;
             temp->status = 1;
             temp->input = 0;
             temp->output = 0;
             
             oStack = push(oStack, temp);
             
         }
         strcpy(tmp,"\0");
         break;
      case ';': 
         //printf("%s\n", tmp); 
         strcpy(tmp,"\0");
         break;
      case '\n': 
         temp = ( struct command *)malloc(sizeof (struct command));
             
         temp->type = SIMPLE_COMMAND;
         temp->status = 1;
         temp->input = 0;
         temp->output = 0;
         string = malloc(sizeof (tmp));
         strcpy(string, tmp);
         temp->u.word = &string;
         dStack = push(dStack, temp);
         strcpy(tmp,"\0");
         break;
      case '#':
         //printf("%s\n", tmp);
         strcpy(tmp,"\0");
         while (input != '\n')
         {
         input = get_next_byte (get_next_byte_argument);
         }
         break;
      default: 
         comb[0] = c;
         comb[1] = '\0';
         strcat(tmp, comb);
      }
      input = get_next_byte (get_next_byte_argument);
  }  
 
  error (1, 0, "command reading not yet implemented");
  return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}
