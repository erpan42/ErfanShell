#ifndef command_hh
#define command_hh

#include "simpleCommand.hh"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <regex.h>
#include <pwd.h>

// Command Data Structure

struct Command {

  int _numOfAvailableSimpleCommands;
  int _numOfSimpleCommands;

  std::vector<SimpleCommand *> _simpleCommands;
  std::string * _outFile;
  std::string * _inFile;
  std::string * _errFile;

  //User Add-on
  bool _background;	//run in background or not
  int _append;	//if ">>", then _append = 1

  int _out_flag;
  int _err_flag;
  int _in_flag;

  void prompt();
  void redirect(int i, std::string * curr);

  Command();
  void insertSimpleCommand( SimpleCommand * simpleCommand );

  void clear();
  void print();
  void execute();
  
  static SimpleCommand *_currentSimpleCommand;
  //i is the index of commandlist
  bool builtIn(int i);	
  //int i  is the index of commandlist.printenv and source
  bool builtIn2(int i);	
};

#endif
