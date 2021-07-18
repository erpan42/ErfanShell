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

//when ctrl+C on ongoing process, nextline is myshell>>myshell>>
extern "C" void ctrlC(int sig) {
	//fflush(stdin);
	printf("\n");
	Shell::prompt();
}

//TODO: Only one msg for the same PID
extern "C" void zombie(int sig) {
	int pid = wait3(0, 0, NULL);

	printf("[%d] exited.\n", pid);
	while (waitpid(-1, NULL, WNOHANG) > 0) {};
}

int main() {
	
		Shell::prompt();
	

	yyparse();
}

Command Shell::_currentCommand;
