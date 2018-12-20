

#include "myShell.h"

#include <sys/stat.h>
/*
  main function is used to create an instance of myShell
  also will get the raw command from cin. 
  It will parse the command for the first time, get the command
  and then pass the raw command to different parser.
*/
int main(void) {
  myShell shell;
  //while loop is running all the time.
  while (1) {
    // cwd stores the current working directory.
    char * cwd = getcwd(NULL, 0);
    std::cout << "myShell:" << cwd << " $";
    //command string buffer
    char command[1024] = "";
    std::cin.getline(command, 1024);
    std::string command_str = command;
    size_t start = command_str.find_first_not_of(' ');
    if (start == std::string::npos) {
      free(cwd);
      continue;
    }
    command_str = command_str.substr(start);
    if (command_str.size() == 0) {
      free(cwd);
      continue;
    }
    //determine if its printing variable using dollar sign
    std::size_t found_dollar = command_str.find("$");
    //determine if its equal sign operator
    std::size_t found_equal = command_str.find("=");
    //check for EOF
    if (std::cin.eof() || command_str.substr(0, 4) == "exit") {
      free(cwd);
      break;
    }
    //parse the command, retrive the raw command
    shell.parseCMD(command);
    if (found_dollar == 0) {
      shell.dollar(command_str);
    }
    //based on the results, choose the specific parser
    else {
      std::string str = shell.replace(command_str);
      if (str.size() == 0) {
        std::cout << "input is not valid." << std::endl;
        free(cwd);
        break;
      }
      char * command_after_replace = new char[str.size() + 1];
      std::copy(str.begin(), str.end(), command_after_replace);
      command_after_replace[str.size()] = '\0';  // add the terminating 0
      // after general parser, call specialized parser
      if (shell.raw_command.compare("cd") == 0) {
        shell.cdCmd(command_after_replace);
      }
      else if (shell.raw_command.compare("exit") == 0) {
        delete[] command_after_replace;
        free(cwd);
        break;
      }
      else if (shell.raw_command.compare("set") == 0) {
        shell.parseSet(command_after_replace);
      }
      else if (shell.raw_command.compare("export") == 0) {
        shell.parseExport(command_after_replace);
      }
      else if (shell.raw_command.compare("inc") == 0) {
        shell.parseInc(command_after_replace);
      }
      else if (found_equal != std::string::npos) {
        shell.big_Equal(shell.replace(command_str));
      }
      else {
        shell.parseExe(command_after_replace);
        shell.myShell_fork();
      }
      delete[] command_after_replace;
    }
    free(cwd);
  }
  return EXIT_SUCCESS;
}
