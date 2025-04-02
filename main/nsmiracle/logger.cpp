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
 * @file   logger.cpp
 * @author Vincenzo Cimino
 * @version 1.0.0
 *
 */

#include "logger.h"
#include "object.h"
#include <cstring>
#include <iostream>

Logger::Logger()
	: log_level_(LogLevel::NONE)
	, log_file_()
	, log_out_()
{
}

Logger::~Logger()
{
	if (log_out_.is_open()) {
		log_out_.flush();
		log_out_.close();
	}
}

void
Logger::setLogLevel(int log_level)
{
	switch (log_level) {
		case 1:
			log_level_ = LogLevel::ERROR;
			break;
		case 2:
			log_level_ = LogLevel::INFO;
			break;
		case 3:
			log_level_ = LogLevel::DEBUG;
			break;
		default:
			log_level_ = LogLevel::NONE;
	}
}

void
Logger::setLogFile(const std::string &log_file)
{
	log_file_ = log_file;

	if (!log_out_.is_open()) {
		log_out_.open(log_file_.c_str(), std::ios::app);
		if (log_out_.fail()) {
			std::cout << "[" << NOW
					  << "]::Logger::setLogFile::FAILED open log file with "
					  << "error : " << strerror(errno) << std::endl;
			return;
		}
	}
}

void
Logger::printOnLog(Logger::LogLevel log_level, const std::string &message)
{
	if (log_level_ == LogLevel::NONE || log_level_ < log_level)
		return;

	auto level_str{""};
	switch (log_level) {
		case LogLevel::ERROR:
			level_str = "ERR";
			break;
		case LogLevel::INFO:
			level_str = "INFO";
			break;
		case LogLevel::DEBUG:
			level_str = "DBG";
			break;
		default:
			return;
	}

	if (log_out_.is_open()) {
		log_out_ << "[" << NOW << "]::" << level_str << "::" << message
				 << std::endl;
		log_out_.flush();
	} else {
		std::cout << "[" << NOW << "]::" << level_str << "::" << message
				  << std::endl;
	}
}
