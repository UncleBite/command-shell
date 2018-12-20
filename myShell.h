#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
//declare the external global variable
extern char ** environ;

class myShell
{
 public:
  void myShell_fork();
  void parseCMD(char * cmd);
  std::string getActualPath(char * raw_path);
  void getAllPath();
  bool findCommand(char * path);
  std::string charCombine(char * a, const char * b);
  void cdCmd(char * cmd);
  void parseExe(char * cmd);
  void setVar();
  void parseSet(char * cmd);
  void parseExport(char * cmd);
  void parseInc(char * cmd);
  bool isNum(std::string input);
  void dollar(std::string cmd_str);
  void equal(std::string cmd_str);
  void big_Equal(std::string cmd_str);
  bool valid_name_checker(std::string input);
  std::string find_in_map(std::string var_in);
  std::string to_string(int value);
  std::string replace(std::string input);
  std::vector<std::string> answer;  //answer stores the argument string as vector
  std::string
      raw_command;  // this is the direct command generated from the parser. EX: cd, ls,exit;
 private:
  bool is_dir;
  bool command_valid;
  int argc;            // size of argument list
  char * raw_path;     // raw path is either /bin/ls or ls
  char * actual_path;  // actual_path is the full path of the command
  std::set<std::string>
      ECE551PATH;  //ECE551PATH is a vector that store all the su path in PATH, like /bin/bash, /usr/bin
  std::map<std::string, std::string>
      variable_map;  // map stores the local variable as name and value.
};

//if the command contains the $ sign but not at the first place(meaning this is another command containing variable),
// it will search in the map, and teplace the variable in command with its value。
std::string myShell::replace(std::string input) {
  std::size_t found = input.find("$");
  //error handler
  if (found == std::string::npos) {
    return input;
  }
  std::string first_valid = input.substr(found);
  std::string before_dollar = input.substr(0, found);
  std::vector<std::string> replace_list;
  char * buf = strdup(first_valid.c_str());
  char * tmp;
  tmp = strtok(buf, "$");
  while (tmp != NULL) {
    replace_list.push_back(tmp);
    tmp = strtok(NULL, "$");
  }
  std::string buffer = "";
  std::map<std::string, std::string>::iterator it;
  for (size_t i = 0; i < replace_list.size(); i++) {
    buffer += replace_list[i].replace(
        0, find_in_map(replace_list[i]).length(), variable_map[find_in_map(replace_list[i])]);
  }
  free(buf);
  return before_dollar + buffer;
}

//return true if input string only contains alphabet, number or underscore
bool myShell::valid_name_checker(std::string input) {
  int index = 0;
  while (input[index]) {
    if (isalpha(input[index]) || isdigit(input[index]) || input[index] == '_')
      index++;
    else
      return false;
  }
  return true;
}

//this function is designed to enable multiple equal assignemnt at the same time
void myShell::big_Equal(std::string cmd_str) {
  std::vector<std::string> equal_list;
  char * buf = strdup(cmd_str.c_str());
  char * tmp;
  tmp = strtok(buf, " ");
  while (tmp != NULL) {
    equal_list.push_back(tmp);
    tmp = strtok(NULL, " ");
  }
  for (size_t i = 0; i < equal_list.size(); i++) {
    equal(equal_list[i]);
  }
  free(buf);
  return;
}

// if command contains equal signs, add the variable and value to the map
void myShell::equal(std::string cmd_str) {
  char * buf = strdup(cmd_str.c_str());
  char * var_name = strtok(buf, "=");
  //error handler
  if (var_name == NULL) {
    std::cout << "equal operator: invalid syntax." << std::endl;
    free(buf);
    return;
  }
  std::string var_name_str = var_name;
  //error handler
  if (!valid_name_checker(var_name_str)) {
    std::cout << "sorry, var name has invalid character." << std::endl;
    return;
  }
  char * value = strtok(NULL, "=");
  //error handler
  if (value == NULL) {
    std::cout << "equal operator: invalid syntax." << std::endl;
    free(buf);
    return;
  }
  std::map<std::string, std::string>::iterator it = variable_map.find(var_name);
  if (it == variable_map.end()) {
    variable_map.insert(std::make_pair(var_name, value));
  }
  else
    it->second = value;
  free(buf);
  return;
}

//return the longest possible string foundi in the map
// if not found, will erase one char from back and search again
std::string myShell::find_in_map(std::string var_in) {
  std::string tmp = var_in;
  std::map<std::string, std::string>::iterator it;
  while (tmp.length() > 0) {
    it = variable_map.find(tmp);
    if (it == variable_map.end())
      tmp.erase(tmp.end() - 1);  // remove from last;
    else
      return tmp;
  }
  return "";
}

//if dollar sign found in the string, will try to retrive it from the map
void myShell::dollar(std::string cmd_str) {
  std::vector<std::string> local_var;
  char * buf = strdup(cmd_str.c_str());
  char * tmp;
  tmp = strtok(buf, "$");
  while (tmp != NULL) {
    local_var.push_back(tmp);
    tmp = strtok(NULL, "$");
  }
  // for each variable, find its value from the map
  for (size_t i = 0; i < local_var.size(); i++) {
    local_var[i].replace(
        0, find_in_map(local_var[i]).length(), variable_map[find_in_map(local_var[i])]);
    std::cout << local_var[i];
  }
  std::cout << "\n";
  free(buf);
  return;
}

