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

#include "codec.h"

#include <math.h>
#include <stdlib.h>

enum {
    ACTION_CODER    =   0,
    ACTION_DECODER  =   1,
};

Codec :: Codec( int blockSize, int symSize, uint16_t type, bool verbose ,bool coder) : m_data(0), m_matrix(0)
{
    m_action = ACTION_CODER;
    m_blockSize = blockSize;
    m_symSize = symSize;
    m_type = type;
    m_verbose = verbose;
    m_rand = new Random( m_type );
    m_randDegree = new Random( m_type );
    m_randDegree->setSeed( 0x9D25 );
    m_randDegreeCount = 0;
    m_dist = new Distribution( blockSize );
    m_dist->setUniform();
	 m_coder = coder;
}

Codec :: ~Codec()
{
    free(m_data);
    delete m_rand;
    delete m_randDegree;
    delete m_dist;
    delete m_matrix;
}

void Codec :: setCoder()
{
    m_action = ACTION_CODER;
    free(m_data);
    m_data = (uint8_t *)malloc( sizeof(uint8_t)*m_symSize*m_blockSize );
}

void Codec :: setDecoder( int pckOverhead )
{
    m_action = ACTION_DECODER;
    m_pckOverhead = pckOverhead;
    free(m_data);
    m_data = (uint8_t *)malloc( sizeof(uint8_t)*m_symSize*(m_blockSize+m_pckOverhead) );
    delete m_matrix;
    m_matrix = new BitMatrix( m_blockSize+m_pckOverhead, m_blockSize );
}

void Codec :: setDistribution(const char *distFile)
{
    m_dist->setDistribution(distFile);
}

/** Functions implementations */
void Codec :: startNewBlock( int seed )
{
    if(m_verbose) {
        printf("Codec::startNewBlock() called with seed:%d\n",seed);
    }
    int s, i;
    // clear data
    for(s=0; s<m_blockSize; s++) {
        for(i=0; i<m_symSize; i++) {
            m_data[ s*m_symSize + i ] = 0;
        }
    }
    m_randDegree->setSeed( seed );
    m_seed = seed;
    m_pckIdx = 0;
    m_randDegreeCount = 0;
}

void Codec :: setToCoder( uint8_t* pck )
{
    int i;
    // copy data
    for(i=0; i<m_symSize; i++) {
        m_data[ m_pckIdx*m_symSize + i ] = pck[i];
//         printf("%X  ", m_data[ m_pckIdx*m_symSize + i ]);
    }
//     printf("\n");
    // update current pck index
    m_pckIdx = (m_pckIdx+1) % m_blockSize;
}

void Codec :: getCodedPck( uint8_t *pck, uint16_t id )
{
    uint16_t i, n, t=1;
    uint16_t symbolsToMix[m_blockSize];

    // 1. get num of symbols to mix
    m_randDegree->setSeed( m_seed );
    m_randDegreeCount = 0;
    while(m_randDegreeCount<id) {
        if(m_verbose) {
            printf("\e[34m.\e[0m");
        }
        t = m_randDegree->getNextStatus();
		  //printf("degree:%x\n",t);
        m_randDegreeCount++;
    }
//     t = m_randDegree->getStatus();
    if(m_verbose) {
        printf("\e[36m %4X \e[0m", t);
    }
    n = m_dist->getDegree( t );
    if(m_verbose) {
        printf("deg: %2X\t", n);
    }

    // 2. set seed
//     t = m_randDegree->getStatus();
    if(m_verbose) {
        printf("\e[32m %4X \e[0m", t);
    }
    m_rand->setSeed( t );
	 if (m_coder )printf(" seed:%x", t);
    // 3. get syndols ID
    getSymbolsToMix( symbolsToMix, n );

    // 4. build output pck
    for(i=0; i<m_symSize; i++) {
        pck[i] = 0x00;
    }

    if(m_verbose) {
        printf("mix: \e[34m[ ");
    }
    for(t=0; t<m_blockSize; t++) {
        if(symbolsToMix[t] != 0) {
            if(m_verbose) {
                printf("%2X ", t);
            }
            for(i=0; i<m_symSize; i++) {
                pck[i] ^= m_data[ t*m_symSize + i ];
            }
        }
    }
    if(m_verbose) {
        printf("]\e[0m\t \n");
    }
}


