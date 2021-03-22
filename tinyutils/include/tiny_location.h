#ifndef TINY_LOCATION_H
#define	TINY_LOCATION_H

#include "tiny_string.h"
namespace tiny
{
	class  Location {
	public:
		// Constructor should be called with a long-lived char*, such as __FILE__.
		// It assumes the provided value will persist as a global constant, and it
		// will not make a copy of it.
		//
		// TODO(deadbeef): Tracing is currently limited to 2 arguments, which is
		// why the file name and line number are combined into one argument.
		//
		// Once TracingV2 is available, separate the file name and line number.
		Location(const char* function_name, const char* file_and_line);
		Location();
		Location(const Location& other);
		Location& operator=(const Location& other);

		const char* function_name() const { return function_name_; }
		const char* file_and_line() const { return file_and_line_; }

		std::string ToString() const;

	private:
		const char* function_name_;
		const char* file_and_line_;
	};
}

inline std::ostream& operator << (std::ostream& out, const tiny::Location& l)
{
	out << l.ToString();
	return out;
}


#define TINY_FROM_HERE_WITH_FUNCTION(function_name) \
  tiny::Location(function_name, __FILE__ ":" STRINGIZE(__LINE__))

// Define a macro to record the current source location.
#define TINY_FROM_HERE			TINY_FROM_HERE_WITH_FUNCTION(__FUNCTION__)


#endif // !TINY_LOCATION_H
