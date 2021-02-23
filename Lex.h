#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>

#include "PreLexer.h"

class Token
{
public:

	std::string type;
	std::string value;
	int start_index;
	int end_index;
	int line_number;

public:

	void print_token()
	{
		if (value == "")
		{
			std::cout << type;
		}
		else
		{
			std::cout << type << ": " << value;
		}
	}
};

class Lexer
{
public:
	std::string content;
	int content_size;
	int current_index;
	char current_char;

	int current_line_number = 1;

	bool debug = false;

	PreLexer pre_lexer;

public:
	Lexer() {}
	Lexer(std::string source_file_path, std::string lexer_rules_filepath)
	{
		init(source_file_path, lexer_rules_filepath);
	}

public:
	std::vector<PreLexerToken> prelexer_tokens;
	std::vector<Container> containers;
	std::vector<Pattern> patterns;
	std::vector<Transform> transforms;
	std::vector<Construct> constructs;
	std::vector<Token> tokens;

public:
	void init(std::string source_file_path, std::string lexer_rules_filepath)
	{
		std::string line;
		std::fstream reader;

		reader.open(source_file_path);

		while (std::getline(reader, line))
		{
			content += line + "\n";
		}

		reader.close();

		if (content.length() > 0)
		{
			content.pop_back(); // removes last incorrect newline

		}

		content.push_back(0);

		content_size = content.size();
		current_index = 0;
		current_char = content[0];

		pre_lexer.init(lexer_rules_filepath);
		pre_lexer.parse();

		load_prelexer_tokens(pre_lexer.tokens);
		load_containers(pre_lexer.containers);
		load_patterns(pre_lexer.patterns);
		load_transforms(pre_lexer.transforms);
		load_constructs(pre_lexer.constructs);
	}

	void load_prelexer_tokens(std::vector<PreLexerToken> _prelexer_tokens)
	{
		prelexer_tokens = _prelexer_tokens;
	}

	void load_containers(std::vector<Container> _containers)
	{
		containers = _containers;
	}

	void load_patterns(std::vector<Pattern> _patterns)
	{
		patterns = _patterns;
	}

	void load_transforms(std::vector<Transform> _transforms)
	{
		transforms = _transforms;
	}

	void load_constructs(std::vector<Construct> _constructs)
	{
		constructs = _constructs;
	}

	void advance()
	{
		current_index++;

		if (current_index >= content_size)
		{
			return;
		}
		else
		{
			current_char = content[current_index];
		}

		if (current_char == '\n')
		{
			current_line_number++;
		}
	}

	void backtrack()
	{
		current_index--;

		if (current_index < 0)
		{
			return;
		}
		else
		{
			current_char = content[current_index];
		}

		if (current_char == '\n')
		{
			current_line_number--;
		}
	}

