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
#include <cstring>
#include <iostream>
#include <fstream>
#include "command.hh"
#include "shell.hh"
#include <vector>

#include "command.hh"
#include "shell.hh"
using namespace std;
extern char **environ;


Command::Command() {
    // Initialize a new vector of Simple Commands
    _simpleCommands = std::vector<SimpleCommand *>();

    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;

    _background = false;
	_append = 0;
	_out_flag = 0;
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
	_append = 0;
}

void Command::print() {
    // printf("\n\n");
    // printf("              COMMAND TABLE                \n");
    // printf("\n");
    // printf("  #   Simple Commands\n");
    // printf("  --- ----------------------------------------------------------\n");

    // int i = 0;
    // // iterate over the simple commands and print them nicely
    // for ( auto & simpleCommand : _simpleCommands ) {
    //     printf("  %-3d ", i++ );
    //     simpleCommand->print();
    // }

    // printf( "\n\n" );
    // printf( "  Output       Input        Error        Background\n" );
    // printf( "  ------------ ------------ ------------ ------------\n" );
    // printf( "  %-12s %-12s %-12s %-12s\n",
    //         _outFile?_outFile->c_str():"default",
    //         _inFile?_inFile->c_str():"default",
    //         _errFile?_errFile->c_str():"default",
    //         _background?"YES":"NO");
    // printf( "\n\n" );
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
	
	//setenv
	if( strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "setenv") == 0 ) {
		if (setenv(_simpleCommands[i]->_arguments[1]->c_str(), _simpleCommands[i]->_arguments[2]->c_str(), 1)) {
			perror("setenv");
		}
		clear();
		Shell::prompt();
		return true;
	}
	//unsetenv
	if( strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "unsetenv") == 0 ) {
		if (unsetenv(_simpleCommands[i]->_arguments[1]->c_str())) {
			perror("unsetenv");
		}
		clear();
		Shell::prompt();
		return true;
	}

	//cd
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

	//printf("_simpleCommands.size is %zu", _simpleCommands.size());
	// Don't do anything if there are no simple commands
	if (_simpleCommands.size() == 0) {
		if(isatty(0))
			Shell::prompt();
		return;
	}


	if( strcmp(_simpleCommands[0]->_arguments[0]->c_str(),"exit") == 0){
		//printf("Exiting\n");
		exit(0);
	}

	if( strcmp(_simpleCommands[0]->_arguments[0]->c_str(),"setenv") == 0){
		 int env = setenv(_simpleCommands[0]->_arguments[1]->c_str(),_simpleCommands[0]->_arguments[2]->c_str(),1);
		 if(env != 0)
			 perror("setenv");
		clear();
		Shell::prompt();
		return;
	}

	if( strcmp(_simpleCommands[0]->_arguments[0]->c_str(),"unsetenv") == 0){
		int env = unsetenv(_simpleCommands[0]->_arguments[1]->c_str());
		if(env != 0)
			perror("unsetenv");
		clear();
		Shell::prompt();
		return;
	}

	if( strcmp(_simpleCommands[0]->_arguments[0]->c_str(),"cd") == 0){
		if(_simpleCommands[0]->_arguments[1])
		{
			//printf("dir=%s\n\n", _simpleCommands[0]->_arguments[1]->c_str());
			int ret;
			if(strncmp(_simpleCommands[0]->_arguments[1]->c_str(),"${HOME}",7)==0)
			{
				char* dir = getenv("HOME");
				ret = chdir(dir);
				//str = str.substr(1,str.length()-2);
				//chdir((_simpleCommands[0]->_arguments[1])->substr(9).c_str());
			}
			else
				ret = chdir(_simpleCommands[0]->_arguments[1]->c_str());
			if(ret != 0)
				//perror("chdir");
				fprintf(stderr,"cd: can't cd to %s\n",_simpleCommands[0]->_arguments[1]->c_str());
		}
		else
		{
			char* dir = getenv("HOME");
			chdir(dir);
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
	int tmpin = dup(0);
	int tmpout = dup(1);
	int tmperr = dup(2);

	int fdin;
	if(_inFile)
		fdin = open(_inFile->c_str(), O_RDONLY, 0666);
	else
		fdin = dup(tmpin);

	int ret;
	int fdout;
	int fderr;
	int proc;
	int status;

	for (unsigned int i=0;i<_simpleCommands.size(); i++) {

		


		//redirect input
		dup2(fdin, 0);
		close(fdin);

		//setup output
		if(i == _simpleCommands.size()-1)
		{
			if (_append)
				if (_outFile)
					fdout = open(_outFile->c_str(), O_WRONLY | O_APPEND, 0666);
				else
					fdout = dup(tmpout);
			else
				if (_outFile)
					fdout = open(_outFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
				else
					fdout = dup(tmpout);
			if(_errFile)
				if(!_append)
					fderr = open(_errFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
				else
					fderr = open(_errFile->c_str(), O_WRONLY | O_APPEND, 0666);
			else
				fderr = dup(tmperr);
		}else
		{
			int fdpipe[2];
			pipe(fdpipe);
			fdout = fdpipe[1];
			fdin = fdpipe[0];
			fderr = dup(tmperr);
		}


		//Redirect the output to fdout, which is fdpipe[1]
		dup2(fdout, 1);
		close(fdout);
		dup2(fderr, 2);
		close(fderr);

		//setenv, unsetenv, cd
		if (builtIn(i)) {
			return;
		}

		//create child process
		ret = fork();
		
		if (ret == -1) {
			perror("fork\n");
			exit(2);
		}

		if (ret == 0) 
		{
			std::vector<char *> vec;
			std::vector<char *> vec_new;
			char c[300];
			char * vec2;
			for(unsigned int a=0;a<_simpleCommands[i]->_arguments.size();a++)
			{
				if(strcmp(_simpleCommands[i]->_arguments[a]->c_str(), "${$}")==0)
				{
					sprintf(c, "%d", proc);
					vec2 = c;
				}
				else if(strcmp(_simpleCommands[i]->_arguments[a]->c_str(), "${?}")==0)
				{
				}
				else if(strcmp(_simpleCommands[i]->_arguments[a]->c_str(), "${!}")==0)
				{
					strcpy(c, getenv("$!"));
					vec2 = c;
				}
				if(strcmp(_simpleCommands[i]->_arguments[a]->c_str(), "${_}")==0)
				{
					vec2 = const_cast<char *>(_simpleCommands[i-1]->_arguments[_simpleCommands[i-1]->_arguments.size()-1]->c_str());
				}
				else if(strcmp(_simpleCommands[i]->_arguments[a]->c_str(), "${SHELL}")==0)
				{
					//char c[300];
					char * path = realpath("../shell", c);
					if(path)
						vec2 = c;
					else
						perror("realpath");
				}
				else
					vec2 = const_cast<char *>(_simpleCommands[i]->_arguments[a]->c_str());
				vec.push_back(vec2);
			}
			vec.push_back(NULL);
			if(strcmp(_simpleCommands[i]->_arguments[0]->c_str(),"printenv") == 0)
			{
				int j=0;
				while(environ[j])
				{
					printf("%s\n", environ[j]);
					j++;
				}
				exit(0);
			}
				execvp(_simpleCommands[i]->_arguments[0]->c_str(), vec.data());
			perror("execvp");
			_exit(1);

		}	//if pid ==  0 
	}	//for

	//restore in/out/err to default
	dup2(tmpin, 0);
	dup2(tmpout, 1);
	dup2(tmperr, 2);
	close(tmpin);
	close(tmpout);
	close(tmperr);

	if(!_background)
	{
		//wait for last process
		int i;
		proc = waitpid(ret, &i, 0);
		if(WIFEXITED(i))
			status = WEXITSTATUS(i);
		//proc = waitpid(ret, NULL, 0);
		//if(WIFEXITED(status))
	}
	// Clear to prepare for next command
	clear();

	// Print new prompt
	if(isatty(0))
	Shell::prompt();
}	//execute

SimpleCommand * Command::_currentSimpleCommand;
