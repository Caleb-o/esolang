#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <cstring>
#include <exception>
#include "process/bytecode.hpp"
#include "process/util.hpp"
#include "process/parser.hpp"
#include "runtime/vm.hpp"

using namespace Process;

std::string read_file(const char *file_name) {
	std::ifstream file(file_name);
	std::string str;

	if (file.is_open()) {
		file.seekg(0, std::ios::end);   
		str.reserve(file.tellg());
		file.seekg(0, std::ios::beg);

		str.assign((std::istreambuf_iterator<char>(file)),
					std::istreambuf_iterator<char>());
	}
	return str;
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
		std::string buffer = read_file(filename);

		if (buffer.size() == 0) {
			throw "File could not be read or is empty";
		}

		Environment *env = p.parse(buffer);
		print_code(env);

		VM vm(env);
		vm.run();
	} catch (const char *msg) {
		std::cerr << "Runtime: " << msg << "\n";
	} catch (std::string& msg) {
		std::cerr << "Runtime: " << msg << "\n";
	} catch (std::exception& e) {
		std::cerr << "Pre-process: " << e.what() << "\n";
	}

	return 0;
}