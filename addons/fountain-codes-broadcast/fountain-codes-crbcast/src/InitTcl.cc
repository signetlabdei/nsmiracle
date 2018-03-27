static char code[] = "\n\
PacketHeaderManager set tab_(PacketHeader/FCM) 1\n\
\n\
Module/FCM/SimpleMAC set packetSize_      500\n\
Module/FCM/SimpleMAC set period_          0.2\n\
Module/FCM/SimpleMAC set destPort_        0\n\
Module/FCM/SimpleMAC set destAddr_        0\n\
Module/FCM/SimpleMAC set debug_           0\n\
Module/FCM/SimpleMAC set alpha_           0.9 \n\
Module/FCM/SimpleMAC set blockSize_       32\n\
Module/FCM/SimpleMAC set symSize_         1\n\
Module/FCM/SimpleMAC set totBlock_        2\n\
Module/FCM/SimpleMAC set genType_         0\n\
Module/FCM/SimpleMAC set seed_            100\n\
Module/FCM/SimpleMAC set max_rounds_      5\n\
Module/FCM/SimpleMAC set propSpeed_       1500\n\
Module/FCM/SimpleMAC set n_class          1\n\
Module/FCM/SimpleMAC set d_max            20000\n\
Module/FCM/SimpleMAC set d_lower          1000\n\
Module/FCM/SimpleMAC set d_upper          6000\n\
Module/FCM/SimpleMAC set TxRadius_        5000.0\n\
\n\
Module/FCM/SimpleMAC/MHopCtrl set debug_       0\n\
Module/FCM/SimpleMAC/MHopCtrl set TxRadius_    5000.0\n\
Module/FCM/SimpleMAC/MHopCtrl set TotalBlocks_ 1\n\
Module/FCM/SimpleMAC/MHopCtrl set Rmax_        15000.0\n\
Module/FCM/SimpleMAC/MHopCtrl set genType_     0\n\
Module/FCM/SimpleMAC/MHopCtrl set RandSeed_    10\n\
Module/FCM/SimpleMAC/MHopCtrl set Lambda_      5\n\
\n\
\n\
Module/FCM/SimpleMAC/CRBCastCtrl set debug_       0\n\
Module/FCM/SimpleMAC/CRBCastCtrl set TxRadius_    6000.0\n\
Module/FCM/SimpleMAC/CRBCastCtrl set TotalBlocks_ 1\n\
Module/FCM/SimpleMAC/CRBCastCtrl set genType_     0\n\
Module/FCM/SimpleMAC/CRBCastCtrl set PbCast_      0.5\n\
Module/FCM/SimpleMAC/CRBCastCtrl set RandSeed_    10\n\
Module/FCM/SimpleMAC/CRBCastCtrl set Rmax_        15000.0\n\
\n\
\n\
";
#include "tclcl.h"
EmbeddedTcl InitTclCode(code);
