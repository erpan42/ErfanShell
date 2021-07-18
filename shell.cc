#include <cstdio>

#include "shell.hh"

void yyrestart(FILE * file);
int yyparse(void);

void Shell::prompt() {

	//if input is from terminal
	if (isatty(0)) {
		printf("myshell>");
		fflush(stdout);
	}
	fflush(stdout);
}



int main() {
	
	Shell::prompt();
	

	yyparse();
}

Command Shell::_currentCommand;
