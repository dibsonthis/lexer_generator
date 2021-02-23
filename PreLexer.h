#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

class PreLexerToken
{
public:
	std::string key;
	std::string value;
public:
	PreLexerToken(std::string key, std::string value) : key(key), value(value) {}
};

class Container
{
public:
	std::string name;
	std::string value;
	std::vector<std::string> values;
	bool is_list = false;
public:
	Container() {}
	Container(std::string name) : name(name) {}
	Container(std::string name, bool is_list) : name(name), is_list(is_list) {}
};

class Pattern
{
public:
	std::string token_name;
	std::vector<Container> start;
	std::vector<Container> end;

public:
	Pattern() {}

};

class Transform
{
public:
	std::string from_name;
	std::string to_name;
	std::string keyword;
	int amount;
	bool or_more;
	Container container;

public:
	Transform() : amount(0), or_more(false) {}

};

class Construct
{
public:
	std::string token_name;
	std::string start_token_name;
	std::string middle_token_name;
	std::string end_token_name;

	bool includes_first;
	bool includes_last;

public:
	Construct() {}

};

class PreLexer
{
public:
	std::string content;
	int content_size;
	int current_index;
	char current_char;

	bool debug = false;

public:
	std::vector<PreLexerToken> tokens;
	std::vector<Container> containers;
	std::vector<Pattern> patterns;
	std::vector<Transform> transforms;
	std::vector<Construct> constructs;

public:
	void init(std::string file_path)
	{
		std::string line;
		std::fstream reader;

		reader.open(file_path);

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
	}

	void skip_whitespace()
	{
		while (current_char == ' ' || current_char == '\t')
		{
			advance();
		}
	}

	std::string get_word()
	{
		std::string word;
		while (current_char != ' ' && current_char != '\t' && current_char != '\n' && current_char != '\r\n' && current_char != 0)
		{
			word += current_char;
			advance();
		}
		return word;
	}

	void parse_token()
	{
		skip_whitespace();
		std::string token_key = get_word();

		skip_whitespace();
		std::string eq = get_word();
		if (eq == "=")
		{
			skip_whitespace();
			std::string token_value = get_word();

			if (token_value == "\\n")
			{
				token_value = "\n";
			}
			else if (token_value == "\\t")
			{
				token_value = "\t";
			}
			else if (token_value == "\\r")
			{
				token_value = "\r";
			}
			else if (token_value == "\\r\\n")
			{
				token_value = "\r\n";
			}
			else if (token_value == "\\s")
			{
				token_value = " ";
			}

			PreLexerToken token(token_key, token_value);
			tokens.push_back(token);

			if (debug)
				std::cout << "PreLexerToken Found: " << token_key << " = " << token_value << "\n";
		}
		else
		{
			std::cout << "###\nError: Expected '=' after " << token_key << "\n###\n";
		}
	}

	void parse_container()
	{
		skip_whitespace();

		std::string container_name = get_word();

		Container container(container_name);

		skip_whitespace();

		std::string eq = get_word();

		if (eq == "=")
		{
			skip_whitespace();

			if (current_char == '"')
			{
				container.is_list = false;

				std::string value;

				advance();

				while (current_char != '"')
				{
					value += current_char;
					advance();
				}

				container.value = value;

				containers.push_back(container);

				if (debug)
					std::cout << "Flat Container Found: " << container_name << " = '" << value << "'" << "\n";
			}
			else if (current_char == '[')
			{
				container.is_list = true;

				advance();

				skip_whitespace();

				while (current_char != ']')
				{
					std::string value;

					while (current_char != ',')
					{
						if (current_char != ' ' && current_char != '\t' && current_char != ']')
						{
							value += current_char;
						}

						if (current_char == ']')
						{
							break;
						}

						advance();
					}

					if (current_char == ']')
					{
						container.values.push_back(value);
						break;
					}

					advance();

					container.values.push_back(value);
				}

				containers.push_back(container);

				if (debug)
				{
					std::cout << "List Container Found: " << container_name << " = ";
					std::cout << " [ ";
					for (auto value : container.values)
					{
						std::cout << value << " ";
					}
					std::cout << "]\n";
				}

			}
		}
		else
		{
			std::cout << "###\nError: Expected '=' after " << container_name << "\n###\n";
		}
	}
	
	void parse_transform()
	{
		skip_whitespace();

		std::string from_name = get_word();

		skip_whitespace();

		std::string eq = get_word();

		if (eq == "=>")
		{
			skip_whitespace();

			std::string to_name = get_word();

			skip_whitespace();

			get_word(); // if --- test when error handling

			skip_whitespace();

			std::string keyword = get_word();

			std::string amount = "*";

			if (keyword == "contains")
			{
				skip_whitespace();

				amount = get_word();
			}

			skip_whitespace();

			std::string container_name = get_word();

			Transform transform;
			transform.from_name = from_name;
			transform.to_name = to_name;
			transform.keyword = keyword;

			if (amount == "*")
			{
				transform.amount = -1;
			}
			else if (amount.find("+") != std::string::npos) 
			{
				transform.or_more = true;
				amount.pop_back();
				transform.amount = stoi(amount);
			}
			else
			{
				transform.amount = stoi(amount);
			}
			
			for (auto container : containers)
			{
				if (container.name == container_name)
				{
					transform.container = container;
					break;
				}
			}

			transforms.push_back(transform);

			if (debug)
			{
				std::cout << "Transform found: " << transform.from_name << " > " << transform.to_name << "\n";
			}
		}
	}

