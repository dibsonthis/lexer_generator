#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>

class Token
{
public:

	std::string type;
	std::string value;
	int col_number;
	int line_number;

public:

	Token()
		: type("NONE"), value("NONE"), col_number(0), line_number(0) {}
	Token(std::string type)
		: type(type), value("NONE"), col_number(0), line_number(0) {}
	Token(std::string type, std::string value)
		: type(type), value(value), col_number(0), line_number(0) {}
	Token(std::string type, int col_number, int line_number)
		: type(type), value("NONE"), col_number(col_number), line_number(line_number) {}
	Token(std::string type, std::string value, int col_number, int line_number)
		: type(type), value(value), col_number(col_number), line_number(line_number) {}

public:

	void print_token()
	{
		if (value == "NONE")
		{
			std::cout << type;
		}
		else
		{
			std::cout << type << ": " << value;
		}
	}
};

class Error
{
public:
	std::string message;
	Error(std::string message) : message(message) {}
};

class Lexer
{
private:

	std::string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	std::string digits = "1234567890";
	std::string whitespace = " \n\t\n\r";
	std::vector<std::string> datatypes = { "int", "float", "char", "str" };
	std::vector<std::string> KEYWORDS = { "var", "def" };

public:

	std::string rules;
	std::string content;

	int current_index;
	char current_char;
	int content_size;

	std::vector<Token> tokens;
	std::vector<Error> errors;

	int line_number;
	int col_number;

	Lexer(std::string content)
		: content(content), current_index(0), current_char(content[0]), content_size(content.size()),
		line_number(1), col_number(0) {}
	Lexer()
		: content(""), current_index(0), current_char(content[0]), content_size(content.size()),
		line_number(1), col_number(0) {}

	void read_file(std::string file_path)
	{
		std::string line;
		std::fstream reader;

		reader.open(file_path);

		while (std::getline(reader, line))
		{
			content += line + "\n";
		}

		reader.close();

		content.pop_back(); // removes last incorrect newline

		content_size = content.size();
		current_char = content[0];
	}

	void advance()
	{
		current_index++;
		col_number++;
		if (current_index >= content_size)
		{
			current_char = 0;
		}
		else
		{
			current_char = content[current_index];
		}
		if (current_char == '\n')
		{
			line_number++;
			col_number = 0;
		}
	}

	bool is_alpha(char c)
	{
		bool result = alphabet.find(c) != std::string::npos;
		return result;
	}

	bool is_digit(char c)
	{
		bool result = digits.find(c) != std::string::npos;
		return result;
	}

	bool is_whitespace(char c)
	{
		bool result = whitespace.find(c) != std::string::npos;
		return result;
	}

	Token build_word()
	{
		std::string word;
		int col = col_number;
		int line = line_number;
		while (is_alpha(current_char) || is_digit(current_char) || current_char == '_')
		{
			word += current_char;
			advance();
		}
		bool is_datatype = std::find(datatypes.begin(), datatypes.end(), word) != datatypes.end();
		if (is_datatype)
		{
			return Token("TYPE", word, col, line);
		}
		bool is_keyword = std::find(KEYWORDS.begin(), KEYWORDS.end(), word) != KEYWORDS.end();
		if (is_keyword)
		{
			return Token("KEYWORD", word, col, line);
		}
		return Token("ID", word, col, line);
	}

	Token build_string()
	{
		advance();
		std::string _string;
		int col = col_number;
		int line = line_number;
		while (current_char != '"')
		{
			if (current_char == 0)
			{
				return Token("ERROR", "No closing quote for string", col, line);
			}
			_string += current_char;
			advance();
		}
		return Token("STRING", _string, col, line);
	}

	Token build_number()
	{
		std::string number;
		int number_of_dots = 0;
		int col = col_number;
		int line = line_number;
		while (is_digit(current_char) || current_char == '.')
		{
			number += current_char;
			if (current_char == '.')
			{
				number_of_dots++;
			}
			advance();
		}
		if (number_of_dots == 0)
		{
			return Token("INT", number, col, line);
		}
		else if (number_of_dots == 1)
		{
			return Token("FLOAT", number, col, line);
		}
		else
		{
			return Token("ERROR", "Invalid number '" + number + "'", col, line);
		}
	}

	void check_errors()
	{
		for (int i = 0; i < tokens.size(); i++)
		{
			if (tokens[i].type == "ERROR")
			{
				Error error("Syntax Error: " + tokens[i].value + " at line "
					+ std::to_string(tokens[i].line_number) + ", col " + std::to_string(tokens[i].col_number));
				errors.push_back(error);
			}
		}

		if (errors.size() > 0)
		{
			print_errors();
			force_compilation_fail();
		}

	}

	void print_errors()
	{
		for (auto error : errors)
		{
			std::cout << error.message << std::endl;
		}
	}

	void force_compilation_fail()
	{
		std::cout << "Compilation failed at lexing stage.\n";

		std::cin.get();

		exit(1);
	}

	void print_tokens()
	{
		std::cout << "[";
		for (int i = 0; i < tokens.size(); i++)
		{
			tokens[i].print_token();
			if (i != tokens.size() - 1)
			{
				std::cout << ", ";
			}
		}
		std::cout << "]";
	}

