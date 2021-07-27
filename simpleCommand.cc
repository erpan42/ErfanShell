#include <cstdio>
#include <cstdlib>

#include <iostream>

#include "simpleCommand.hh"

SimpleCommand::SimpleCommand() {
  _arguments = std::vector<std::string *>();
}

SimpleCommand::~SimpleCommand() {
  // iterate over all the arguments and delete them
  for (auto & arg : _arguments) {
    delete arg;
  }
}

char * SimpleCommand::checkExpansion(char * argument) {
	char * arg = strdup(argument);
	char * checkDollar = strchr(arg, '$');
	char * checkBraces = strchr(arg, '{');

	char * replace = (char *) malloc (sizeof(argument) + 50);
	char * temp = replace;

	if (checkDollar && checkBraces) {
		while (*arg != '$') {
			*temp = *arg;
			temp++; arg++;
		}
		*temp = '\0';

		while (checkDollar) {
			if (checkDollar[1] == '{' && checkDollar[2] != '}') {
				char * temporary = checkDollar + 2;
				char * env = (char *) malloc (20);
				char * envtemp = env;

				while (*temporary != '}') {
					*envtemp = *temporary;
					envtemp++; temporary++;
				}
				*envtemp = '\0';

				char * get = getenv(env);

				strcat(replace, get);

				while (*(arg-1) != '}') arg++;

				char * buf = (char *) malloc (20);
				char * tbuf = buf;

				while (*arg != '$' && *arg) {
					*tbuf = *arg;
					tbuf++; arg++;
				}
				*tbuf = '\0';
				strcat(replace, buf);
			}
			checkDollar++;
			checkDollar = strchr(checkDollar, '$');
		}
		argument = strdup(replace);
		return argument;
	}
	return NULL;
}

char * SimpleCommand::tilde(char * argument) {
	if (argument[0] == '~') {
		
		if (strlen(argument) == 1) {
		
			argument = strdup(getenv("HOME"));
			return argument;
		
		} else {

			if (argument[1] == '/') {
				char * dir = strdup(getenv("HOME"));
				argument++;
				argument = strcat(dir, argument);
				return argument;
			}

			char * nArgument = (char *) malloc (strlen(argument) + 20);
			char * uName = (char *) malloc (50);
			char * user = uName;
			char * temp = argument;

			temp++;

			while (*temp != '/' && *temp) *(uName++) = *(temp++);
			*uName = '\0';


			if (*temp) {

				nArgument = strcat(getpwnam(user)->pw_dir, temp);
				argument = strdup(nArgument);
				return argument;

			} else {
				
				argument = strdup(getpwnam(user)->pw_dir);
				return argument;

			}
		}

	}

	return NULL;
}

void SimpleCommand::insertArgument( std::string * argument ) {
  char * exp = tilde((char *)argument->c_str());
	if (exp) 
		argument = new std::string(strdup(exp));
  // simply add the argument to the vector
  _arguments.push_back(argument);
}

// Print out the simple command
void SimpleCommand::print() {
  for (auto & arg : _arguments) {
    std::cout << "\"" << *arg << "\" \t";
  }
  // effectively the same as printf("\n\n");
  std::cout << std::endl;
}
