// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*struct Node
{
  void * data;
  struct Node *next;
};*/

 int lineNumber=1;
 int thereserror =0;
 char wholeline[1000]="\0";
struct Node * pop (struct Node * head, command_t * data)
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

struct Node * deQueue (struct Node * head, command_t * data)
{
  if (head == NULL)
  {
     *data = NULL;
     return NULL;
  }
  else
  {
      struct Node * temp = head;
      if ( head -> next == NULL)
      {
          *data = head->data;
          free(head);
          return NULL;
      }
     
      while (temp->next->next != NULL)
      {
          temp = temp->next;
      }

      *data = temp->next->data;
      free(temp->next);
      temp->next = NULL;
      return head; 
  }

  return head;

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

struct Node *  addSimpleCommand ( struct Node * head,
                                  char * tmp)
{
  struct command * temp;
  char ** string;
  temp = malloc(sizeof (struct command));
             
  temp->type = SIMPLE_COMMAND;
  temp->status = 1;
  temp->input = 0;
  temp->output = 0;
  temp->u.word = malloc(sizeof (temp->u));
   
  int i = 0, a = 0, b = 0;
  char  tmpstr[120];
  while((tmp[i]==' ')||(tmp[i]=='\t'))
  {
     i++;
  }

  strcpy(tmpstr,tmp+i);
  i=0;
  strcpy(tmp,tmpstr);
  while(tmp[i]!='\0')
  {
     a = b;

     if ((tmp[i]==' ')||(tmp[i]=='\t'))
     {   
         b=1;
     }
     else b=0;

     if((a==1)&&(b==1))
     {
         strcpy(tmpstr,tmp+i);
	 strcpy(tmp+i-1,tmpstr);
         i--;	
     } 
     i++;
  }
  if ((i>0)&&(tmp[i-1]==' '))
  {
     tmp[i-1]='\0';
  }

  string = malloc(sizeof (char *));
  *string = malloc(sizeof (tmp));
  strcpy(*string, tmp);
  temp->u.word = string;
  head = push(head, temp);

  return head;
}

struct Node * redirection ( struct Node * head,
                            int tmp)
{
  struct command * temp1, * temp2;
  head = pop(head, &temp1);
  head = pop(head, &temp2);
if (temp2==NULL) {
     
     fprintf(stderr,"line %d: %s\n",lineNumber,wholeline);exit(1);

     }
 else if ( tmp == 1)
  {
     temp2->input = *(temp1->u.word);
  }
  else
  {
     temp2->output = *(temp1->u.word);
  }

  free(temp1);
  head = push(head, temp2);

  return head;
}

struct command * peek(struct Node * head)
{
  return head->data;
}

int searchParen ( struct Node * head)
{
    struct Node * tmp;
    tmp = head;
    if ( tmp == NULL) return 0;
    while ( tmp != NULL) {
        if ( (peek(tmp))->type == SUBSHELL_COMMAND) return 1;
        tmp = tmp->next;
    }

    return 0;
}


int isEmpty(char * tmp)
{
     while (*tmp != '\0')
     {
         if (*tmp != ' ')
         {
             return 0;
         }
         else
         {
             tmp ++;
         }
     }
     return 1;
}

void depthTraverse( struct command * p)
{
  if (p != NULL)
  {
      if (p->type == SUBSHELL_COMMAND)
      {
         printf("(");
         depthTraverse(p->u.subshell_command);
      }
      else if (p->type != SIMPLE_COMMAND)
      {
         depthTraverse(p->u.command[0]);
      }

      if (p -> type == SIMPLE_COMMAND) 
      {
          printf("%s", *(p->u.word));
      }
      else if (p -> type == SUBSHELL_COMMAND)
      {
          printf("%d", p->type);
      }
      else
      {
          printf("%d", p->type);
      }

      if (p->type != SIMPLE_COMMAND)
      {
         depthTraverse(p->u.command[1]);
      }
  }
}


/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */
typedef struct Node * command_stream;

struct Node **
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  struct Node * stack;
  stack = NULL;
  int input;
//  int lineNumber = 1;
  char c; 
  char tmp[120] = "\0";
  char comb[2];
  char ** string;
//  char  wholeline[1000] = "\0";
  string = NULL;
  int metRedirection = 0;
  int inSubshell = 0;
//  int thereserror=0;
  int paren=0;
  struct command * temp;
  struct command * com; 
  struct command * com1;
  struct command * com2;
  struct command * tree = NULL;
  struct Node * oStack = NULL;
  struct Node * dStack = NULL; 
  struct Node ** forest;
  forest = malloc(sizeof(struct Node **));
  *forest = malloc(sizeof(struct Node *));
  *forest =  NULL;

