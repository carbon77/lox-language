#include <string>
#include <iostream>
#include <fstream>

#include "chunk.h"
#include "vm.h"
#include "debug.h"

VM vm;

static void repl()
{
  while (true)
  {
    std::string line;
    std::cout << "> " << std::flush;

    if (!std::getline(std::cin, line))
      break;
    if (line == "exit")
      break;

    line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
    vm.interpret(line);
  }
}

static std::string readFile(std::string path)
{
  std::string source;
  std::ifstream inputFile(path);

  if (!inputFile.is_open())
  {
    std::cerr << "Can't open file \"" << path << "\"\n";
    exit(74);
  }

  std::string line;
  while (std::getline(inputFile, line))
  {
    source += line;
    if (!inputFile.eof())
      source += '\n';
  }

  return source;
}

static void runFile(std::string path)
{
  std::string source = readFile(path);
  InterpretResult result = vm.interpret(source);

  if (result == InterpretResult::INTERPRET_COMPILE_ERROR)
    exit(65);
  if (result == InterpretResult::INTERPRET_RUNTIME_ERROR)
    exit(70);
}

int main(int argc, char *argv[])
{
  try
  {
    if (argc == 1)
      repl();
    else if (argc == 2)
      runFile(argv[1]);
    else
      throw std::runtime_error("Usage: clox [path]");

    vm.free();
  }
  catch (std::exception &exp)
  {
    std::cerr << exp.what() << std::endl;
    return 1;
  }

  return 0;
}
