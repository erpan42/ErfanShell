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

//part2.1 when keydown ctrl+C, go to nextline in shell. nextline is myshell>>myshell>>

extern "C" void ctrlC(int sig) {
	//fflush(stdin);
	printf("\n");
	Shell::prompt();
}

int main() {
	
	Shell::prompt();
	

	yyparse();
}

Command Shell::_currentCommand;
