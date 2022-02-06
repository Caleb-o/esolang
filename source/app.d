import std.stdio;
import std.getopt;
import std.file;
import parsing.parser;
import interpreter.vm;


void repl() {
	string line;
	write("> ");
    while ((line = stdin.readln()) !is null) {
        Parser p = new Parser(line);
		
		VM vm = new VM(p.parse());
		vm.interpret();
		write("> ");
	}
}

void main(string[] args) {
	string fileName;
	bool quiet;

	try {
		getopt(
			args,
			"file",		&fileName,
			"quiet",	&quiet,
		);

		if (fileName !is null) {
			string data = cast(string)read(fileName);
			if (data !is null && data.length > 0) {
				writeln("Parsing ", fileName);
				
				Parser p = new Parser(data);
				auto env = p.parse();
				// printCode(env);

				VM vm = new VM(env);
				vm.interpret();
			}
		} else {
			repl();
		}
	} catch (FileException e) {
		writefln("File Error: %s", e.msg);
	} catch(Exception) {
		writeln("Incorrect arguments provided.");
		writeln("Usage: esolang [--file|--quiet] [filename]");
	}
}