	bool matches_pattern(Pattern pattern)
	{
		for (auto container : pattern.start)
		{
			if (container.value.find(current_char) != std::string::npos)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	void build_token_from_pattern(Pattern pattern)
	{
		Token token;
		token.type = pattern.token_name;
		token.start_index = current_index;

		token.value += current_char;

		std::string all_end_patterns;

		advance();

		for (auto container : pattern.end)
		{
			all_end_patterns += container.value;
		}

		while (all_end_patterns.find(current_char) != std::string::npos)
		{
			token.value += current_char;
			advance();
		}

		token.end_index = current_index;

		tokens.push_back(token);
	}

	void transform_token(Transform transform, Token &token)
	{
		// IN

		if (transform.keyword == "in")
		{
			if (token.type == transform.from_name &&
				std::find(transform.container.values.begin(), transform.container.values.end(), token.value) 
				!= transform.container.values.end())
			{
				token.type = transform.to_name;
			}
		}

		// CONTAINS

		else if (transform.keyword == "contains")
		{
			if (transform.amount == -1)
			{
				for (char c : transform.container.value)
				{
					if (token.type == transform.from_name && token.value.find(c) != std::string::npos)
					{
						token.type = transform.to_name;
						return;
					}
				}
			}
			else if (transform.amount > -1 && transform.or_more == false)
			{
				int count = 0;

				for (char c : token.value)
				{
					if (token.type == transform.from_name && transform.container.value.find(c) != std::string::npos)
					{
						count++;
					}
				}

				if (count == transform.amount)
				{
					token.type = transform.to_name;
					return;
				}
			}
			else if (transform.amount > -1 && transform.or_more)
			{
				int count = 0;

				for (char c : token.value)
				{
					if (token.type == transform.from_name && transform.container.value.find(c) != std::string::npos)
					{
						count++;
					}
				}

				if (count >= transform.amount)
				{
					token.type = transform.to_name;
					return;
				}
			}
		}

		// EXCLUDES

		else if (transform.keyword == "excludes")
		{
			bool includes_char = false;

			for (char c : transform.container.value)
			{
				if (token.type == transform.from_name && token.value.find(c) != std::string::npos)
				{
					includes_char = true;
				}
			}

			if (!includes_char)
			{
				token.type = transform.to_name;
			}
		}
	}

	void tokenise()
	{
		if (content.length() == 1)
		{
			std::cout << "Warning: Source File is empty.\n";
			return;
		}

		while (current_char != 0)
		{
			for (Pattern pattern : patterns)
			{
				if (matches_pattern(pattern))
				{
					build_token_from_pattern(pattern);
				}
			}

			std::vector<PreLexerToken> matching_prelexer_tokens;

			for (auto prelexer_token : prelexer_tokens)
			{
				if (current_char == prelexer_token.value[0])
				{
					matching_prelexer_tokens.push_back(prelexer_token);
				}
			}

			if (matching_prelexer_tokens.size() > 0)
			{
				std::sort(matching_prelexer_tokens.begin(), matching_prelexer_tokens.end(),
					[](PreLexerToken a, PreLexerToken b) {return a.value.length() > b.value.length(); });

				bool found_matching_token = false;

				for (auto matching_prelexer_token : matching_prelexer_tokens)
				{
					std::string word;

					for (int i = 0; i < matching_prelexer_token.value.length(); i++)
					{
						word += current_char;
						advance();
					}

					if (word == matching_prelexer_token.value)
					{
						Token token;
						token.type = matching_prelexer_token.key;
						token.value = matching_prelexer_token.value;

						token.start_index = current_index - word.length();
						token.end_index = current_index - 1;

						if (current_char == '\n')
						{
							token.line_number = current_line_number - 1;
						}
						else
						{
							token.line_number = current_line_number;
						}

						tokens.push_back(token);

						found_matching_token = true;

						break;
					}
					else
					{
						for (int i = 0; i < matching_prelexer_token.value.length(); i++)
						{
							backtrack();
						}
					}
				}

				if (found_matching_token == false)
				{
					advance();
				}

			}
			else if (matching_prelexer_tokens.size() == 0)
			{
				advance();
			}
		}

		// do transforms

		for (Token& token : tokens)
		{
			for (Transform transform : transforms)
			{
				if (token.type == transform.from_name)
				{
					transform_token(transform, token);
				}
			}
		}

		// do constructions

		for (int token_index = 0; token_index < tokens.size(); token_index++)
		{
			for (Construct construct : constructs)
			{
				if (tokens[token_index].type == construct.start_token_name)
				{
					if (construct.middle_token_name == "")
					{
						Token token;
						token.type = construct.token_name;
						token.line_number = tokens[token_index].line_number;
						token.start_index = tokens[token_index].start_index;

						if (tokens[token_index + 1].type == construct.end_token_name)
						{
							token.end_index = tokens[token_index + 1].end_index;

							token.value = tokens[token_index].value + tokens[token_index + 1].value;

							tokens.erase(tokens.begin() + token_index + 1);

							tokens[token_index] = token;

							break;
						}									
					}
					else if (construct.middle_token_name == "*")
					{
						Token token;
						token.type = construct.token_name;
						token.line_number = tokens[token_index].line_number;
						token.start_index = tokens[token_index].start_index;

						if (construct.includes_first)
						{
							token.value = tokens[token_index].value;
						}

						int start_index = token_index;

						token_index++;

						while (tokens[token_index].type != construct.end_token_name)
						{
							if (tokens[token_index].type == "EOF")
							{
								return;
							}

							token.value += tokens[token_index].value;
							token_index++;
						}

						if (construct.includes_last)
						{
							token.value += tokens[token_index].value;
						}

						token.end_index = tokens[token_index].end_index;

						int end_index = token_index;

						tokens[start_index] = token;

						for (int i = end_index; i > start_index; i--)
						{
							tokens.erase(tokens.begin() + i);
						}

						break;
					}
				}
			}
		}
	}
};
