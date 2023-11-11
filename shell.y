
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token 	NOTOKEN GREAT NEWLINE GREATGREAT LESS AMPERSAND GREATAMPERSAND GREATERAMPERSAND PIPE EXIT SPECIAL CD

%union	{
		char   *string_val;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include "command.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: simple_command
        ;

simple_command:	
	pipe_list iomodifier_opt_list background_opt NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	| EXIT NEWLINE{
		printf("Good bye!!\n");
		exit(0);
	}
	| CD WORD NEWLINE{
		char string[100];
		printf("Directory was : %s\n", getcwd(string,100));
		chdir($2);
		printf("Directory is : %s\n", getcwd(string,100));
	}
	| CD NEWLINE{
		char string[100];
		printf("Directory was : %s\n", getcwd(string,100));
		chdir("/home");
		printf("Directory is : %s\n", getcwd(string,100));
	}
	;

command_and_args:
	command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);

	       Command::_currentSimpleCommand->insertArgument( $1 );\
	}
	;

command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;
	
pipe_list:
	pipe_list PIPE command_and_args{
	}
	| command_and_args
	;
	
iomodifier_opt_list:
	iomodifier_opt_list iomodifier_opt
	|
	;

iomodifier_opt:
	GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
	}
	|
	GREATGREAT WORD {
		printf("   Yacc: insert output (>>) \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
	}
	|
	LESS WORD {
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	|   GREATAMPERSAND WORD {
		
		//Command::_currentCommand._outFile = $2;
		Command::_currentCommand._errFile = $2;
		Command::_currentCommand._greaterFlag=1;
	
	}
	|GREATERAMPERSAND WORD  {
	
		//Command::_currentCommand._outFile = $2;
		Command::_currentCommand._errFile = $2;
	
	
	}
	;

background_opt:
	AMPERSAND {
		printf("   Yacc: background \n");
		Command::_currentCommand._background = 1;
	}
	| /* can be empty */ 
	;

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
