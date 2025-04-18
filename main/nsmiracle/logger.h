//
// Copyright (c) 2025 Regents of the SIGNET lab, University of Padova.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Padova (SIGNET lab) nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/**
 * @file   logger.h
 * @author Vincenzo Cimino
 * @version 1.0.0
 *
 * @brief A Logger class represents an object used to print log messages from a
 * given plugin/module of a node.
 *
 */

#ifndef LOGGER
#define LOGGER

#include <fstream>
#include <string>

class Logger
{
public:
	/**
	 * Enum representing the amount of logs being generated.
	 * NONE: no logs will be generated
	 * ERROR: only errors will be generated
	 * INFO : general info about the algorithms running and ERROR
	 * DEBUG: details that allow to understand the execution flow and ERROR +
	 * INFO
	 *
	 */
	enum class LogLevel { NONE = 0, ERROR = 1, INFO = 2, DEBUG = 3 };

	/**
	 * Logger constructor.
	 *
	 */
	Logger();

	/**
	 * Logger destructor
	 *
	 */
	~Logger();

	/**
	 * Set the Logger log level.
	 *
	 * @param log_level a integer representings the amount of logs being
	 * generated. Valid values are 1 = ERR, 2 = INFO, 3 = DBG.
	 *
	 **/
	void setLogLevel(int log_level);

	/**
	 * Set the Logger file name.
	 *
	 * @param log_file the string representings the name of the file where to
	 * store log messages.
	 *
	 **/
	void setLogFile(const std::string &log_file);

	/**
	 * Method to print the provided log message either to the console
	 * or to a file if a log_file name is specified.
	 * The log message is printed only if log_level_ >= log_level.
	 *
	 * @param log_level LogLevel representing the amout of logs.
	 * @param message String log message.
	 *
	 */
	void printOnLog(LogLevel log_level, const std::string &message);

protected:
	LogLevel log_level_; /**< Log level of log messages.*/
	std::string log_file_; /**< Name of the file where to write log messages.*/
	std::ofstream log_out_; /**< Output stream to file for log messages.*/
};

#endif
