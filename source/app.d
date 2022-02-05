import std.stdio;
import std.getopt;
import std.file;
import parsing.environment : Environment;
import parser = parsing.parser;
import interpreter.vm;


void repl() {
	string line;
	write("> ");
    while ((line = stdin.readln()) !is null) {
        parser.Parser p = new parser.Parser(line);
		
		VM vm = new VM(p.parse());
		vm.interpret();
		write("> ");
	}
}

void main(string[] args) {
	string fileName;
	bool quiet;

	getopt(
		args,
		"file",		&fileName,
		"quiet",	&quiet,
	);

	// Incorrect arguments provided
	if (args.length > 1) {
		writeln("Incorrect arguments provided.");
		writeln("Usage: lang [--file|--quiet] [filename]");
	} else {
		if (fileName !is null) {
			string data = cast(string)read(fileName);
			if (data !is null && data.length > 0) {
				writeln("Parsing ", fileName);
				
				parser.Parser p = new parser.Parser(data);
				p.parse();
				// VM vm = new VM(p.parse());
				// vm.interpret();
			}
		} else {
			repl();
		}
	}
}