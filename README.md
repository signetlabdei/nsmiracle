About nsmiracle
------------------

NSMIRACLE is a Multi-InteRfAce Cross-Layer Extension for the Network
Simulator version 2.

Its main features are:
 - to allow researches to re-define and extend the protocol stack in a
   flexible and modular way
 - to provide support for inter-protocol and inter-layer interaction,
   by providing means to exchange cross-layer messages
 - to provide effective support for multiple and heterogeneous
   interfaces
 - to allow easy, modular and hence flexible implementation of
   new wireless technologies
 - to provide support for improved modeling and simulation of wireless
   communications with respect to the well-known shortcomings of the
   current NS release (2.31 at the time of this writing)


------------------
 Contents
-----------------

The nsmiracle distribution contains the following software:

 - the nsmiracle core library
 - several additional libraries which provide:
   + CBR applications
   + Cross-layer message tracers
   + IP functionality (ip-interface correspondance and routing support)
   + TCP (wrapper for ns2 TCP modules)
   + Miracle PHY - a library and API for the implementation of
     wireless technologies
   + port multiplexers
   + UMTS PHY, MAC and RLC
   + 80211 (wrapper for ns2 implementations, including dei80211mr)
   + additional tools for dei80211mr use in nsmiracle
   + mobility  models (e.g., Gauss-Markov mobility)



-----------------------
 Authors
----------------------

The nsmiracle library for the Network Simulator 2 has been developed
by (in alfabetical order):

Nicola Baldo         (baldo@dei.unipd.it)
Federico Maguolo     (maguolof@dei.unipd.it)
Marco Miozzo         (miozzo@dei.unipd.it)

At the time of this release (April 2007), all authors are with the
Department of Information Engineering, University of Padova, Italy.
