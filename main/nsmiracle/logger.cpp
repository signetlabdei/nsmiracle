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
 * @file   uwlogger.cpp
 * @author Vincenzo Cimino
 * @version 1.0.0
 *
 */

#include "logger.h"
#include <chrono>
#include <iomanip>


Logger::Logger()
	: node_id_(0)
	, log_level_(LogLevel::NONE)
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
Logger::printOnLog(const std::string &log_level, const std::string &module,
		const std::string &message)
{
	if (log_level_ == LogLevel::NONE || log_level_ < strToLog(log_level))
		return;

	const double timestamp =
			(double) (std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch())
							.count()) /
			1000.0;

	if (!log_file_.empty()) {
		if (!log_out_.is_open()) {
			log_out_.open(log_file_.c_str(), ios::app);
			if (log_out_.fail()) {
				std::cerr << "[" << NOW
						  << "]::FAILED open log file with error : "
						  << strerror(errno) << std::endl;
				return;
			}
		}

		log_out_ << std::setprecision(15) << std::left << log_level << "::[" << timestamp
				<< "]::[" << NOW << "]::" << module << "(" << node_id_
				<< ")::" << message << endl;
		log_out_.flush();
	} else {
		std::cout << std::setprecision(15) << std::left << log_level << "::["
				  << timestamp << "]::[" << NOW << "]::" << module << "("
				  << node_id_ << ")::" << message << std::endl;
	}
}
