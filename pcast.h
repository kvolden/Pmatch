#ifndef pcast_h
#define pcast_h

#include <sstream>
#include <string>

namespace pcast{
	template <typename Input>
	Input cast(std::string& value){									// For casting string value to input data type
		Input converted_value;
		std::stringstream stream;
		stream << value;
		stream >> converted_value;
		return converted_value;
	}

	template<>														// Partial template specification: string
	std::string cast(std::string& value){
		return value;
	}

	template<>														// Partial template specification: char
	char cast(std::string& value){
		return value.c_str()[0];
	}

	template<>														// Partial template specification: char
	signed char cast(std::string& value){
		return (signed char)value.c_str()[0];
	}

	template<>														// Partial template specification: char
	unsigned char cast(std::string& value){
		return (unsigned char)value.c_str()[0];
	}

	template<>														// Partial template specification: char
	const char cast(std::string& value){
		return value.c_str()[0];
	}

	template<>														// Partial template specification: char
	const signed char cast(std::string& value){
		return (signed char)value.c_str()[0];
	}

	template<>														// Partial template specification: char
	const unsigned char cast(std::string& value){
		return (unsigned char)value.c_str()[0];
	}

	template<>														// Partial template specification: char*
	char* cast(std::string& value){
		return (char*)value.c_str();
	}

	template<>														// Partial template specification: signed char*
	signed char* cast(std::string& value){
		return (signed char*)value.c_str();
	}

	template<>														// Partial template specification: unsigned char*
	unsigned char* cast(std::string& value){
		return (unsigned char*)value.c_str();
	}

	template<>														// Partial template specification: const char*
	const char* cast(std::string& value){
		return value.c_str();
	}

	template<>														// Partial template specification: const signed char*
	const signed char* cast(std::string& value){
		return (signed char*)value.c_str();
	}

	template<>														// Partial template specification: const unsigned char*
	const unsigned char* cast(std::string& value){
		return (unsigned char*)value.c_str();
	}
	
}

#endif