//this function convert integer to string
std::string myShell::to_string(int value) {
  std::ostringstream os;
  os << value;
  return os.str();
}

// this function return true if input string only consists
bool myShell::isNum(std::string input) {
  std::string::const_iterator it = input.begin();
  while (it != input.end() && std::isdigit(*it)) {
    ++it;
  }
  if (it == input.end()) {
    return true;
  }
  return false;
}

// if command is inc,  will first find the value from the map,
// then treat it as integer and increment by 1.
// if not found, will replce it by 1.
void myShell::parseInc(char * cmd) {
  std::string cmd_str(cmd);
  const char ch = ' ';
  char * first = strchr(cmd, ch);
  if (first == NULL) {
    std::cout << "Inc command: invalid syntax." << std::endl;
    return;
  }
  std::string key = cmd_str.substr(0, first - cmd);
  std::string tmp(first);
  std::string value = tmp.substr(1, cmd_str.length() - tmp.length());
  std::cout << key << std::endl;
  //error handler
  if (!valid_name_checker(value)) {
    std::cout << "sorry, var name has invalid character." << std::endl;
    return;
  }
  std::map<std::string, std::string>::iterator it = variable_map.begin();
  it = variable_map.find(value);
  //if not found
  if (it == variable_map.end()) {
    variable_map.insert(std::make_pair(value, "1"));
  }
  else {  // found ti
    if (isNum(it->second)) {
      if (it->second == "4294967295")  // if overflow set it to 0 then increase
        it->second = "1";
      else
        it->second =
            to_string((atoi((it->second).c_str()) + 1));  // if type is digit, then increase
    }
    else {
      it->second = "1";
    }
  }
  return;
}

//if command is export, will will first find the value from the map,
// if found it, will add its var name and its value to the environment
// if not found, will print out the error message.
void myShell::parseExport(char * cmd) {
  std::string cmd_str(cmd);
  const char ch = ' ';
  char * first = strchr(cmd, ch);
  if (first == NULL) {
    std::cout << "export command: invalid syntax." << std::endl;
    return;
  }
  std::string key = cmd_str.substr(0, first - cmd);
  std::string tmp(first);
  std::string value = tmp.substr(1, cmd_str.length() - tmp.length());
  std::map<std::string, std::string>::iterator it;
  it = variable_map.find(value);
  //error handler
  if (it == variable_map.end()) {
    std::cout << "Var name doesn't exist." << std::endl;
    return;
  }

  if (setenv(it->first.c_str(), it->second.c_str(), 1) == -1) {
    //error handler
    char * mesg = strerror(errno);
    printf("Mesg:%s\n", mesg);
  }
  return;
}

// if command is set, will first find the value from the map,
// if found it, will replace the old value. If not found,
// will add the new value pair to the map.
void myShell::parseSet(char * cmd) {
  std::string cmd_str(cmd);
  const char ch = ' ';
  char * first = strchr(cmd, ch);
  //error handler
  if (first == NULL) {
    std::cout << "set command: invalid syntax." << std::endl;
    return;
  }

  char * second = strchr(first + 1, ch);
  //error handler
  if (second == NULL) {
    std::cout << "set command: invalid syntax." << std::endl;
    return;
  }
  std::string tmp(first);
  std::string temp(second);
  std::string var_name = tmp.substr(1, second - first - 1);
  //error handler
  if (!valid_name_checker(var_name)) {
    std::cout << "sorry, var name has invalid character." << std::endl;
    return;
  }
  int i = second - cmd;
  std::string var_value = temp.substr(1, cmd_str.length() - i);
  std::map<std::string, std::string>::iterator it = variable_map.find(var_name);
  if (it == variable_map.end()) {
    variable_map.insert(
        std::make_pair(tmp.substr(1, second - first - 1), temp.substr(1, cmd_str.length() - i)));
  }
  else
    it->second = var_value;
  return;
}

// parseEXE take char pointer as input,it will first tokenize each argument.
// also, it will get the full path of a command
// and  take care of the escape character.
void myShell::parseExe(char * cmd) {
  std::string cmd_str(cmd);
  raw_path = cmd;
  char * raw_path_tmp = cmd;
  raw_path_tmp = strtok(raw_path_tmp, " ");
  getAllPath();
  std::string full_command = getActualPath(raw_path_tmp);
  std::string ch = " ";
  size_t first = cmd_str.find(ch);
  std::string value = cmd_str.substr(first + 1, cmd_str.length() - first);
  std::stringstream ss;
  answer.clear();
  answer.push_back(full_command);
  for (size_t i = 0; i < value.size(); i++) {
    if (value[i] == '\\' && value[i + 1] == ' ') {
      ss << ' ';
      i++;
    }
    else if (value[i] == '\\' && value[i + 1] == '\\') {
      ss << '\\';
      i++;
    }
    else if (value[i] == ' ') {
      if (ss.str().length() != 0) {
        answer.push_back(ss.str());
        ss.str("");
      }
    }
    else {
      ss << value[i];
    }
  }
  if (ss.str().size() > 0 && first != std::string::npos)
    answer.push_back(ss.str());
}