	void parse_construct()
	{
		skip_whitespace();

		std::string token_name = get_word();

		skip_whitespace();

		std::string eq = get_word();

		if (eq == "=")
		{
			skip_whitespace();

			char c = current_char;
			int i = current_index;
			int number_of_statements = 0;

			while (c != '\n' && c != 0)
			{
				i++;
				c = content[i];

				if (c == '+' || c == '-')
				{
					number_of_statements++;
				}
			}

			if (number_of_statements == 0)
			{
				std::cout << "#construct takes at least two statements. Consider using #transform\n";
			}
			else if (number_of_statements == 1)
			{
				skip_whitespace();

				std::string start_token_name;
				std::string end_token_name;

				start_token_name = get_word();

				skip_whitespace();
				get_word(); // +
				skip_whitespace();

				end_token_name = get_word();

				Construct construct;
				construct.token_name = token_name;
				construct.start_token_name = start_token_name;
				construct.middle_token_name = "";
				construct.end_token_name = end_token_name;

				constructs.push_back(construct);

				if (debug)
				{
					std::cout << "Construct found: "
						<< construct.start_token_name << " + "
						<< construct.end_token_name << "\n";
				}
			}

			else if (number_of_statements == 2)
			{
				skip_whitespace();

				std::string start_token_name;
				std::string middle_token_name;
				std::string end_token_name;

				start_token_name = get_word();

				skip_whitespace();
				std::string first_op = get_word(); // + or -
				skip_whitespace();

				middle_token_name = get_word();

				skip_whitespace();
				std::string second_op = get_word(); // + or -
				skip_whitespace();

				end_token_name = get_word();

				Construct construct;
				construct.token_name = token_name;
				construct.start_token_name = start_token_name;
				construct.middle_token_name = middle_token_name;
				construct.end_token_name = end_token_name;

				if (first_op == "+")
				{
					construct.includes_first = true;
				}
				else if (first_op == "-")
				{
					construct.includes_first = false;
				}

				if (second_op == "+")
				{
					construct.includes_last = true;
				}
				else if (second_op == "-")
				{
					construct.includes_last = false;
				}

				constructs.push_back(construct);

				if (debug)
				{
					std::cout << "Construct found: " 
						<< construct.start_token_name << " + " 
						<< construct.middle_token_name << " + " 
						<< construct.end_token_name << "\n";
				}
			}
			else if (number_of_statements > 2)
			{
				std::cout << "#construct takes at most three two statements.\n";
			}
		}
	}

	void parse_pattern()
	{
		skip_whitespace();

		std::string token_name = get_word();

		skip_whitespace();

		std::string eq = get_word();

		if (eq == "=")
		{
			skip_whitespace();

			char c = current_char;
			int i = current_index;
			int number_of_statements = 0;

			while (c != '\n' && c != 0)
			{
				i++;
				c = content[i];

				if (c == '+')
				{
					number_of_statements++;
				}
			}

			if (number_of_statements == 0)
			{
				std::vector<Container> start;

				std::string container_name;

				while (current_char != '\n')
				{
					if (current_char == ' ' || current_char == '\t')
					{
						advance();
						continue;
					}

					if (current_char == '|')
					{
						for (Container c : containers)
						{
							if (c.name == container_name)
							{
								start.push_back(c);
							}
						}

						container_name = "";
					}

					container_name += current_char;

					advance();
				}

				for (Container c : containers)
				{
					if (c.name == container_name)
					{
						start.push_back(c);
					}
				}

				Pattern pattern;
				pattern.token_name = token_name;
				pattern.start = start;
				pattern.end = start;

				patterns.push_back(pattern);
			}
			else if (number_of_statements == 1)
			{
				std::vector<Container> start;
				std::vector<Container> end;

				std::string container_name;

				while (current_char != '+')
				{
					if (current_char == ' ' || current_char == '\t')
					{
						advance();
						continue;
					}

					if (current_char == '|')
					{
						for (Container c : containers)
						{
							if (c.name == container_name)
							{
								start.push_back(c);
							}

							advance();
						}

						container_name = "";
					}

					container_name += current_char;

					advance();
				}

				for (Container c : containers)
				{
					if (c.name == container_name)
					{
						start.push_back(c);
					}
				}

				container_name = "";

				skip_whitespace();
				advance();
				skip_whitespace();

				// Second Statement

				while (current_char != '\n')
				{
					if (current_char == ' ' || current_char == '\t')
					{
						advance();
						continue;
					}

					if (current_char == '|')
					{
						for (Container c : containers)
						{
							if (c.name == container_name)
							{
								end.push_back(c);
							}
						}

						container_name = "";

						advance();
					}

					container_name += current_char;

					advance();
				}

				for (Container c : containers)
				{
					if (c.name == container_name)
					{
						end.push_back(c);
					}
				}

				Pattern pattern;
				pattern.token_name = token_name;
				pattern.start = start;
				pattern.end = end;

				patterns.push_back(pattern);

				if (debug)
				{
					std::cout << "Pattern found: " << pattern.token_name << "\n";
				}
			}
		}
	}

	void parse()
	{
		if (content.length() == 1)
		{
			std::cout << "Error: PreLexer File is empty.\n";
			return;
		}
		while (current_char != 0)
		{
			if (current_char == '#')
			{
				std::string word = get_word();

				if (word == "#token")
				{
					parse_token();
				}
				else if (word == "#container")
				{
					parse_container();
				}
				else if (word == "#pattern")
				{
					parse_pattern();
				}
				else if (word == "#transform")
				{
					parse_transform();
				}
				else if (word == "#construct")
				{
					parse_construct();
				}
			}

			advance();
		}
	}
};
