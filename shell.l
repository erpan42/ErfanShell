
/*
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 * Tokens: \n, \t, |, &, <, >, >&, >>, >>&, 2>
 * %token NOTOKEN NEWLINE PIPE AMPERSAND LESS GREAT GREATAMPERSAND GREATGREAT GREATGREATAMPERSAND TWOGREAT
 */
%x src

%{

#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "y.tab.hh"
#include "shell.hh"
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
  unput(c);
}

extern "C" char * read_line();

int mygetc(FILE * f) {
  static char *p;
  char ch;

  if (!isatty(0)) {
    return getc(f);
  }

  if (p==NULL || *p == 0) {
    char * s = read_line();
    p =s;
  }
  ch = *p;
  p++;

  return ch;
}

#undef getc
#define getc(f) mygetc(f)

%}

%option noyywrap

%%

\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

"|" {
	return PIPE;
}

"&"	{
	return AMPERSAND;
}

"<" {
	return LESS;
}

">" {
  return GREAT;
}

">&" {
	return GREATAMPERSAND;
}

">>" {
	return GREATGREAT;
}



">>&" {
	return GREATGREATAMPERSAND;
}

"2>" {
	return TWOGREAT;
}

source {
  BEGIN(src);
}

<src>[^ \t\n][^ \t\n]* {
  FILE * in = fopen(yytext, "r+");

  if (!in) {
    perror("fopen");
    BEGIN(INITIAL);
  } else {
    fputc('\n', in);
    yypush_buffer_state(yy_create_buffer(in, YY_BUF_SIZE));
    BEGIN(INITIAL);
    Shell::_srcCmd = true;
    yyparse();
    yypop_buffer_state();
    fclose(in);
    Shell::_srcCmd = false;
  }
}

~[^ \n\t]* {
  std::string str = std::string(yytext);
  std::string home = getenv("HOME");
  if (str.size() == 1) {
    yylval.cpp_string = new std::string(home);
  } 
  else {
    if (str[1] == '/') {
      str = home + str.substr(1, -1);
    } 
    else {
      str = std::string("/homes/") + str.substr(1,-1);
    }
    yylval.cpp_string = new std::string(str);
  }
  return WORD;
}

\$\([^\n]*\) {
	/*  Subshell*/
	//get rid of $(
	std::string str = yytext;
	std::string sstr = str.substr(2);
	//get rid of )
	std::size_t	pos = str.find(")");
	std::string temp = sstr.substr(0,pos-2);
	char *subshell = const_cast<char*> (temp.c_str());
	//printf("the subshell command is %s\n", subshell);

	int tempin = dup(0);
	int tempout = dup(1);

	int fdpipein[2];	//subshell
	pipe(fdpipein);

	int fdpipeout[2];	//outshell
	pipe(fdpipeout);

	//subshell command in
	write(fdpipein[1], subshell, strlen(subshell));
	write(fdpipein[1], "\n", 1);
	close(fdpipein[1]);	//close fdpipein[1]

	//subshell command out
	dup2(fdpipein[0], 0);	//fdpipein[0] to input
	close(fdpipein[0]);	//close fdpipein[0]

	//
	dup2(fdpipeout[1], 1);	//
	close(fdpipeout[1]);	//close fdpipeout[1]

	int ret = fork();

	if (ret == 0) {	//in child process
		execvp("/proc/self/exe", NULL);
		_exit(1);
	} else if (ret < 0) {
		perror("fork");
		exit(1);
	}

	dup2(tempin, 0);
	dup2(tempout, 1);
	close(tempin);
	close(tempout);

	char c;
	char * buffer = (char *) malloc (4096);
	int i = 0;
	
	while (read(fdpipeout[0], &c, 1)) {
		if (c == '\n') {	//if \n then put a space
			buffer[i++] = ' ';
		}
		else {
			buffer [i++] = c;
		}
	}
	buffer[i] = '\0';	//printf("the output of subshell is %s\n", buffer);
	//read backward
	int j = i - 1;
	for (j = i - 1; j >= 0; j--) {	
		//printf("c = %c\n", buffer[j]);
		myunputc(buffer[j]);
	}
}



