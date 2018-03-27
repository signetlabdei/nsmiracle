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

#include "distribution.h"

#include <stdio.h>
#include <stdlib.h>

/**
 *  Cumulative sum of phy(x).
 *  phy(i) = prob. to mix i symbols.
 *  sumPhi[i] = sum from 1 to i of phy(i) = phy(1) + ... + phy(i)
 */
// static uint16_t sumPhi[SYMBOLS_PER_BLOCK] = 
// {   0x17FD, 0x27FB, 0x2FFA, 0x37F9, 0x3FF8, 0x47F7, 0x4FF6, 0x57F5,
//     0x5FF4, 0x67F3, 0x6FF2, 0x77F1, 0x7FF0, 0x87EF, 0x8FEE, 0x97ED,
//     0x9FEC, 0xA7EB, 0xAFEA, 0xB7E9, 0xBFE8, 0xC7E7, 0xCFE6, 0xD7E5,
//     0xDFE4, 0xE7E3, 0xEFE3, 0xF7E2, 0xFA00, 0xFB00, 0xFD00, 0xFFFF };

/*
    0.25; 0.15; ...0.2/28...; 0.25; 0.15
    0xFFFF * 0.25 = 0x3FFF          4001
    0xFFFF * 0.15 = 0x2666          
    0xFFFF * 0.2  = 0x3333
    (0xFFFF * 0.2)/28 = 0x01D4      3330
*/
// static uint16_t sumPhi[SYMBOLS_PER_BLOCK] =
// {   0x4001, 0x6667, 0x683B, 0x6A0F, 0x6BE3, 0x6DB7, 0x6F8B, 0x715F,
//     0x7333, 0x7507, 0x76DB, 0x78AF, 0x7A83, 0x7C57, 0x7E2B, 0x7FFF,
//     0x81D3, 0x83A7, 0x857B, 0x874F, 0x8923, 0x8AF7, 0x8CCB, 0x8E9F,
//     0x9073, 0x9247, 0x941B, 0x95EF, 0x97C3, 0x9997, 0xD998, 0xFFFF  };

Distribution :: Distribution( int size ) : m_phy(0), m_sumPhy(0)
{
    m_size = size;
    m_phy = (uint16_t*)malloc( sizeof(uint16_t)*m_size );
    m_sumPhy = (uint16_t*)malloc( sizeof(uint16_t)*m_size );
    setUniform();
}

Distribution :: ~Distribution()
{
    free( m_phy );
    m_phy = 0;
    free( m_sumPhy );
    m_sumPhy = 0;
}

void Distribution :: setUniform()
{
    int i;
    float d = (float)0xFFFF/m_size;
    for(i=0; i<m_size; i++) {
        m_phy[i] = (int)d;
        m_sumPhy[i] = (int)d*(i+1);
    }
    m_sumPhy[m_size-1] = 0xFFFF;
}

void Distribution :: setDistribution(const char* distFile)
{
    int i;
//     printf("setD\n");
    for(i=0; i<m_size; i++) {
        m_phy[i] = 0;
    }
    FILE *f = fopen(distFile, "r");
    if(!f) {
        printf("\e[31mERR: non posso aprire: %s\e[0m\n", distFile);
        setUniform();
        return;
    }
    int p, v;
    while(fscanf(f, "%i,%i\n", &p, &v)!=EOF) {
        if(p<=0 || p>m_size || v<0 || v>0xFFFF)
            continue;
        m_phy[p-1] = (uint16_t)v;
//         printf("%i,%i\n", p,v); fflush(stdout);
    }
    fclose(f);
    m_sumPhy[0] = m_phy[0];
    for(i=1; i<m_size; i++) {
        m_sumPhy[i] = m_sumPhy[i-1] + m_phy[i];
    }
    if(m_sumPhy[m_size-1]!=0xFFFF) {
        printf("\e[31mERR: %s non contiene una distribuzione valida di dim %i\e[0m\n", distFile, m_size);
        setUniform();
    }
//     printf("end setD\n");
//     printSumPhy();
}

void Distribution :: printPhy()
{
    int i;
    printf("{ ");
    for(i=1; i<=m_size; i++) {
        printf("0x%X, ", m_phy[i-1]);
        if(!(i%16))
            printf("\n");
    }
    printf(" }\n");
}

void Distribution :: printSumPhy()
{
    int i;
    printf("{ ");
    for(i=1; i<=m_size; i++) {
        printf("0x%X, ", m_sumPhy[i-1]);
        if(!(i%16))
            printf("\n");
    }
    printf(" }\n");
}

// uint8_t Distribution :: getDegree( Random *rand )
// {
//     int i;
//     uint16_t n;
//     if(!rand) {
//         return 0;
//     }
//     n = rand->getStatus( );
//     rand->setSeed( n<<3|n );
//     printf("\e[35m %4X\e[0m\t", n);
//     for(i=0; i<5; i++) {
//         n = rand->getNextStatus( );
// //         printf("\e[35m %4X\e[0m\t", n);
//     }
//     n = rand->getStatus( );
//     printf("\e[35m %4X\e[0m\t", n);
//     for(i=0; i<m_size; i++) {
//         if(n<=m_sumPhy[i]) {
// //             printf("%2i\n", i+1);
//             return i+1;
//         }
//     }
//     // we can arrive here only if sumPhy[SYMBOLS_PER_BLOCK-1] != 0xFFFF
//     printf("ERR: can not find how many symbols to mix.\nCheck sumPhy!!\n");
//     return 1;
// }

uint8_t Distribution :: getDegree( uint16_t n )
{
    int i;
    for(i=0; i<m_size; i++) {
        if(n<=m_sumPhy[i]) {
//             printf("%2i\n", i+1);
            return i+1;
        }
    }
    // we can arrive here only if sumPhy[SYMBOLS_PER_BLOCK-1] != 0xFFFF
    printf("ERR: can not find how many symbols to mix.\nCheck sumPhy!!\n");
    return 1;
}

// /** -----------------------------------------
//  *  OPTIMIZATION: Se Phi è uniforme conviene usare una ricerca per bisezione
//  *  ma se sono più probabili valori bassi conviene una ricerca per scansione
//  */
// uint16_t getNumSymbols()
// {
//     int i;
//     uint16_t n = getNextRand( 16 );
// //     printf("getNumPck: %4X\t", n);
//     for(i=0; i<SYMBOLS_PER_BLOCK; i++) {
//         if(n<=sumPhi[i]) {
// //             printf("%2i\n", i+1);
//             return i+1;
//         }
//     }
//     // we can arrive here only if sumPhy[SYMBOLS_PER_BLOCK-1] != 0xFFFF
//     printf("ERR: can not find how many symbols to mix.\nCheck sumPhy!!\n");
//     return 1;
// }

// uint16_t getRandPhy( uint16_t n )
// {
//     int i;
//     for(i=0; i<SYMBOLS_PER_BLOCK; i++) {
//         if(n<=sumPhi[i]) {
//             return i+1;
//         }
//     }
//     // we can arrive here only if sumPhy[SYMBOLS_PER_BLOCK-1] != 0xFFFF
//     printf("ERR: can not find how many symbols to mix.\nCheck sumPhy!!\n");
//     return 1;
// }
