/** **************************************************************************
**
** This file is part of the Signet Network Programming System.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation.
**
**
**  @author: Alberto Scarpa <alscarpa@dei.unipd.it>
**          cell < 339 4713470 >
**
****************************************************************************/

#ifndef __SIGNET_ENCODER_RANDOM_GENERATOR_H__
#define __SIGNET_ENCODER_RANDOM_GENERATOR_H__

#include "global.h"

class Random
{
    public:
        enum {
            LINUX_TYPE  =   0,
            AL_TYPE     =   1,
            MSP_TYPE    =   2,
            TINYOS_TYPE =   3,
	    MLCG_TYPE	=   4,
        };
        Random( uint16_t type );
        void setSeed( uint16_t seed );
//         uint16_t getStatus();
        uint16_t getNextStatus();
//         uint16_t getNextBit();

        // return int in [0, 2^size-1]
//         uint16_t getNextRand( uint8_t size );

//         void setSeed_tinyos( uint16_t seed );
//         uint16_t getNextRand_tinyos();

    private:
        uint16_t m_seed;
        uint16_t m_type;
        uint64_t m_longSeed;
        uint16_t m_mask;
};

#endif //__SIGNET_ENCODER_RANDOM_GENERATOR_H__
