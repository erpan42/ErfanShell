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

//part2 when keydown ctrl+C, go to nextline in shell. nextline is myshell>>myshell>>

extern "C" void ctrlC(int sig) {
	//fflush(stdin);
	printf("\n");
	Shell::prompt();
}

// Only one msg for the same PID
extern "C" void zombie(int sig) {
	int pid = wait3(0, 0, NULL);

	//printf("[%d] exited.\n", pid);
	for (unsigned i=0; i<Shell::_bgPIDs.size(); i++) {
  		if (pid == Shell::_bgPIDs[i]) {
  			printf("[%d] exited\n", pid);
  			Shell::_bgPIDs.erase(Shell::_bgPIDs.begin()+i);
  			break;
  		}
  	}
}

int main() {
	
	//part2: when keydown ctrl+C, go to nextline in shell.
	struct sigaction sigCtrl;
	sigCtrl.sa_handler = ctrlC;
	sigemptyset(&sigCtrl.sa_mask);
	sigCtrl.sa_flags = SA_RESTART;

	if (sigaction(SIGINT, &sigCtrl, NULL)) {
		perror("sigaction");
		exit(2);
	}

	// Zombie sigaction
	// Only analysize signal if background flag is true
	//if (Shell::_currentCommand._background == true) {
		struct sigaction sigZombie;
		sigZombie.sa_handler = zombie;
		sigemptyset(&sigZombie.sa_mask);
		sigZombie.sa_flags = SA_RESTART;

		if (sigaction(SIGCHLD, &sigZombie, NULL)) {
			perror("sigaction");
			exit(-1);
		}
	//}
	//Create .shellrc
	FILE*fd = fopen(".shellrc", "r");
	if (fd) {
		yyrestart(fd);
		yyparse();
		yyrestart(stdin);
		fclose(fd);
	}
	else {
		Shell::prompt();
	}
	

	yyparse();
}

Command Shell::_currentCommand;
std::vector<int> Shell::_bgPIDs;
bool Shell::_srcCmd;
