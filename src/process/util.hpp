#pragma once
#include "token.hpp"

#pragma warning(disable : 4996)
#include <cstdint>
#include <memory>
#include <string>
#include <fstream>
#include <stdexcept>

using namespace Process;

namespace Util {
	static std::string read_file(const char *file_name) {
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

	// https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
	template<typename ... Args>
	static std::string string_format(const std::string& format, Args ... args)
	{
		int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
		if (size_s <= 0){ throw std::runtime_error("Error during formatting."); }

		auto size = static_cast<size_t>(size_s);
		auto buf = std::make_unique<char[]>(size);
		
		std::snprintf(buf.get(), size, format.c_str(), args ...);
		return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
	}

	// https://stackoverflow.com/questions/650162/why-the-switch-statement-cannot-be-applied-on-strings
	static constexpr uint32_t hash(const char* data, size_t const size) noexcept{
		uint32_t hash = 5381;

		for(const char *c = data; c < data + size; ++c)
			hash = ((hash << 5) + hash) + (unsigned char) *c;

		return hash;
	}

	
	/*
		Dlang Version
		"proc":		Kind.PROC,
		"dup":		Kind.DUP,
		"swap":		Kind.SWAP,
		"input":	Kind.INPUT,
		"drop":		Kind.POP,
		"conv":		Kind.CONVERT,
		"bind":		Kind.BIND,
		"bindmove": Kind.BIND_MOVE,
		"using":	Kind.USING,
		"assert":	Kind.ASSERT,
		"return":	Kind.RETURN,
		"if":		Kind.IF,
		"elif":		Kind.ELIF,
		"else":		Kind.ELSE,
		"print": 	Kind.PRINT,
		"println": 	Kind.PRINTLN,
		"true":		Kind.BOOL,
		"false":	Kind.BOOL,
		"void" : 	Kind.TYPEID,
		"int":		Kind.TYPEID,
		"float":	Kind.TYPEID,
		"string":	Kind.TYPEID,
		"bool": 	Kind.TYPEID,
		"struct":	Kind.TYPEID,
	*/
	static TokenKind get_keyword_kind(std::string str) {
		switch(hash(str.c_str(), str.size())) {
			case hash("proc", 4):		return TokenKind::PROC;
			case hash("dup", 3):		return TokenKind::DUP;
			case hash("swap", 4):		return TokenKind::SWAP;
			case hash("drop", 4):		return TokenKind::POP;
			case hash("conv", 4):		return TokenKind::CONVERT;
			case hash("bind", 4):		return TokenKind::BIND;
			case hash("using", 5):		return TokenKind::USING;
			case hash("return", 6):		return TokenKind::RETURN;
			case hash("if", 2):			return TokenKind::IF;
			case hash("else", 4):		return TokenKind::ELSE;
			case hash("print", 5):		return TokenKind::PRINT;
			case hash("println", 7):	return TokenKind::PRINTLN;
			case hash("true", 4):		return TokenKind::BOOL_LIT;
			case hash("false", 5):		return TokenKind::BOOL_LIT;
			case hash("void", 4):		return TokenKind::TYPEID;
			case hash("int", 3):		return TokenKind::TYPEID;
			case hash("float", 5):		return TokenKind::TYPEID;
			case hash("bool", 4):		return TokenKind::TYPEID;
			case hash("string", 6):		return TokenKind::TYPEID;

			default:
				return TokenKind::ID;
		}
	}
}