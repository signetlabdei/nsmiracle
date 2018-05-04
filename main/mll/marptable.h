/*
 * Copyright (c) 2008, Karlstad University
 * Erik Andersson, Emil Ljungdahl, Lars-Olof Moilanen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: marptable.h 92 2008-03-25 19:35:54Z emil $
 */

#ifndef ARPTABLE_H
#define ARPTABLE_H

#include <packet.h>
#include <map>

#ifndef EADDRNOTAVAIL
#define EADDRNOTAVAIL 125
#endif /* !EADDRNOTAVAIL */

#ifndef DROP_IFQ_ARP_FULL
#define DROP_IFQ_ARP_FULL               "ARP"
#endif

#define ARP_MAX_REQUEST_COUNT   3

/**
 * ARP table entry
 */
class MARPEntry {
	public:
		/**
		 * Constructor
		 * @param dst Destination network address
		 */
		MARPEntry(nsaddr_t dst) : up_(0), ipaddr_(dst), macaddr_(0), hold_(0), count_(0) { }

		/** Is address up? */
		int		up_;
		/** network address */
		nsaddr_t	ipaddr_;
		/** mac address */
		int		macaddr_;
		/** cached packet */
		Packet		*hold_;
		/** Number of tries */
		int		count_;
};

/**
 * ARP table
 */
class MARPTable {
	public:
		/** Constructor */
		MARPTable();
		/** Desctructor */
		~MARPTable();

		/**
		 * Add entry to ARP table
		 * @param entry MARPEntry to add
		 */
		void addEntry(MARPEntry *entry);

		/**
		 * Lookup entry in table
		 * @param addr network address to look for
		 * @return If entry found, a pointer to an MARPEntry, else null
		 */
		MARPEntry *lookup(nsaddr_t addr);

		/**
		 * Remove all entries in table
		 */
		void clear();

	private:
		/** The ARP table as an associative array */
		map<nsaddr_t, MARPEntry*> table_;
};

#endif /* ARPTABLE_H */
