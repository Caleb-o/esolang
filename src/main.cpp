#include <iostream>
#include <fstream>
#include <string>
#include <exception>
#include "process/parser.hpp"

using namespace Process;

static const char *read_file(const char *file_name) {
	std::streampos size;
	char * memblock;

	std::ifstream file(file_name, std::ios::in);
	if (file.is_open())
	{
		size = file.tellg();
		memblock = new char [size];
		file.seekg (0, std::ios::beg);
		file.read (memblock, size);
		file.close();
	}
	return memblock;
}

static void usage() {
	std::cout << "Usage: eso filename\n";
}

int main(int argc, char **argv) {
	if (argc <= 1) {
		usage();
		return 0;
	}

	if (argc == 2) {
		Parser p;
		
		try {
			Environment *env = p.parse(read_file(argv[1]));
			delete env;
		} catch (std::string& msg) {
			std::cerr << "Runtime: " << msg << "\n";
		} catch (std::exception& e) {
			std::cerr << "Pre-process: " << e.what() << "\n";
		}

		std::cout << "Done.\n";
	} else {
		usage();
	}


	return 0;
}