  input = get_next_byte (get_next_byte_argument);
  while ( input != EOF )
  {
      c = (char)input;
      if ((c == '`') || (c == '\'')|| (c == '\t')) thereserror ++;
      comb[0]=c;
      comb[1]='\0';
      if (c!= '\n')strcat(wholeline,comb);
      
      //putchar (c);
      switch (c)
      {
      case '(': 
         //printf("%s\n", tmp); 
         //if ( tmp == "\0")i
         paren++;
         if ((input=get_next_byte (get_next_byte_argument))=='(') {
            thereserror++;
            comb[0]=input;
            comb[1]='\0';
            strcat(wholeline,comb);break;}
         else {
             input = ungetc (input, get_next_byte_argument);
             }
         strcpy(tmp,"\0");
         temp = malloc(sizeof (struct command));
         
         temp->type = SUBSHELL_COMMAND;
         temp->status = 1;
         temp->input = 0;
         temp->output = 0;

         oStack = push(oStack, temp);
         //printf("find a (\n");
         break;
      case ')':
         if (paren == 0) {thereserror++; break;}
         else paren--;
         if ((input=get_next_byte (get_next_byte_argument))==')') {
            thereserror++;
            comb[0]=input;
            comb[1]='\0';
            strcat(wholeline,comb);break;}
         else {
             input = ungetc (input, get_next_byte_argument);
             }
         dStack = addSimpleCommand(dStack, tmp); 
         //printf("COM: %s\n", *(peek(dStack))->u.word);
         strcpy(tmp,"\0");

         if (metRedirection > 0)
         {
             dStack = redirection(dStack, metRedirection);
             metRedirection = 0;
         }

         while ((peek(oStack))->type != 5 )
         {
		dStack = pop(dStack, &com2);
		dStack = pop(dStack, &com1);
		oStack = pop(oStack, &com);
        // printf("\nOper: %d\n", com->type);
		com->u.command[0] = malloc(sizeof(com->u));
		com->u.command[1] = malloc(sizeof(com->u));
		com->u.command[0] = com1;
		com->u.command[1] = com2;
		dStack = push(dStack, com);
         }
         
         oStack = pop(oStack, &com);
         dStack = pop(dStack, &com1);
         com->u.subshell_command = malloc(sizeof(com->u));
         com->u.subshell_command = com1;
         //printf("\nOperator: %d\n", com1->type);
         //printf("\nTTT\n");
         //depthTraverse(com);
         //printf("\nTTT\n");
       
         dStack = push(dStack, com);

         //printf("\nOperator: %d\n", (peek(dStack))->type);
         break;
      case '>': 
         if ((input=get_next_byte (get_next_byte_argument))=='>') {
            thereserror++;
            comb[0]=input;
            comb[1]='\0';
            strcat(wholeline,comb);break;}
         else {
             input = ungetc (input, get_next_byte_argument);
             }
         if (isEmpty(tmp)==0) {
            dStack = addSimpleCommand(dStack, tmp); 
            strcpy(tmp,"\0");}
         else {
            if (dStack == NULL) {thereserror++;break;} 
            else if ((peek(dStack)->type)!=5) {thereserror++;break;}
             }
             //fprintf(stderr, "%d: Incorrect syntax: >\n\n", lineNumber);
             //exit(0);
             //printf("Why>\n");
         if (metRedirection > 0)
         {
             dStack = redirection(dStack, metRedirection);
             metRedirection = 0;
         }
         metRedirection = 2;
         break;
      case '<': 
         if ((input=get_next_byte (get_next_byte_argument))=='<') {
            thereserror++;
            comb[0]=input;
            comb[1]='\0';
            strcat(wholeline,comb);break;}
         else {
             input = ungetc (input, get_next_byte_argument);
             }
         if (isEmpty(tmp)==0) {
            dStack = addSimpleCommand(dStack, tmp); 
            strcpy(tmp,"\0");}
         else {
            if (dStack == NULL) {thereserror++;break;} 
            else if ((peek(dStack)->type)!=5) {thereserror++;break;}
             }
         //if (isEmpty(tmp)==0) {
         //   dStack = addSimpleCommand(dStack, tmp); 
         //   strcpy(tmp,"\0");
         //}   
         if (metRedirection > 0)
         {
             dStack = redirection(dStack, metRedirection);
             metRedirection = 0;
         }
         metRedirection = 1;
         break;
      case '|':
         if (isEmpty(tmp)==0) {
            dStack = addSimpleCommand(dStack, tmp); 
            strcpy(tmp,"\0");}
         else {
            if (dStack == NULL) {thereserror++;break;} 
            else if ((peek(dStack)->type)!=5) {thereserror++;break;}
             }
         //if ( isEmpty(tmp) == 0)
         //{ 
         //    dStack = addSimpleCommand(dStack, tmp); 
         //    strcpy(tmp, "\0");
        // }

         if (metRedirection > 0)
         {
             dStack = redirection(dStack, metRedirection);
             metRedirection = 0;
         }

         input = get_next_byte (get_next_byte_argument);
         if ( input != '|')
         {
             /*c = (char)input;
             comb[0] = c;
             comb[1] = '\0';
             strcat(tmp, comb);
             */
             input = ungetc (input, get_next_byte_argument);
             temp = malloc(sizeof (struct command));
            
             temp->type = PIPE_COMMAND;
             temp->status = 1;
             temp->input = 0;
             temp->output = 0;
            
             if (oStack == NULL)
             { 
                oStack = push(oStack, temp);
             }
             else if ((peek(oStack))->type < 3 || (peek(oStack))->type == 5)
             {
                oStack = push(oStack, temp);
             }
             else 
             {
                dStack = pop(dStack, &com2);
                dStack = pop(dStack, &com1);
                oStack = pop(oStack, &com);
                com->u.command[0] = malloc(sizeof(com->u));
                com->u.command[1] = malloc(sizeof(com->u));
                com->u.command[0] = com1;
                com->u.command[1] = com2;
                dStack = push(dStack, com);
                oStack = push(oStack, temp);
             }
         }
         else
         {

         comb[0]=input;
         comb[1]='\0';
         strcat(wholeline,comb);
         input = get_next_byte (get_next_byte_argument);
         if (input == '|'){
            thereserror++;
            
            comb[0]=input;
            comb[1]='\0';
            strcat(wholeline,comb);
            break;
         }
         else 
             input = ungetc (input, get_next_byte_argument);
         /*while (dStack != NULL)
         {
             dStack = pop(dStack, &com2);
             
             printf("\nStack Traverse\n");
             depthTraverse(com2);
             printf("\nEnd\n");
         }*/
             temp = malloc(sizeof (struct command));
             
             temp->type = OR_COMMAND;
             temp->status = 1;
             temp->input = 0;
             temp->output = 0;
            
             if (oStack == NULL)
             { 
                oStack = push(oStack, temp);
             }
             else if ((peek(oStack))->type == 1)
             {
                oStack = push(oStack, temp);
             }
             else 
             {
                while ( oStack != NULL && (peek(oStack))->type != 1 )
                {
                        if ((peek(oStack))->type == 5) break;
			dStack = pop(dStack, &com2);
			dStack = pop(dStack, &com1);
			oStack = pop(oStack, &com);
			com->u.command[0] = malloc(sizeof(com->u));
			com->u.command[1] = malloc(sizeof(com->u));
			com->u.command[0] = com1;
			com->u.command[1] = com2;
			dStack = push(dStack, com);
                }
                oStack = push(oStack, temp);
             }
         }
         break;
      case '&': 
         if (isEmpty(tmp)==0) {
            dStack = addSimpleCommand(dStack, tmp); 
            strcpy(tmp,"\0");}
         else {
            if (dStack == NULL) {thereserror++;break;} 
            else if ((peek(dStack)->type)!=5) {thereserror++;break;}
             }
         //if ( isEmpty(tmp) != 1)
         //{
         //    dStack = addSimpleCommand(dStack, tmp);
         //    strcpy(tmp, "\0");
         //}
         if (metRedirection > 0)
         {  // printf("fuck up here\n");
             dStack = redirection(dStack, metRedirection);
             metRedirection = 0;
         }

         input = get_next_byte (get_next_byte_argument);
         if ( input != '&')
         {
           // printf("%d syntax & error\n", lineNumber);
           // depthTraverse(peek(dStack));
           // exit(1);
           
            thereserror++;
             input = ungetc (input, get_next_byte_argument);
            break;
         }
         else
         {
         comb[0]=input;
         comb[1]='\0';
         strcat(wholeline,comb);
             temp = malloc(sizeof (struct command));
             
             temp->type = AND_COMMAND;
             temp->status = 1;
             temp->input = 0;
             temp->output = 0;
             
             if (oStack == NULL)
             { 
                oStack = push(oStack, temp);
             }
             else if ((peek(oStack))->type == 1)
             {
                oStack = push(oStack, temp);
             }
             else 
             {
                while ( oStack != NULL && (peek(oStack))->type != 1 )
                {
                        if ((peek(oStack))->type == 5) break;
			dStack = pop(dStack, &com2);
			dStack = pop(dStack, &com1);
			oStack = pop(oStack, &com);
			com->u.command[0] = malloc(sizeof(com->u));
			com->u.command[1] = malloc(sizeof(com->u));
			com->u.command[0] = com1;
			com->u.command[1] = com2;
			dStack = push(dStack, com);
                }
                oStack = push(oStack, temp);
             }
         }
       // printf("stack top after AND is:\n");
       // depthTraverse(peek(dStack));
         break;
      case ';': 
         if ((input=get_next_byte (get_next_byte_argument))==';') {
            thereserror++;
            comb[0]=input;
            comb[1]='\0';
            strcat(wholeline,comb);break;}
         else {
             input = ungetc (input, get_next_byte_argument);
             }
         if (isEmpty(tmp)==0) {
            dStack = addSimpleCommand(dStack, tmp); 
            strcpy(tmp,"\0");}
         else {
            if (dStack == NULL) {thereserror++;break;} 
            else if ((peek(dStack)->type)!=5) {thereserror++;break;}
             }
         //if ( isEmpty(tmp) != 1)
         //{
         //    dStack = addSimpleCommand(dStack, tmp);
         //    strcpy(tmp, "\0");
         //}

         if (metRedirection > 0)
         {
             dStack = redirection(dStack, metRedirection);
             metRedirection = 0;
         }

         temp = malloc(sizeof (struct command));
         
         temp->type = SEQUENCE_COMMAND;
         temp->status = 1;
         temp->input = 0;
         temp->output = 0;

        /* if (oStack == NULL)
         { 
            oStack = push(oStack, temp);
         }*/
        // else 
        // {
                 if (searchParen(oStack)) {
		     while ( oStack != NULL) 
	             {
                        if ((peek(oStack))->type == 5) break;
				 dStack = pop(dStack, &com2);
				 dStack = pop(dStack, &com1);
				 oStack = pop(oStack, &com);
				 com->u.command[0] = malloc(sizeof(com->u));
				 com->u.command[1] = malloc(sizeof(com->u));
				 com->u.command[0] = com1;
				 com->u.command[1] = com2;
				 dStack = push(dStack, com);
		     }
		     oStack = push(oStack, temp);
                 } else {
			 while (oStack != NULL)
			 {
				dStack = pop(dStack, &com2);
				dStack = pop(dStack, &com1);
				oStack = pop(oStack, &com);
				
				com->u.command[0] = malloc(sizeof(com->u));
				com->u.command[1] = malloc(sizeof(com->u));
				com->u.command[0] = com1;
				com->u.command[1] = com2;
				dStack = push(dStack, com);
			 }
			 dStack = pop(dStack, &com);
			 *forest = push(*forest, com);
			 break;
                 }
        // }
         
         break;
      case '\n':
         if (thereserror)
          { // printf("test-p-bad.sh: line 1: syntax error near unexpected token `newline'\n");printf("test-p-bad.sh: line 1: ``'\n");exit(1);

            fprintf(stderr,"line %d: %s\n",lineNumber,wholeline);exit(1);
          }
         else{int i=0;
              while (wholeline[i]!='\0'){wholeline[i]='\0';i++;}
              
             
          } 
         if ( searchParen(oStack)) {
              input = ';';
               ungetc (input, get_next_byte_argument);
              break;
         }

        /* if (isEmpty(tmp)==0) {
            dStack = addSimpleCommand(dStack, tmp); 
            strcpy(tmp,"\0");}
         else {
            if (dStack == NULL) {strcpy(tmp, "\0");lineNumber ++;break;} 
            else if (oStack != NULL && (peek(dStack)->type) < 4 && (peek(dStack)->type)!=5) {strcpy(tmp, "\0");lineNumber ++;break;}
             }*/
         if ( isEmpty(tmp) != 1)
         {
             dStack = addSimpleCommand(dStack, tmp);
             strcpy(tmp, "\0");
         }else {
             if (dStack ==  NULL) {
                 strcpy(tmp, "\0");
                 lineNumber ++;
                 break;
             } else if ( oStack != NULL && (peek(oStack))->type < 4 && (peek(dStack))->type != 5) {
                
            fprintf(stderr,"line %d: %s\n",lineNumber,wholeline);exit(1);
                // strcpy(tmp, "\0");
                // lineNumber ++;
                // break;
             }
         }

         if (metRedirection > 0)
         {
             dStack = redirection(dStack, metRedirection);
             metRedirection = 0;
         }

         while (oStack != NULL)
         {
                dStack = pop(dStack, &com2);
                dStack = pop(dStack, &com1);
                oStack = pop(oStack, &com);
                
                com->u.command[0] = malloc(sizeof(com->u));
                com->u.command[1] = malloc(sizeof(com->u));
                com->u.command[0] = com1;
                com->u.command[1] = com2;
                dStack = push(dStack, com);
         }
         dStack = pop(dStack, &com);
         *forest = push(*forest, com);
         lineNumber ++;
         break;
      case '#':
         //printf("%s\n", tmp);
         strcpy(tmp,"\0");
         while (input != '\n')
         {
         input = get_next_byte (get_next_byte_argument);
         }
         lineNumber ++;
         int i=0;
         while(wholeline[i]!='\0'){wholeline[i]='\0';i++;}
         break;
      default: 
         comb[0] = c;
         comb[1] = '\0';
         strcat(tmp, comb);
      //   printf("tmp is %s\n",tmp);
      }
      input = get_next_byte (get_next_byte_argument);
  }  

  /*while ( forest != NULL) 
  {
     forest = pop(forest, &tree);
     printf("root: %d\n", tree->type);
     depthTraverse(tree);
     printf("\n");
  }*/

         if (paren!=0)
          {  fprintf(stderr,"%d: Incorrect Syntax %s\n",lineNumber,wholeline);exit(1);
          }
  return forest;
}

command_t
read_command_stream (struct Node ** s)
{
  struct command * tmp;
  if (*s != NULL)
  {
     *s = deQueue(*s, &tmp);
     return tmp;
  }
  else 
  {
     return NULL;
  }
  /* FIXME: Replace this with your implementation too.  */
  //error (1, 0, "command reading not yet implemented");
  return 0;
}
