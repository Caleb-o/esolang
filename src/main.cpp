#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <exception>
#include <memory>
#include "process/bytecode.hpp"
#include "process/util.hpp"
#include "process/parser.hpp"
#include "runtime/vm.hpp"

using namespace Process;

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

	Parser p(Util::get_directory(filename));

	try {
		std::string buffer = Util::read_file(filename);

		if (buffer.size() == 0) {
			throw "File could not be read or is empty";
		}

		std::shared_ptr<Environment> env = p.parse(buffer);
		if (debug) print_env(env);

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