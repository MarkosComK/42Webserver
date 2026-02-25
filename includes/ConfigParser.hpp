#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "Config.hpp"
#include <string>
#include <vector>

void testConfigParsing(const std::string &configPath);

class ConfigParser {
	public:
		ConfigParser(const std::string &filePath);

		const std::vector<ServerConfig> &getServers() const;

	private:
		std::vector<ServerConfig> _servers;

		std::string readFile(const std::string &filePath);
		std::vector<std::string> tokenize(const std::string &content);
		void parse(std::vector<std::string> &tokens);
		ServerConfig parseServerBlock(std::vector<std::string> &tokens, size_t &i);
		Location parseLocationBlock(std::vector<std::string> &tokens, size_t &i);
		size_t parseSize(const std::string &value);
		ListenPair parseHost(const std::string &value);

		class ParseException : public std::exception {
			public:
				ParseException(const std::string &msg);
				virtual ~ParseException() throw();
				virtual const char *what() const throw();
			private:
				std::string _msg;
		};
};

#endif