	void build_tokens()
	{
		while (current_char != 0)
		{
			if (is_whitespace(current_char))
			{
				if (current_char == '\n')
				{
					Token token = Token("NEWLINE", col_number, line_number);
					tokens.push_back(token);
				}
				advance();
			}
			else if (is_alpha(current_char))
			{
				Token token = build_word();
				tokens.push_back(token);
			}
			else if (is_digit(current_char))
			{
				Token token = build_number();
				tokens.push_back(token);
			}
			else if (current_char == '(')
			{
				Token token = Token("LPAREN", col_number, line_number);
				tokens.push_back(token);
				advance();
			}
			else if (current_char == ')')
			{
				Token token = Token("RPAREN", col_number, line_number);
				tokens.push_back(token);
				advance();
			}
			else if (current_char == '+')
			{
				Token token = Token("PLUS", col_number, line_number);
				tokens.push_back(token);
				advance();
			}
			else if (current_char == '-')
			{
				Token token = Token("MINUS", col_number, line_number);
				tokens.push_back(token);
				advance();
			}
			else if (current_char == '*')
			{
				advance();
				if (current_char == '/')
				{
					Token token = Token("BLOCK_COMMENT_END", col_number, line_number);
					tokens.push_back(token);
					advance();
				}
				else if (current_char == '*')
				{
					Token token = Token("POWER", col_number, line_number);
					tokens.push_back(token);
					advance();
				}
				else
				{
					Token token = Token("MULTIPLY", col_number, line_number);
					tokens.push_back(token);
				}
			}
			else if (current_char == '.')
			{
				Token token = Token("PERIOD", col_number, line_number);
				tokens.push_back(token);
				advance();
			}
			else if (current_char == ',')
			{
				Token token = Token("COMMA", col_number, line_number);
				tokens.push_back(token);
				advance();
			}
			else if (current_char == '!')
			{
				Token token = Token("EXCLAMATION", col_number, line_number);
				tokens.push_back(token);
				advance();
			}
			else if (current_char == ':')
			{
				advance();
				if (current_char == ':')
				{
					Token token = Token("DOUBLE_COLON", col_number, line_number);
					tokens.push_back(token);
					advance();
				}
				else
				{
					Token token = Token("COLON", col_number, line_number);
					tokens.push_back(token);
				}
			}
			else if (current_char == ';')
			{
				Token token = Token("SEMICOLON", col_number, line_number);
				tokens.push_back(token);
				advance();
			}
			else if (current_char == '[')
			{
				Token token = Token("LBRACKET", col_number, line_number);
				tokens.push_back(token);
				advance();
			}
			else if (current_char == ']')
			{
				Token token = Token("RBRACKET", col_number, line_number);
				tokens.push_back(token);
				advance();
			}
			else if (current_char == '{')
			{
				Token token = Token("LBRACE", col_number, line_number);
				tokens.push_back(token);
				advance();
			}
			else if (current_char == '}')
			{
				Token token = Token("RBRACE", col_number, line_number);
				tokens.push_back(token);
				advance();
			}
			else if (current_char == '<')
			{
				advance();
				if (current_char == '=')
				{
					Token token = Token("LESS_OR_EQUAL", col_number, line_number);
					tokens.push_back(token);
					advance();
				}
				else
				{
					Token token = Token("LEFT_ANGLE", col_number, line_number);
					tokens.push_back(token);
				}
			}
			else if (current_char == '>')
			{
				advance();
				if (current_char == '=')
				{
					Token token = Token("GREATER_OR_EQUAL", col_number, line_number);
					tokens.push_back(token);
					advance();
				}
				else
				{
					Token token = Token("RIGHT_ANGLE", col_number, line_number);
					tokens.push_back(token);
				}
			}
			else if (current_char == '=')
			{
				advance();
				if (current_char == '>')
				{
					Token token = Token("ARROW", col_number, line_number);
					tokens.push_back(token);
					advance();
				}
				else
				{
					Token token = Token("EQUAL", col_number, line_number);
					tokens.push_back(token);
				}
			}
			else if (current_char == '/')
			{
				advance();
				if (current_char == '/')
				{
					Token token = Token("LINE_COMMENT", col_number, line_number);
					tokens.push_back(token);
					advance();
				}
				else if (current_char == '*')
				{
					Token token = Token("BLOCK_COMMENT_BEGIN", col_number, line_number);
					tokens.push_back(token);
					advance();
				}
				else
				{
					Token token = Token("DIVIDE", col_number, line_number);
					tokens.push_back(token);
				}
			}
			else if (current_char == '\\')
			{
				advance();
				if (current_char == '\\')
				{
					Token token = Token("DOUBLE_B_SLASH", col_number, line_number);
					tokens.push_back(token);
					advance();
				}
				else
				{
					Token token = Token("B_SLASH", col_number, line_number);
					tokens.push_back(token);
				}
			}
			else if (current_char == '"')
			{
				Token token = build_string();
				tokens.push_back(token);
				advance();
			}
			else
			{
				std::string current_char_string(1, current_char);
				Token token = Token("ERROR", "Invalid character '" + current_char_string + "'", col_number, line_number);
				tokens.push_back(token);
				advance();
			}
		}

		tokens.push_back(Token("EOF"));
	}
};