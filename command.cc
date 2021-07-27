/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 * DO NOT PUT THIS PROJECT IN A PUBLIC REPOSITORY LIKE GIT. IF YOU WANT 
 * TO MAKE IT PUBLICALLY AVAILABLE YOU NEED TO REMOVE ANY SKELETON CODE 
 * AND REWRITE YOUR PROJECT SO IT IMPLEMENTS FUNCTIONALITY DIFFERENT THAN
 * WHAT IS SPECIFIED IN THE HANDOUT. WE OFTEN REUSE PART OF THE PROJECTS FROM  
 * SEMESTER TO SEMESTER AND PUTTING YOUR CODE IN A PUBLIC REPOSITORY
 * MAY FACILITATE ACADEMIC DISHONESTY.
 */

#include <cstdio>
#include <cstdlib>

#include <iostream>

#include "command.hh"
#include "shell.hh"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h> 
#include <errno.h>
#include <string.h>
#include <string>
#include <stdlib.h>


Command::Command() {
    // Initialize a new vector of Simple Commands
    _simpleCommands = std::vector<SimpleCommand *>();

    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;

    _background = false;
	_append = 0;
}

void Command::insertSimpleCommand( SimpleCommand * simpleCommand ) {
    // add the simple command to the vector
    _simpleCommands.push_back(simpleCommand);
}

void Command::clear() {
    // deallocate all the simple commands in the command vector
    for (auto simpleCommand : _simpleCommands) {
        delete simpleCommand;
    }

    // remove all references to the simple commands we've deallocated
    // (basically just sets the size to 0)
    _simpleCommands.clear();

    if ( _outFile ) {
        delete _outFile;
    }
    _outFile = NULL;

    if ( _inFile ) {
        delete _inFile;
    }
    _inFile = NULL;

    if ( _errFile ) {
        delete _errFile;
    }
    _errFile = NULL;

    _background = false;
}

void Command::print() {
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    int i = 0;
    // iterate over the simple commands and print them nicely
    for ( auto & simpleCommand : _simpleCommands ) {
        printf("  %-3d ", i++ );
        simpleCommand->print();
    }

    printf( "\n\n" );
    printf( "  Output       Input        Error        Background\n" );
    printf( "  ------------ ------------ ------------ ------------\n" );
    printf( "  %-12s %-12s %-12s %-12s\n",
            _outFile?_outFile->c_str():"default",
            _inFile?_inFile->c_str():"default",
            _errFile?_errFile->c_str():"default",
            _background?"YES":"NO");
    printf( "\n\n" );
}

void Command::redirect(int i, std::string * curr) {
    if (i==0) {
        if ( _inFile ) {
            printf("Ambiguous input redirect.\n");
            exit(1);
        } else {
            _inFile = curr;
        }
    }
    if (i==2) {
        if ( _errFile ) {
            printf("Ambiguous error redirect.\n");
            exit(1);
        } else {
            _errFile = curr;
        }
    }
    if (i==1) {
        if ( _outFile ) {
            printf("Ambiguous output redirect.\n");
            exit(1);
        } else {
            _outFile = curr;
        }
    }
}

bool Command::builtIn(int i) {
	

	if( strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "setenv") == 0 ) {
		if (setenv(_simpleCommands[i]->_arguments[1]->c_str(), _simpleCommands[i]->_arguments[2]->c_str(), 1)) {
			perror("setenv");
		}
		clear();
		Shell::prompt();
		return true;
	}

	if( strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "unsetenv") == 0 ) {
		if (unsetenv(_simpleCommands[i]->_arguments[1]->c_str())) {
			perror("unsetenv");
		}
		clear();
		Shell::prompt();
		return true;
	}

	if (strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "cd") == 0) {

		int error;
		if (_simpleCommands[i]->_arguments.size() == 1) {	//if only "cd", then go HOME
			error = chdir(getenv("HOME"));
		}else {
			error = chdir(_simpleCommands[i]->_arguments[1]->c_str());
		}

		if (error < 0) {	//if error
			perror("cd");
		}

		clear();
		//Shell::prompt();
		return true;
	}
	return false;
}

bool Command::builtIn2(int i) {
	//printenv in child process
	if (strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "printenv") == 0) {
		char ** envvar = environ;

		int i = 0;
		while (envvar[i] != NULL) {
			printf("%s\n", envvar[i]);
			i++;
		}
		return true;
	}

	return false;
}

