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
 * given module of a node.
 *
 */

#ifndef LOGGER
#define LOGGER

#include "object.h"
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

class Logger
{
public:
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
	 * Set the Logger node id.
	 *
	 * @param node_id the integer value that will be the logger node id.
	 *
	 **/
	inline void
	setLogNodeId(size_t node_id)
	{
		node_id_ = node_id;
	}

	/**
	 * Set the Logger log level.
	 *
	 * @param log_level the string representings the amount of logs being
	 *generated. Valid values are ERR,INFO,DBG.
	 *
	 **/
	inline void
	setLogLevel(const std::string &log_level)
	{
		log_level_ = strToLog(log_level);
	}

	/**
	 * Set the Logger file name.
	 *
	 * @param log_file the string representings the name of the file where to
	 *store log messages.
	 *
	 **/
	inline void
	setLogFile(const std::string &log_file)
	{
		log_file_ = log_file;
	}

	/**
	 * Method to print the provided log message either to the console
	 * or to a file if a log_file name is specified.
	 * The log message is printed only if log_level_ >= log_level.
	 *
	 * @param log_level String of valid log level (ERR/INFO/DBG).
	 * @param module String representing the name of the module.
	 * @param message String log message.
	 *
	 */
	void printOnLog(const std::string &log_level, const std::string &module,
			const std::string &message);

protected:
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
	 * Dictionary of log levels.
	 *
	 */
	const std::unordered_map<std::string, LogLevel> log_dict = {
			{"ERR", LogLevel::ERROR},
			{"INFO", LogLevel::INFO},
			{"DBG", LogLevel::DEBUG}};

	/**
	 * Method that converts a string representing the loglevel
	 * into the enum type of loglevel.
	 *
	 * @param log_level string to be converted
	 *
	 * @return LogLevel converted enum type of loglevel
	 */
	LogLevel
	strToLog(const std::string &log_level) const
	{
		auto ll = log_dict.find(log_level);

		if (ll != log_dict.end())
			return ll->second;

		std::cerr << "[" << NOW
				  << "]::Logger::INVALID Log Level. Default to NONE"
				  << std::endl;

		return LogLevel::NONE;
	}

	size_t node_id_; /**< Id of the node to which log messages are associated.*/
	LogLevel log_level_; /**< Log level of log messages.*/
	std::string log_file_; /**< Name of the file where to write log messages.*/
	std::ofstream log_out_; /**< Output stream to print into file log messages.*/
};

#endif