//if the command is cd, it will parse and then execute cd command;
void myShell::cdCmd(char * cmd) {
  std::vector<char *> cd_args;
  char ** cd_argv = NULL;
  char * cmd_bp = cmd;
  char * input = strtok(cmd_bp, " ");
  while (input != NULL) {
    cd_args.push_back(input);
    input = strtok(NULL, " ");
  }
  cd_argv = new char *[cd_args.size() + 1];
  for (size_t k = 0; k < cd_args.size(); k++)
    cd_argv[k] = cd_args[k];
  int cd_argc = cd_args.size();
  cd_argv[cd_args.size()] = NULL;
  if (cd_argc > 2) {
    std::cout << "invalid argument of cd" << std::endl;
    return;
  }
  else {
    if (cd_argv[1] != NULL) {
      int res = chdir(cd_argv[1]);
      delete[] cd_argv;
      //error handler
      if (res == -1) {
        perror("error in cd");
      }
      else if (res == 0) {
        return;
      }
    }
    else {
      delete[] cd_argv;
      int res = chdir("/");
      //error handler
      if (res == -1) {
        perror("error in cd");
      }
      else if (res == 0) {
        return;
      }
    }
  }
}

//combine a with b; result = b/a, with a tailing null terminator
std::string myShell::charCombine(char * a, const char * b) {
  std::string a_bp(a);
  std::string b_bp(b);
  std::string ans = b_bp + "/" + a_bp;
  return ans;
}

//get all the directory from getenv("PATH"), and split, store them into vector<string> ECE551PATH;
void myShell::getAllPath() {
  const std::string folder_list = getenv("PATH");
  char delimiter = ':';
  size_t previous = 0;
  size_t index = folder_list.find(delimiter);
  while (index != std::string::npos) {
    ECE551PATH.insert(folder_list.substr(previous, index - previous));
    previous = index + 1;
    index = folder_list.find(delimiter, previous);
  }
  ECE551PATH.insert(folder_list.substr(previous));
}

//given the relative path, will return the absolute path
std::string myShell::getActualPath(char * raw_path) {
  command_valid = false;
  is_dir = false;
  std::string raw_path_str = raw_path;
  struct stat sb;
  if (stat(raw_path, &sb) == 0 && S_ISDIR(sb.st_mode)) {
    std::cout << raw_path << " is directory." << std::endl;
    is_dir = true;
    return raw_path;
  }
  if (strchr(raw_path, '/') != NULL)  // raw path contains the "/"
  {
    if (stat(raw_path, &sb) == 0 && sb.st_mode & S_IXUSR) {
      command_valid = true;
      return raw_path;
    }
  }
  else {
    for (std::set<std::string>::iterator it = ECE551PATH.begin(); it != ECE551PATH.end(); ++it) {
      struct stat sb;
      std::string tmp = charCombine(raw_path, (*it).c_str());
      if (stat(tmp.c_str(), &sb) == 0 && sb.st_mode & S_IXUSR) {
        command_valid = true;
        return tmp;
      }
    }
    //error handler
    printf("Command %s not found\n", raw_path);
  }
  return raw_path;
}

// given the raw input, will store parse the command into raw_command
// seperated by space
void myShell::parseCMD(char * cmd) {
  char cmd_temp[1024];
  //initialize ECE551PATH with PATH;
  variable_map.insert(std::make_pair("ECE551PATH", getenv("PATH")));
  strcpy(cmd_temp, cmd);
  raw_command = strtok(cmd_temp, " ");
}

//will create the child process and execute the command.
void myShell::myShell_fork() {
  if (command_valid == true && is_dir == false) {
    pid_t pid = fork();
    if (pid == 0) {
      // child process create the child process
      char ** env = environ;
      char ** exe_argv = new char *[answer.size() + 1];
      for (size_t i = 0; i < answer.size(); i++) {
        exe_argv[i] = (char *)(answer[i].c_str());
      }
      exe_argv[answer.size()] = NULL;
      execve(exe_argv[0], exe_argv, env);
      delete[] exe_argv;
      //error handler
      perror("child process error");
    }
    else if (pid > 0) {
      //parent process wait for child process and check the exit status.
      int status;
      pid_t e_pid = waitpid(pid, &status, 0);

      if (e_pid != -1 && WIFSIGNALED(status)) {
        int exit_signal = WTERMSIG(status);
        printf("Program was killed by signal %d\n", exit_signal);
      }
      if (e_pid != -1 && WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        printf("Program exited with status %d\n", exit_code);
      }
    }
    else {
      //error occur
      perror("parent process error");
    }
  }
  //error handler
  else if (command_valid == false) {
    std::cout << "Invalid command." << std::endl;
    return;
  }
}