["][^\n\"]*["] {
	/* Quotes */
	
	yylval.cpp_string = new std::string(yytext);
	*yylval.cpp_string = yylval.cpp_string->substr(1, yylval.cpp_string->length()-2);

	/*printf("after dequote, string = %s\n",yylval.cpp_string);*/
	return WORD;
}

[^ \t\n|><&]*\\[^ \t\n]* {
	/* Escaping */

	char * str = strdup(yytext);
	char * newstr = (char*) malloc (100); 	//printf("before deescaping, string = %s\n", str);

	int i = 0;
	while (*str){
		if (*str == '\\'){
			if (*(str+1) == '\\'){
				str = str +2;
				newstr[i++] = '\\';
			}else{
				newstr[i++] = *(++str);
			}
		}else{
			newstr[i++] = *str;
		}
		str++;
	}
	newstr[i] = '\0';

	
	yylval.cpp_string = new std::string(newstr);

	return WORD;
}

$\{[^ \}\n]*\}([^ \n]*|$\{[^ \}\n]*\})* {
  std::string str = std::string(yytext);
  std::string result = "";
  std::size_t found = str.find('$');
  while(found!=-1) {
    std::string component;
    result += str.substr(0, found);
    if (str[found+1] == '{') {
      std::size_t end = str.find('}');
      if (end != -1) {
        component = str.substr(found+2, end-found-2);
        if (getenv(component.c_str())) result += getenv(component.c_str());
        else result += component;      
        str = str.substr(end+1,-1);
      }
    }
    found = str.find('$');
  }
  result += str;
  /*result = result + str.substr(2,str.size()-3);
  if (getenv(str.c_str()))
    yylval.cpp_string = new std::string(getenv(str.c_str()));
  else
    yylval.cpp_string = new std::string(str);*/
  yylval.cpp_string = new std::string(result);
  return WORD;
}

\`[^\n\`]*\`|$\([^\n]*\) {
  std::string str = std::string(yytext);
  if (str.at(0) == '$') {
    str = str.substr(1,str.size()-1);
  }
  str = str.substr(1,str.size()-2);
  str += "\nexit\n";
  //printf("str : %s",str.c_str());

  int pin[2], pout[2], tmpin, tmpout;
  tmpin = dup(0); tmpout = dup(1);
  pipe(pin); pipe(pout);

  dup2(pin[0], 0);
  dup2(pout[1], 1);
  close(pin[0]);
  close(pout[1]);

  write(pin[1], str.c_str(), str.size());
  close(pin[1]);

  int ret = fork();
  if (ret==0) {
    char ** args = new char*[2];
    args[0] = (char*)"/proc/self/exe";
    args[1] = NULL;
    execvp(args[0], args);
    perror("execvp(subshell)");
    exit(1);
  } else if (ret < 0) {
    perror("fork");
    exit(1);
  } else {

    waitpid(ret, NULL, 0);

    dup2(tmpin, 0);
    dup2(tmpout, 1);
    close(tmpin);
    close(tmpout);
  
    int BUFSIZE = 1024;
    char* c = new char[BUFSIZE];
    int rd = read(pout[0], c, BUFSIZE);
    close(pout[0]);

    for (int i = rd - 12; i >= 0; --i ) {
      if (c[i] == '\n') {
        c[i] = '\t';
      }
      myunputc( c[i] );
    }
    delete c;
  }
}


[^ \t\n|><][^ \t\n|><]*  {
  /* Assume that file names have only alpha chars */
  yylval.cpp_string = new std::string(yytext);
  return WORD;
}

.  {
    return NOTOKEN;
}

%%

/*TODO: subshell, escape, quote*/