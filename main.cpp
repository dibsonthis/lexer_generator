#include "Lex.h"

int main()
{
	Lexer lexer;
	lexer.pre_lexer.debug = true;
	lexer.init("test.txt", "token_rules.txt");
	lexer.tokenise();

	std::cin.get();
}