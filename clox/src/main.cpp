#include <string>
#include <iostream>
#include <fstream>

#include "chunk.h"
#include "vm.h"
#include "debug.h"

static void repl()
{
  while (true)
  {
    std::string line;
    std::cout << "> ";

    std::getline(std::cin, line);

    interpret(line);
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
    source += line + '\n';
  }

  return source;
}

static void runFile(std::string path)
{
  std::string source = readFile(path);
  InterpretResult result = interpret(source);

  std::cout << source;

  if (result == InterpretResult::INTERPRET_COMPILE_ERROR)
    exit(65);
  if (result == InterpretResult::INTERPRET_RUNTIME_ERROR)
    exit(70);
}

int main(int argc, char *argv[])
{
  VM vm;

  if (argc == 1)
  {
    repl();
  }
  else if (argc == 2)
  {
    runFile(argv[1]);
  }
  else
  {
    std::cerr << "Usage: clox [path]\n";
    exit(64);
  }

  vm.free();

  return 0;
}
