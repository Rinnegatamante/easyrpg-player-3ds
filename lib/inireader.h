/*
 * Read an INI file into easy-to-access name/value pairs.
 *
 * Go to the project home page for more info:
 * http://code.google.com/p/inih/
 *
 * inih and INIReader are released under the New BSD license:
 *
 * Copyright (c) 2009, Brush Technology
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Brush Technology nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY BRUSH TECHNOLOGY ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL BRUSH TECHNOLOGY BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef LCF_INIREADER_H
#define LCF_INIREADER_H

#include <map>
#include <string>

/**
 * Read an INI file into easy-to-access name/value pairs. (Note that I've gone
 * for simplicity here rather than speed, but it should be pretty decent.)
 */
class INIReader
{
public:
	/**
	 * Construct INIReader and parse given filename. See ini.h for more info
	 * about the parsing.
	 */
	INIReader(std::string filename);

	/**
	 * Return the result of ini_parse(), i.e., 0 on success, line number of
	 * first error on parse error, or -1 on file open error.
	 */
	int ParseError() const;

	/**
	 * Get a string value from INI file, returning default_value if not found.
	 */
	std::string Get(std::string section, std::string name,
					std::string default_value);

	/**
	 * Get an integer (long) value from INI file, returning default_value if
	 * not found.
	 */
	long GetInteger(std::string section, std::string name, long default_value);

private:
	int _error;
	std::map<std::string, std::string> _values;
	static std::string MakeKey(std::string section, std::string name);
	static int ValueHandler(void* user, const char* section, const char* name,
							const char* value);
};

#endif
