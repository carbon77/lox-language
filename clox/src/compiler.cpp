#include "scanner.h"
#include "compiler.h"
#include <iomanip>

void compile(std::string source)
{
    Scanner scanner(source);

    int line = -1;
    while (true)
    {
        Token token = scanner.scan_token();
        if (token.line != line)
        {
            std::cout << std::setw(4) << token.line;
            line = token.line;
        }
        else
        {
            std::cout << "   |";
        }

        std::cout << static_cast<int>(token.type) << " " << token.length << " '" << token.start << "'\n";

        if (token.type == TokenType::END_OF_FILE)
            break;
    }
}
