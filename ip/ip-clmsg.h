
#ifndef IP_CLMSG_H
#define IP_CLMSG_H

#include <clmessage.h>


#define IP_CLMSG_VERBOSITY 5	// verbosity of this message
#define IP_CLMSG_UPD_ROUTE_VERBOSITY 2  // verbosity of this message


/* Message for getting modules address */
extern ClMessage_t IP_CLMSG_SEND_ADDR;
extern ClMessage_t IP_CLMSG_REQ_ADDR;
extern ClMessage_t IP_CLMSG_UPD_ROUTE;



/**
 * Message for requesting modules address
 */
class IPClMsgReqAddr : public ClMessage
{
	public:
		/** Constructor */
		IPClMsgReqAddr(int src);

		/** Copy constructor */
		IPClMsgReqAddr(IPClMsgReqAddr *m);

		/**
		 * Creates a copy of the object
		 * @return Pointer to a copy of the object
		 */
		ClMessage* copy();
};

/**
 * Message for returning modules address
 */
class IPClMsgSendAddr : public ClMessage
{
	public:
		/** Constructor */
		IPClMsgSendAddr();
		IPClMsgSendAddr(DestinationType dtype, int value);

		/** Copy constructor */
		IPClMsgSendAddr(IPClMsgSendAddr *m);

		/**
		 * Creates a copy of the object
		 * @return Pointer to a copy of the object
		 */
		ClMessage* copy();

		/**
		 * Sets the address for network layer
		 * @param symboltime The address
		 */
		void setAddr(nsaddr_t addr);

		/**
		 * Get the address for network layer
		 * @return address for network layer
		 */
		nsaddr_t getAddr();
	private:
		/** network layer address */
		nsaddr_t addr_;
};


class Packet;


class IpClMsgUpdRoute : public ClMessage {
  
  
  public:
    
    
  IpClMsgUpdRoute( Packet* p );
  
  virtual ~IpClMsgUpdRoute() { }
  
  virtual ClMessage* copy();  // copy the message
  
  
  Packet* getPacket() { return packet; }
  
  void setPacket( Packet* p ) { packet = p; }
   

  protected:
  
  
  Packet* packet;

  
};



#endif /*  IP_CLMSG_T */
