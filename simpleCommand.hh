#ifndef simplecommand_hh
#define simplecommand_hh

#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include "y.tab.hh"

//C
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#define MAXFILENAME 1024


struct SimpleCommand {

  // Simple command is simply a vector of strings
  std::vector<std::string *> _arguments;

  SimpleCommand();
  ~SimpleCommand();
  void insertArgument( std::string * argument );
  char * checkExpansion(char * argument);
  char * tilde(char * argument);
  void print();
};

#endif
