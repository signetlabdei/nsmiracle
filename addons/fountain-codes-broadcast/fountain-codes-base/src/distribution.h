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

#ifndef __SIGNET_DISTRIBUTION_H__
#define __SIGNET_DISTRIBUTION_H__

#include "random_generator.h"
#include "global.h"

class Distribution
{
    public:
        Distribution( int size );
        ~Distribution();
        void setUniform();
        void setDistribution(const char* distFile);

//         uint8_t getDegree( Random *rand );
        uint8_t getDegree( uint16_t n );

        void printPhy();
        void printSumPhy();

    private:
        int m_size;
        uint16_t *m_phy;
        uint16_t *m_sumPhy;
};

#endif //__SIGNET_DISTRIBUTION_H__
