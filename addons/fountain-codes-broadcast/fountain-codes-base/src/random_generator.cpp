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

#include "random_generator.h"

#include <stdlib.h>

// LFSR polyno
// mat taps     16 15 13 4
// index taps   15 14 12 3
// binary taps  1101 0000 0000 1000 = 0xD008 = 0x8000 | 0x4000 | 0x1000 | 0x0008

/** Look up table for fast 4bit xor
 *  b0  b1  b2  b3  |   xor
 *  0   0   0   0   |   0
 *  0   0   0   1   |   1
 *  0   0   1   0   |   1
 *  0   0   1   1   |   0
 *
 *  0   1   0   0   |   1
 *  0   1   0   1   |   0
 *  0   1   1   0   |   0
 *  0   1   1   1   |   1
 *
 *  1   0   0   0   |   1
 *  1   0   0   1   |   0
 *  1   0   1   0   |   0
 *  1   0   1   1   |   1
 *
 *  1   1   0   0   |   0
 *  1   1   0   1   |   1
 *  1   1   1   0   |   1
 *  1   1   1   1   |   0
 *
 *  [ 0110 1001 1001 0110 ] = 0x6996
 */

static const uint16_t m_o = 0x6996;

Random :: Random( uint16_t type )
{
    m_seed = (uint16_t)1;
    m_mask = (uint16_t)137*29*m_seed;
    m_longSeed = 1;
    m_type = type;
}

void Random :: setSeed( uint16_t seed )
{
    switch(m_type) {
        case LINUX_TYPE: {
            srand48( seed );
//         printf("setSeed: 0x%4X\t",seed);
				//printf("s:%x\n",seed);
            break;
        }
        case AL_TYPE: {
            m_seed = seed;
				//printf("s:%x\n",m_seed);
            break;
        }
        case MSP_TYPE: {
            m_longSeed = seed;
				//printf("s:%x\n",m_seed);            break;
        }
        case TINYOS_TYPE: {
		  			//di seguito i 48 seed che causano loop infinito 
					//La scelta e` quella di usare il seed precedente a quello
					//brutto, trattandosi di soli 48 su 65535 seed questo non
					//inficia particolarmente le prestazioni. 
					if(seed==0x6d4) break;
					if(seed==0x1744) break;
					if(seed==0x19d0) break;
					if(seed==0x1ccc) break;
					if(seed==0x1cfa) break;
					if(seed==0x1f79) break;
					if(seed==0x2040) break;
					if(seed==0x2ee4) break;
					if(seed==0x3362) break;
					if(seed==0x3914) break;
					if(seed==0x3ac4) break;
					if(seed==0x3c25) break;
					if(seed==0x4ee4) break;
					if(seed==0x4fb9) break;
					if(seed==0x56e4) break;
					if(seed==0x57c4) break;
					if(seed==0x5c40) break;
					if(seed==0x5ecc) break;
					if(seed==0x653a) break;
					if(seed==0x72d4) break;
					if(seed==0x77ec) break;
					if(seed==0x7bfa) break;
					if(seed==0x82d4) break;
					if(seed==0x8759) break;
					if(seed==0x8ad4) break;
					if(seed==0x9744) break;
					if(seed==0x9e40) break;
					if(seed==0xa039) break;
					if(seed==0xa0cc) break;
					if(seed==0xa4cc) break;
					if(seed==0xa704) break;
					if(seed==0xac3a) break;
					if(seed==0xb6c4) break;
					if(seed==0xbcc0) break;
					if(seed==0xbdcc) break;
					if(seed==0xbec4) break;
					if(seed==0xc4d4) break;
					if(seed==0xd2e4) break;
					if(seed==0xd875) break;
					if(seed==0xdac2) break;
					if(seed==0xdc7a) break;
					if(seed==0xddfa) break;
					if(seed==0xe3d9) break;
					if(seed==0xe704) break;
					if(seed==0xef04) break;
					if(seed==0xef39) break;
					if(seed==0xf8c4) break;
					if(seed==0xfd40) break;
            m_seed = 119*119*seed;
            m_mask = 137*29*seed;
				//printf("s:%x\n",seed);			//printf("setseed\n");
            break;
        }

			case MLCG_TYPE: {
				m_seed = (seed + 1);
				//printf("s:%x\n",m_seed);
			}
    }
}

// void Random :: setSeed_tinyos( uint16_t seed )
// {
//     m_seed = (uint16_t)119*119*seed;
//     m_mask = (uint16_t)137*29*m_seed;
// }

// uint16_t Random :: getStatus()
// {
//     return m_seed;
// }

uint16_t Random :: getNextStatus()
{
    uint16_t r = 1;
    switch(m_type) {
        case LINUX_TYPE: {
            r = ((uint16_t)((drand48()*0xFFFF)))&0xFFFF;
            break;
        }
        case AL_TYPE: {
            uint16_t t = (m_seed & 0xD000)>>12;
            t |= (m_seed & 0x0008)>>2;
            m_seed <<= 1;
            m_seed |= ((m_o>>t)&0x0001);
            r = m_seed;
            break;
        }
        case MSP_TYPE: {
            r = ((m_longSeed = m_longSeed * 1103515245 + 12345) % (0xFFFF));
            break;
        }
        case TINYOS_TYPE: {
            uint16_t t = m_seed;
            uint16_t e = ((t&0x8000) != 0) ? 1 : 0;
				//printf("getNext\n");
				t <<= 1;
            if(e == 1)
                t ^= 0x100B;
            t++;
            m_seed = t;
            r = (t^m_mask);
            break;
        }
		  case MLCG_TYPE: {
				uint32_t rt,p,q;
    			uint64 tmpseed;
   			tmpseed =  (uint64)33614U * (uint64)m_seed;
				q = tmpseed; 	/* low */
				q = q >> 1;
				p = tmpseed >> 32 ;		/* hi */
				rt = p + q;
        		if (rt & 0x80000000) { 
	  				rt = rt & 0x7FFFFFFF;
	  				rt++;
				}
				m_seed = rt;
				r=rt; 
			}
    }
	//printf("%x\n",r);
    return r;
}

// uint16_t Random :: getNextRand_tinyos()
// {
//     uint16_t t = m_seed;
//     uint16_t e = ((t&0x8000) != 0) ? 1 : 0;
//     t <<= 1;
//     if(e == 1)
//         t ^= 0x100B;
//     t++;
//     m_seed = t;
//     return (t^m_mask);
// }

// uint16_t Random :: getNextBit()
// {
//     uint16_t out;
//     uint16_t t = (m_seed & 0xD000)>>12;
//     t |= (m_seed & 0x0008)>>2;
//     m_seed <<= 1;
//     out = ((m_o>>t)&0x0001);
//     m_seed |= out;
//     return out;
// }

// uint16_t Random :: getNextRand( uint8_t size )
// {
//     uint16_t out = 0x0000;
//     uint16_t t;
//     int i;
//     if(m_ideal) {
//         out = ((uint16_t)(drand48()*0xFFFF))&0xFFFF;
//     } else {
//     //     printf("[ ");
//         for(i=0; i<size; i++) {
//             out <<= 1;
//             t = getNextBit();
//     //         printf("%i %X   ", t, m_seed);
//             out |= t;
//         }
//     //     printf("]\n");
//     }
//     return out;
// }
