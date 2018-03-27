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

#ifndef __SIGNET_CODEC_H__
#define __SIGNET_CODEC_H__

#include "bit_matrix.h"
#include "distribution.h"
#include "global.h"
#include "random_generator.h"

/** NON MODIFICARE */
// #define MATRIX_NUM_OF_ROW   NUM_OF_MAX_TX
// #define MATRIX_NUM_OF_COL   NUM_OF_SYMBOLS
// 
// #define MATRIX_NUM_OF_BIT_IN_ROW   NUM_OF_MAX_TX
// #define MATRIX_NUM_OF_BIT_IN_COL   NUM_OF_SYMBOLS


/** ***************************************************************************
 *  ENCODER INTERFACE
 *****************************************************************************/
// #define SYMBOLS_PER_BLOCK_2     5   // log2(SYMBOLS_PER_BLOCK)
// #define BLOCK_SIZE              800 // SYMBOLS_PER_BLOCK * SYMBOL_SIZE in bytes

class Codec
{
    public:
        Codec( int blockSize, int symSize, uint16_t type, bool verbose, bool coder );
        ~Codec();
        void setCoder();
        void setDecoder( int pckOverhead );
        void setDistribution(const char* distFile);
        void startNewBlock( int seed );			// start encoding a new block
        void setToCoder( uint8_t* pck );		// set a new pkt to encode
        void setToDecoder( uint8_t* pck, uint16_t id, int seed );
        void getCodedPck( uint8_t* pck, uint16_t id );	// get (fill) pck with seed==id
        int decode(); 					// try decoding; returns (matrix_size - matrix_rank)
        uint8_t * getData();

        void printMatrix();
        void printData();
//     uint16_t getRandPhy( uint16_t n );
    private:
        void swapRow( int r1, int r2 );
        void xorRow( int r1, int r2 );
        void getSymbolsToMix( uint16_t *p, uint16_t n );
    private:
        int m_action;
        int m_blockSize;
        int m_symSize;
        int m_pckIdx;
        int m_pckOverhead;
        int m_randDegreeCount;
        uint16_t m_seed;
        bool m_verbose;
	bool m_coder;
        uint16_t m_type;
        uint8_t *m_data;
        Random *m_rand;
        Random *m_randDegree;
        Distribution *m_dist;
        BitMatrix *m_matrix;
};

#endif //__SIGNET_CODEC_H__
