#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <exception>
#include "process/bytecode.hpp"
#include "process/util.hpp"
#include "process/parser.hpp"

using namespace Process;

static char *read_file(const char *file_name) {
	std::streampos size;
	char *memblock = nullptr;

	std::ifstream file(file_name, std::ios::in);

	if (file.is_open())
	{
		file.seekg(0, std::ios::end);
		size = file.tellg();
		memblock = new char[size];
		file.seekg(0, std::ios::beg);
		file.read(memblock, size);
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
	
	// Simple argument parsing
	bool debug = false;
	char *filename = nullptr;

	for(int i = 1; i < argc; ++i) {
		switch(Util::hash(argv[i], std::strlen(argv[i]))) {
			case Util::hash("-d", 2): 		debug = true; break;
			default: {
				std::string arg(argv[i]);

				if (arg.find('.') != std::string::npos ||
					arg.find('/') != std::string::npos) {
					filename = argv[i];
				}
				break;
			}
		}
	}

	if (filename == nullptr) {
		usage();
		return 0;
	}

	Parser p;
		
	try {
		char *buffer = read_file(filename);

		if (buffer == nullptr) {
			throw "File could not be read";
		}

		Environment *env = p.parse(buffer);

		print_code(env);

		delete env;
	} catch (const char *msg) {
		std::cerr << "Runtime: " << msg << "\n";
	} catch (std::string& msg) {
		std::cerr << "Runtime: " << msg << "\n";
	} catch (std::exception& e) {
		std::cerr << "Pre-process: " << e.what() << "\n";
	}

	std::cout << "Done.\n";


	return 0;
}