void Command::execute() {

	
	// Don't do anything if there are no simple commands
    if ( _simpleCommands.size() == 0 ) {
        Shell::prompt();
        return;
    }

    // Print contents of Command data structure
    //print();

    // exit myshell
    std::string* cmd = _simpleCommands[0]->_arguments[0];
    if ( !strcmp(cmd->c_str(),"exit") ) {
            printf( "Good bye!!\n");
        exit(1);
    }

    // set the environment variable
    if ( !strcmp(cmd->c_str(),"setenv") ) {
        if ( _simpleCommands[0]->_arguments.size() != 3 ) {
            fprintf(stderr, "setenv should take two arguments\n");
            return;
        }
        const char * A = _simpleCommands[0]->_arguments[1]->c_str();
        const char * B = _simpleCommands[0]->_arguments[2]->c_str();
        setenv(A, B, 1);
        clear();
        Shell::prompt();
        return;
    }

    // unset the environment varibale
    if ( !strcmp(cmd->c_str(),"unsetenv") ) {
        const char * A = _simpleCommands[0]->_arguments[1]->c_str();
        unsetenv(A);
        clear();
        Shell::prompt();
        return;
    }

    // change directory
    if ( !strcmp(cmd->c_str(),"cd") ) {
        if (_simpleCommands[0]->_arguments.size()==1) {
            chdir(getenv("HOME"));
        } else {
            const char * path = _simpleCommands[0]->_arguments[1]->c_str();
            chdir(path);
        }
        clear();
        Shell::prompt();
        return;
    }

	// Print contents of Command data structure
	//print();

	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec
	int dfltin = dup(0);
	int dfltout = dup(1);
	int dflterr = dup(2);

	int fdin = 0;
	int	fdout = 0;
	int fderr = 0;

	if (_inFile) {
		const char* infile = _inFile->c_str();
		fdin = open(infile, O_RDONLY);
		if (fdin < 0) {
            fprintf(stderr, "/bin/sh: 1: cannot open %s: No such file\n", infile);
            Shell::prompt();
            clear();
            return;
        }

	}
	else {
		fdin = dup(dfltin);
	}

	

	int pid;
	int ret;
	for (size_t i = 0; i < _simpleCommands.size(); i++) {

		


		//redirect input
		dup2(fdin, 0);
		close(fdin);

		//setup output
		if (i == _simpleCommands.size() - 1) {
			//Last simple command
			//Last simple command
            if ( _outFile ) {
                const char* outfile = _outFile->c_str();
                if ( _append ){
                    fdout = open(outfile, O_CREAT|O_WRONLY|O_APPEND, 0664);
                }
                else {
                    fdout = open(outfile, O_CREAT|O_WRONLY|O_TRUNC, 0664);
                }
            } 
            else {
                //Use default output
                fdout = dup(tmpout);
            }

            if ( _errFile ) {
                const char* errfile = _errFile->c_str();
                if ( _append ){
                    fderr = open(errfile, O_CREAT|O_WRONLY|O_APPEND, 0664);
                }
                else {
                    fderr = open(errfile, O_CREAT|O_WRONLY|O_TRUNC, 0664);
                }
            }
            else {
                fderr = dup(tmperr);
            }
            dup2(fderr, 2);
            close(fderr);
            int n = _simpleCommands[i]->_arguments.size();
            char * c = strdup(_simpleCommands[i]->_arguments[n-1]->c_str());
            setenv("_", c, 1);
		}
		else {	//Not last simple command->create pipe
			int fdpipe[2];
			pipe(fdpipe);
			fdout = fdpipe[1];
			fdin = fdpipe[0];
		}	//if/else

		//Redirect the output to fdout, which is fdpipe[1]
		dup2(fdout, 1);
		close(fdout);

		ret = fork();
        if (ret==0) {
            // printenv function call built
            if (!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "printenv")) {
                char **p = environ;
                while (*p!=NULL) {
                    printf("%s\n", *p);
                    p++;
                }
                exit(0);
            }
            // and call exec
            const char * command = _simpleCommands[i]->_arguments[0]->c_str();
            int argnum = _simpleCommands[i]->_arguments.size();
            char ** args = new char*[argnum+1];
            for (int j=0; j<argnum; j++) {
                args[j] = (char *)_simpleCommands[i]->_arguments[j]->c_str();
            }
            args[argnum] = NULL;
            execvp(command, args);
            perror("execvp");
            exit(1);
        } 
        else if (ret < 0) {
            perror("fork");
            return;
        }
	}	//for

	//restore in/out/err to default
	dup2(dfltin, 0);
	dup2(dfltout, 1);
	dup2(dflterr, 2);
	close(dfltin);
	close(dfltout);
	close(dflterr);

	if (_background == false) {
        // Wait for last command
        int status;
        waitpid(ret, &status, 0);
        std::string s = std::to_string(WEXITSTATUS(status));
        setenv("?", s.c_str(), 1);
        char * pError = getenv("ON_ERROR");
        if (pError != NULL && WEXITSTATUS(status) != 0) printf("%s\n", pError);
    } else {
        std::string s = std::to_string(ret);
        setenv("!", s.c_str(), 1);
        Shell::_bgPIDs.push_back(ret);
    }
	// Clear to prepare for next command
	clear();

	// Print new prompt
	Shell::prompt();
}	//execute

SimpleCommand * Command::_currentSimpleCommand;
