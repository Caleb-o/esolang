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
	// https://stackoverflow.com/questions/8518743/get-directory-from-file-path-c/34740989
	static std::string get_directory(std::string path) {
		const size_t pos = path.find_last_of("\\/");
		return (std::string::npos != pos)
			? path.substr(0, pos)
			: "";
	}

	static std::string get_file_no_ext(std::string path) {
		const size_t pos = path.find_last_of("\\/");
		const size_t dot_pos = path.find_last_of('.');

		return (std::string::npos != pos && std::string::npos != dot_pos)
			? path.substr(pos+1, dot_pos - pos - 1)
			: "";
	}

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

	static TokenKind get_keyword_kind(std::string str) {
		switch(hash(str.c_str(), str.size())) {
			case hash("proc", 4):		return TokenKind::PROC;
			case hash("dup", 3):		return TokenKind::DUP;
			case hash("swap", 4):		return TokenKind::SWAP;
			case hash("rot", 3):		return TokenKind::ROT;
			case hash("drop", 4):		return TokenKind::POP;
			case hash("conv", 4):		return TokenKind::CONVERT;
			case hash("bind", 4):		return TokenKind::BIND;
			case hash("strict", 6):		return TokenKind::BIND_STRICT;
			case hash("using", 5):		return TokenKind::USING;
			case hash("unpack", 6):		return TokenKind::UNPACK;
			case hash("return", 6):		return TokenKind::RETURN;
			case hash("or", 2):			return TokenKind::OR;
			case hash("and", 3):		return TokenKind::AND;
			case hash("if", 2):			return TokenKind::IF;
			case hash("else", 4):		return TokenKind::ELSE;
			case hash("loop", 4):		return TokenKind::LOOP;
			case hash("print", 5):		return TokenKind::PRINT;
			case hash("println", 7):	return TokenKind::PRINTLN;
			case hash("true", 4):		return TokenKind::BOOL_LIT;
			case hash("false", 5):		return TokenKind::BOOL_LIT;
			case hash("void", 4):		return TokenKind::TYPEID;
			case hash("int", 3):		return TokenKind::TYPEID;
			case hash("float", 5):		return TokenKind::TYPEID;
			case hash("bool", 4):		return TokenKind::TYPEID;
			case hash("string", 6):		return TokenKind::TYPEID;
			case hash("capture", 7):	return TokenKind::TYPEID;

			default:
				return TokenKind::ID;
		}
	}
}