void Codec :: setToDecoder( uint8_t *pck, uint16_t id, int seed )
{
    uint16_t i, n, t=1;
    uint16_t symbolsToMix[m_blockSize];

    // 1. get num of symbols to mix
    m_randDegree->setSeed( seed ); // ERA m_seed
    m_randDegreeCount = 0;
    while(m_randDegreeCount<id) {
        t = m_randDegree->getNextStatus();
        if(m_verbose) {
            printf("\e[35m.\e[0m");
        }
        m_randDegreeCount++;
    }
    n = m_dist->getDegree( t/*m_randDegree->getStatus()*/ );

    // 2. set seed
//     t = m_randDegree->getStatus();
    m_rand->setSeed( t );
    // 3. get syndols IDs
    getSymbolsToMix( symbolsToMix, n );

    // 3.5 print symbol to mix
    if(m_verbose) {
        printf("\e[35m[ ");
        for(t=0; t<m_blockSize; t++) {
            if(symbolsToMix[t] != 0) {
                printf("%2X ", t);
            }
        }
        printf("]\e[0m\n");
    }

//     printf("\e[33m%i\e[0m\t", m_pckIdx);
    // 4. store coded pck
    for(i=0; i<m_symSize; i++) {
        m_data[ m_pckIdx*m_symSize + i ] = pck[i];
    }

    // 5. Add entry to coder matrix
    for(t=0; t<m_blockSize; t++) {
        if(symbolsToMix[t] != 0) {
            m_matrix->setBitRC( m_pckIdx, t );
//             printf("\e[33m [%i, %i] \e[0m",m_pckIdx, t);
        }
    }

    m_pckIdx++;
//     printf("\e[32m%i %i %i \e[0m\n", m_pckIdx, m_blockSize, m_pckOverhead, m_blockSize+m_pckOverhead);
    if(m_pckIdx>=m_blockSize+m_pckOverhead) {
        m_pckIdx = m_blockSize;
//         printf("\e[31m%i\e[0m\n", m_pckIdx);
    }
}

int Codec :: decode()
{
    int cIdx, rIdx, r;
    int n_blackCol = 0;
    bool find = false;

    if(m_verbose) {
        printf("Encoded matrix at receiver:\n");
        //m_matrix->printMatrix();
    }

    // su tutte le colonne
    rIdx = 0;
    for(cIdx=0; cIdx<m_blockSize; cIdx++) {
//         printf("\ncIdx: %i\t", cIdx);
        // cerco una colonna con un 1
        r = rIdx;
        find = false;
        while(!find && r<m_blockSize+m_pckOverhead) {
            if(m_matrix->testBitRC(r, cIdx)) {
                find = true;
            } else {
                r++;
            }
        }
        if(!find) {
            n_blackCol++;
//             printf("nf\n");
            continue;
        }
//         printf("f  %i\t", r);
        // porto la riga in cima
        if(rIdx != r) {
            swapRow(rIdx, r);
        }
        // pulisco la colonna
        for(r=0; r<m_blockSize+m_pckOverhead; r++) {
            if(r==rIdx) continue;
            if(m_matrix->testBitRC(r, cIdx)) {
//                 printf("%i ", r);
                xorRow(rIdx, r);
            }
        }
        rIdx++;
    }

    if(m_verbose) {
        printf("\nDecoded matrix at receiver:\n");
        //m_matrix->printMatrix();
    }
    return n_blackCol;
}

void Codec :: printMatrix()
{
    m_matrix->printMatrix();
    printf("\n");
}

uint8_t * Codec :: getData()
{
    return m_data;
}

void Codec :: getSymbolsToMix( uint16_t *p, uint16_t n )
{
    int i, t, count;
    for(i=0; i<m_blockSize; i++) {
        p[i] = 0;
    }
    i = n;
	 //printf("i: %d -",i);
    // OPTIMIZATION: random permutation
    while(i != 0) {
		  count++;
			//printf("%d ",count);
			if (count==1000) {
				if(m_coder)printf(" - brutto\n");
				break;
			}
//         t = (m_rand->getNextStatus()>>(16-(int)ceil(log2(m_blockSize))));
        t = m_rand->getNextStatus() % m_blockSize;
//		  printf("random: %x, i: %d\n",t,i);
        if(p[t]==0) {
            p[t] = 1;
            i--;
        } else {
            // A dot indicates we chose to mix a symbol already chosen.
            // Random permutation solve the problem.
//             printf(". ");
        }
    }
//	if(m_coder)printf(" - buono\n");
}

void Codec :: swapRow( int r1, int r2 )
{
    int c;
    // swap matrix
    m_matrix->swapRows( r1, r2 );
    // swap data
    for(c=0; c<m_symSize; c++) { 
        //r1->t   r2->r1    t->r2
        #if 0
        t = m_data[r1*m_symSize + c];
        m_data[r1*m_symSize + c] = m_data[r2*m_symSize + c];
        m_data[r2*m_symSize + c] = t;
        #endif
        m_data[r1*m_symSize + c] ^= m_data[r2*m_symSize + c];
        m_data[r2*m_symSize + c] ^= m_data[r1*m_symSize + c];
        m_data[r1*m_symSize + c] ^= m_data[r2*m_symSize + c];
    }
}

void Codec :: xorRow( int r1, int r2 )
{
    int c;
    // xor matrix
    m_matrix->xorRows( r1, r2 );
    // xor data
    for(c=0; c<m_symSize; c++) {
        m_data[r2*m_symSize + c] ^= m_data[r1*m_symSize + c];
    }
}

void Codec :: printData( )
{
    int r, c;
    for(r=0; r<m_blockSize+m_pckOverhead; r++) {
        printf("%2i [ ", r);
        for(c=0; c<m_symSize; c++) {
            printf("%2X ", m_data[r*m_symSize+c]);
        }
        printf("]\n");
    }
}

uint16_t Codec::returnSeed()
{
 return m_seed;
}