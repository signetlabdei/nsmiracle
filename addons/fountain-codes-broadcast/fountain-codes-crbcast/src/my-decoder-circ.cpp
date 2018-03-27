/** **************************************************************************
**
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation.
**
**
**  @author: Michele Rossi <Rossi@dei.unipd.it>
**
**
****************************************************************************/

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>

#include "ranrotw.h"
#include "time.h"

// Include encoding/decoding libraries
#include "codec.h"

// Default values
#define SYMBOL_SIZE             1 	// size, in bytes, of an encoded symbol (packet)
#define BLOCK_SIZE              32 	// number of symbols in a transmission block  /* Put 32 HEREEEEE!!!! */
#define XIMIN			1
#define XIMAX			16
#define STRLEN			200
#define MIN(x,y) 		(x>=y ? y : x)
#define MAX(x,y) 		(x>=y ? x : y)

// Indices for the topology classes and the users matrix
#define N_NODES		0
#define DIST		1
#define SNR			2
#define PER			3

#define US_CLASS	0
#define US_PER		1

#define	UWSPEED		1.5	// Km/sec!

#define TD			1	// Data tx time (e.g., 1000 bits at 1 kbps)
#define TA			0.1	// ACK tx time


//PC: mu modified to be 4-D (includes class ID)
int *xmin, *xmax, ****mu;

long int seed = time(0); 
TRanrotWGenerator myrand(seed);

// Statistics
long	nfailures=0, ntot=0, nrounds=0;
long	ntransmissions=0, ntransmissions2=0;
double	delay=0;	
long	zeta;

void printUsage( char *name );

int xxmin(int round)
{
	int acc=0;	
	int i;
	
	if (acc==0) 
	for (i=1; i<=round; i++) acc += xmin[i];

	return acc;
}

int xxmax(int round)
{
	int acc=0;
	int i;
	
	if (acc==0) 
	for (i=1; i<=round; i++) acc += xmax[i];

	return acc;
}

// PC: mod to include class ID in assignments and to detect class change when reading file
void read_Policy_File(char *PolicyFileName)
{
	FILE	*fp;
	int 	ret=0, c, x, m, l, op;
	char	str[STRLEN];
	
	printf("Opening optimal policy file: %s \n", PolicyFileName);
	
	fp = fopen(PolicyFileName, "r");

/*
	fscanf(fp, "%s", str);
	c = atoi(str);
	fscanf(fp, "%s", str);
	l = atoi(str);
	fscanf(fp, "%s", str);
	op = atoi(str);			// optimal policy value
	
	mu[c][xmin[1]][0][l] = op;
	// printf("%d\t%d\n", l, op);
*/
	
	while (true)
	{
		ret = fscanf(fp, "%s", str);
		if (ret==EOF) break;
		
		c = atoi(str);
		fscanf(fp, "%s", str);
		l = atoi(str);
		fscanf(fp, "%s", str);
		x = atoi(str);
		fscanf(fp, "%s", str);
		m = atoi(str);
		fscanf(fp, "%s", str);
		op = atoi(str);			// optimal policy value

		//printf("Read: c=%d\tl=%d\tx=%d\tm=%d\top=%d\t", c, l, x, m, op);
			
		
		if (m == 65535 && op == 65535) // to detect class change
		{
			mu[c-1][1][0][0] = x; // here the optimal policy value is x, not op
			//printf("mu[%d][%d][%d][%d] given value: %d\n", c-1,1,0,0, mu[c-1][1][0][0]);
		}
		else
		{
			mu[c-1][l][x][m] = op;
			//printf("mu[%d][%d][%d][%d] given value: %d\n", c-1,l,x,m, mu[c-1][l][x][m]);
		}
		

		

		
	}
	fclose(fp);
}

int main( int argc, char **argv )
{

	int i, j, l, R=0, L, zeta, cl;
	int verbose = false;
	int verboseEncoder = false;
	
	int seed;
	int seedStart;
	int seedStop;
	int seedPrint;
	long TriesPerSeed;

	int MaxPktsToSend;
	int MaxPktsPerRound;
	int RandomGenType;
	
	int symSize	= SYMBOL_SIZE;
   	int blockSize	= BLOCK_SIZE;

	char distFile[STRLEN];		// file containing the encoding distribution to use
	
	int optimal_policy = false;
	char optPolicyFile[STRLEN];	// file containing the optimal TX policy for redundancy

	double classes[20][4], val;
	double maxprop=0;
	int n_class=0, ret, ind_user=0, tot_user=0;
	char str[STRLEN];
	double **users;
	int *min;
	
	FILE	*fout, *ftopo;
	char	foutName[STRLEN];


// Types for random number generators defined in "distribution.h"
	enum {
            LINUX_TYPE  =   0,
            AL_TYPE     =   1,
            MSP_TYPE    =   2,
            TINYOS_TYPE =   3,
            MLCG_TYPE	=   4,
        };
// ---------------------------------------------
// Initialize variables
// ---------------------------------------------

	// p and R not an input any more
	if (argc<7) printUsage( argv[0] );

	printf("\tYou chose the following arguments:\n");
	printf("\t\tNumber of rounds (L): %s\n", argv[1]);
	printf("\t\tProbability distribution file: %s\n", argv[2]);
	printf("\t\tSimulation ID: %s\n", argv[3]);	
	printf("\t\tVerbosity: %s\n", argv[4]);
	printf("\t\tNumber of seeds to try: %s\n", argv[5]);
	printf("\t\tTopology file: %s\n", argv[6]);
	if (argc==8)   printf("\t\tOptimal policy file: %s\n", argv[7]);

	if (argc>8)	{
		perror("\nToo many arguments for my-decoder.\n\n");
		exit(-100);
	}
	
	// argv[1]=L argv[2]=distFile argv[3]=simID argv[4]=verbose argv[5]=#Seeds argv[6]=topofile argv[7]=optPolicyFile

	strcpy(foutName, argv[3]);
	strcat(foutName, ".log");
	fout = fopen(foutName, "a");
	
	L = atoi(argv[1]);		// max. # of TX rounds



	zeta 		= 5;			// Parameter used in the heuristic 
	RandomGenType	= LINUX_TYPE;		// Set random generator to use 
						// LINUX_TYPE -> drand48()
						// MSP_TYPE -> linear congruential

	TriesPerSeed	= 1;			// # of simulations per seed
	MaxPktsPerRound	= XIMAX;		// Max. redundancy that can be sent for each TX round
	MaxPktsToSend	= BLOCK_SIZE + L*MaxPktsPerRound; // Max. # of pkts we can send over all TX rounds

	strcpy(distFile, argv[2]);
	verbose		= atoi(argv[4]);
	seedStop	= atoi(argv[5]);

	fprintf(fout, "Max. rounds= %d\t", L);
	fprintf(fout, "distFile: %s\t", distFile);
	fprintf(fout, "simID: %s\n", argv[3]);
	fprintf(fout, "#Seeds: %d\t", seedStop);
	fprintf(fout, "Topo file: %s\t", argv[6]);
	
	// if only 4 arguments are given -> use heuristic TX policy 
	if (argc==8) 
	{
		strcpy(optPolicyFile, argv[7]);
		optimal_policy = true;

		fprintf(fout, "optPolicyFile: [%s]\n", optPolicyFile);
	}
	else
		fprintf(fout, "Heuristic based simulation\n");

	fprintf(fout, "Results written on file: %s%s\t\tLog on file: %s%s\n", argv[3], ".stat", argv[3], ".log");
	fprintf(fout, "-----------------------------------------------------------------------------------------\n\n");
	fclose(fout);
	
	
	// Allocate structure for the fountain based encoder
	Codec *coder   = new Codec( blockSize, symSize, RandomGenType, verboseEncoder, false);
	// Initialize encoder structures for the given distribution file
	coder->setDistribution(distFile);

	// Define data to Encode -> Transmit and, eventually, -> Decode
	uint8_t **data;
	data =  (uint8_t **)malloc(sizeof(uint8_t *)*blockSize);
	for (i=0; i<blockSize; i++) data[i] = (uint8_t *)malloc(sizeof(uint8_t)*symSize);
	
	// Initialize data structure
	for(int j=0; j<blockSize; j++)
	{
 		for(int i=0; i<symSize; i++) data[j][i] = j; 
	}

	// Initialize structure to store encoded packets
	uint8_t **encodedPkts;
	encodedPkts = (uint8_t **)malloc(sizeof(uint8_t *)*MaxPktsToSend);
	for (i=0; i<MaxPktsToSend; i++)
		encodedPkts[i] = (uint8_t *)malloc(sizeof(uint8_t)*symSize);
uid
	// Read topology file and store class information
	if ((ftopo = fopen(argv[6], "r")) == NULL )
	{
		perror("Topology file not found...\n\n");
		exit(-1);
	}

	while (1) {
		ret = fscanf(ftopo, "%s", str);   34
		if (ret==EOF) break;   val = atof(str);   classes[n_class][N_NODES] = val;
		
		ret = fscanf(ftopo, "%s", str);   if (ret==EOF) break;   val = atof(str);
		classes[n_class][DIST] = val;
		ret = fscanf(ftopo, "%s", str);   if (ret==EOF) break;   val = atof(str);
		classes[n_class][SNR] = val;
		ret = fscanf(ftopo, "%s", str);   if (ret==EOF) break;   val = atof(str);
		classes[n_class][PER] = val;
		
		n_class++;
	}
	
	if (verbose)
		for(i=0; i<n_class; i++)
			printf("Class: %d,\tR=%g,\td=%g,\tSNR=%g,\tPER=%g\n", i+1, classes[i][N_NODES], classes[i][DIST], classes[i][SNR], classes[i][PER]);
	
	// Compute maximum propagation time
	for(i=0; i<n_class; i++)
		maxprop = MAX(maxprop, classes[i][DIST]);
	maxprop /= UWSPEED;
	uiduid
	// Count users and generate user properties matrix 
	tot_user = 0;
	for (cl=0; cl < n_class; cl++)
		for(j=0; j < classes[cl][N_NODES]; j++)
			tot_user++;

	users = (double **) malloc (tot_user * sizeof(double*));
	for (i=0; i < tot_user; i++)
		users[i] = (double *) calloc(2, sizeof(double));
	
	for (cl=0; cl < n_class; cl++)
		for(j=0; j < classes[cl][N_NODES]; j++)	{
			users[ind_user][US_CLASS] = cl;
			users[ind_user][US_PER] = classes[cl][PER];
			ind_user++;
		}
	
	R = ind_user;

	// Allocate structures for fountain based decoders at uidall R receivers
	Codec **decoder;
	decoder = (Codec **)malloc(R*sizeof(Codec *));

	for (i=0; i<R; i++) {
		// Allocate structures for fountain based decouidder at all receivers
		decoder[i] = new Codec( blockSize, symSize, RandomGenType, verboseEncoder, false);

		// Initialize decoder structures for the given distribution file
	 	decoder[i]->setDistribution(distFile);
	}


	if (verbose)
		for(i=0; i < R; i++)
			printf("User %d ---\t class=%g, PER=%g\n", i+1, users[i][US_CLASS]+1, users[i][US_PER]);	
	
	// To keep track of pkts correctly RX by each receiver
	int *RXok;
	RXok = (int *)calloc(R, sizeof(int));
	
uid
	if (optimal_policy==true) 
	{
		xmin = (int *)malloc(sizeof(int)*(L+1));	// xmin[0], xmin[1], ..., xmin[L]
		xmax = (int *)malloc(sizeof(int)*(L+1));	// idem

		xmin[1]	= BLOCK_SIZE;
		xmax[1] = BLOCK_SIZE + XIMAX;
	 
		for (i=2; i<=L; i++) 
		{
			xmin[i] = 1;
			xmax[i] = xmin[i]+XIMAX-1;
		}

		// optimal mapping; mu[class][l][x][m]
		// class: node class (in the topology)
		// x: tot. # of pkts TX up to and including round l
		// m: min. # of pkts correctly RX among all users up to and including round l
		// l: current round number

		mu = (int****) malloc(sizeof(int***) * n_class);

		for (cl=0; cl < n_class; cl++)
			mu[cl] = (int***) malloc(sizeof(int**) * (L+1));

		for (cl=0; cl < n_class; cl++)
			for (i=0; i<=L; i++)
				mu[cl][i] = (int**) malloc(sizeof(int*) * (xxmax(L)+2));

		for (cl=0; cl < n_class; cl++)
			for (i=0; i<=L; i++)
				for (j=0; j<=xxmax(L)+1; j++)
					mu[cl][i][j] = (int*) calloc(xxmax(L)+2, sizeof(int)); // from 0 -> L

		read_Policy_File(optPolicyFile);
	}

// ---------------------------------------------
// Main simulation cycle
// ---------------------------------------------
	seedStart = 1;
	
	// PC: need to allocate vectors here...
	// PC: changed to report minimum number of packets received by all nodes IN A CLASS
	min	= (int *) calloc(n_class, sizeof(int));
	
	// Iterate for different seeds
	seedPrint = MAX((int)(seedStop*10/100),1);
	
	for (seed=seedStart; seed<=seedStop; seed++)
	{
		if (seed%seedPrint==0)
		{ 
			printf("%2.1f%%", ((double)seed/(double)seedStop)*100.);
			printf(" - Seed=%d\n", seed);
		}

		if (verbose) printf("\n [*]Seed=%d\n", seed);

		// for each seed iterate for a number of trials 
		for (i=1; i<=TriesPerSeed; i++) 
		{
			int  maxr, txpkts, lasttx;
			
			// Prepare Encoder
			coder->setCoder();
			for (j=0; j<R; j++)
			{
				// Prepare Decoders for RX max. redundancy of (MaxPktsToSend-blockSize)
				decoder[j]->setDecoder( (MaxPktsToSend-blockSize) );
				
				// Set Decoder @ receiver j for current seed
        		decoder[j]->startNewBlock( seed );
			}

			// Set Encoder with current seed
			coder->startNewBlock( seed );
			
			// Send data to Encoder
        	for(j=0; j<blockSize; j++)
        		coder->setToCoder( &data[j][0] ); // coder->setToCoder( data[j] );
			
			// Actual encoding: encode "MaxPktsToSend" pkts @ encoder
			for(j=1; j<=MaxPktsToSend; j++)
				coder->getCodedPck( encodedPkts[j-1], j );

			txpkts 	= 0;	// pointer to next pkt to TX

			// Counts down residual rank
			maxr = BLOCK_SIZE;
		
			// Iterate over TX rounds
			for (l=1; l<=L; l++)
			{
				int tmp=0;
				
				// lasttx: points to the first packet to send in this round
				lasttx = txpkts + 1;

				// txpkts: points to the last packet to send in this round
				// PC: changed so that Xi = max # of retransmissions among those required by the opt policy for each class
	 			if (optimal_policy==true)
				{
					if (l==1)
						for (i=0; i < n_class; i++) {
							tmp = MAX(tmp, mu[i][1][0][0]);
							if (verbose) printf("Round 1, class:%d, min[class]=%d, tmp:%d, mu[i][1][0][0]=%d.\n", i+1, min[i], tmp, mu[i][1][0][0]);
						}
					else 
						for (i=0; i < n_class; i++)	{
							tmp = MAX(tmp, mu[i][l][txpkts][min[i]]);
							if (verbose) printf("Round i>1, class:%d, min[class]=%d, tmp:%d, mu[i][l][txpkts][min[i]]=%d\n", i+1, min[i], tmp, mu[i][l][txpkts][min[i]]);
						}
				}
				else {
					tmp = maxr + zeta;
				}

				txpkts += tmp;

				if (verbose) printf("Round=%d, firsttx=%d, lasttx=%d, TXinRound=%d\n", l, lasttx, txpkts, tmp);

				maxr = 0;
				for (j=0; j<R; j++)
				{
					if (verbose) printf("Packet(s) ");
					for (tmp=lasttx; tmp<=txpkts; tmp++) 
					{
						double aa = myrand.Random();
						if(aa > users[j][US_PER])       // should be users[j][US_PER]
						{
							if (verbose) printf("%d, ", tmp-1);
                         	decoder[j]->setToDecoder( encodedPkts[tmp-1], tmp );
							RXok[j]++;
						}
					}
					if (verbose) printf("    OK at node %d\n", j);
					// Track maximum residual rank after this round l
					if (verbose) printf("Res rank at user %d: %d, maxr before user j: %d, maxr AFTER user j: %d.\n", j, decoder[j]->decode(), maxr, MAX(decoder[j]->decode(), maxr));
					maxr = MAX(decoder[j]->decode(), maxr);
				}
				// if here maxr==0 -> we successfully decode at all receivers
				if (maxr==0) {
					if (verbose) printf("Decoding OK @ all receivers\n");
					ntransmissions += txpkts;
					ntransmissions2 += (txpkts*txpkts);
					nrounds += l;
					delay += txpkts*TD + l*2*maxprop + (l-1)*(TA);
					
					break;
				}
				else {
					// calculate min. # of pkts correctly RX among all receivers
					// PC: MINIMUM IS class-dependent and must be initialized to some big value here
					for (cl=0; cl < n_class; cl++)
						min[cl] = MAXINT;
					// PC: Now the minimum can be calculated correctly
					if (verbose)
					{
						for (j=0; j<R; j++)
							printf("%d\t",j+1);
						printf("\n");
						for (j=0; j<R; j++)
							printf("%g\t",users[j][US_CLASS]+1);
						printf("\n");
						for (j=0; j<R; j++)
							printf("%d\t",RXok[j]);
						printf("\n");
					}
					
					for (j=0; j<R; j++) {
						//printf("Recalculating min. User: %d, class: %d, min of class: %d, RXok of this user: %d, new min of class: %d.\n", j+1, (int)users[j][US_CLASS], min[(int)users[j][US_CLASS]], RXok[j], MIN(min[(int)users[j][US_CLASS]], RXok[j]));
						min[(int)users[j][US_CLASS]] = MIN(min[(int)users[j][US_CLASS]], RXok[j]);
					}
				
					if (verbose) {	
						printf("Min correctly RX = ");
						for (cl=0; cl < n_class; cl++)
							printf("%d  ", min[cl]);
						printf("\n");
						if (l==L) printf("Failure for seed %d\n", seed);
					}
					
					if (l==L) { 
						nfailures++; 
						ntransmissions += txpkts; 
						ntransmissions2 += (txpkts*txpkts);
						nrounds += l;
						delay += txpkts*TD + l*2*maxprop + (l-1)*(TA);
					}
				}

			} // end for over l (TX rounds)

			// reset variables storing the # of correctly RXed packets
			for (j=0; j<R; j++)
				RXok[j] = 0;
			
			ntot++;
			
		} // end for over i (tries per seed)
	} // end for over seeds
	
	
	// -----------------------------------------------------------------------------------------------------
	//  compute statistics (ntot here is the number of trials done, both over diff seeds and sims per seed)
	// -----------------------------------------------------------------------------------------------------
	//  Columns:
	//  1. P[failure]
	//  2. # trasmissioni
	//  3. var # trasmissioni
	//  4. # rounds
	//  5. delay
	printf("\n dist=%.2f, nfailures=%ld, ntot=%ld, failureProb=%g\n", (double)classes[0][DIST], nfailures, ntot, (double)nfailures/(double)ntot);
	printf("averageTX=%g\t averageROUNDS=%g\n", (double)ntransmissions/(double)ntot, (double)nrounds/(double)ntot);
	printf("averageDELAY=%g [s]\n", delay/(double)ntot);

	strcpy(foutName, argv[3]);
	strcat(foutName, ".stat");
	fout = fopen(foutName, "a");
	fprintf(fout, "%.2f\t%g\t%g\t%g\t%g\t%g\n", (double)classes[0][DIST], (double)nfailures/(double)ntot, (double)ntransmissions/(double)ntot, sqrt((double)ntransmissions2/(double)ntot-pow((double)ntransmissions/(double)ntot, 2.)), (double)nrounds/(double)ntot, delay/(double)ntot);
	fclose(fout);

	// ----------------------------------------------------------------
	// free used memory
	// ----------------------------------------------------------------
	delete coder;
	for (j=0; j<R; j++) delete decoder[j];
	free(decoder);
	free(RXok);
	free(min);
	for (i=0; i<MaxPktsToSend; i++) free(encodedPkts[i]);
	free(encodedPkts);
	for (i=0; i<blockSize; i++) free(data[i]);
	free(data);
	for (i=0; i<tot_user; i++)
		free(users[i]);
	free(users);
	if (optimal_policy==true)
	{
		free(xmin);
		free(xmax);
		for (cl=0; cl<n_class; cl++)
			for (i=xxmin(1); i<=xxmax(L)+1; i++)
				for (j=0; j<=xxmax(L)+1; j++)
					free(mu[cl][i][j]); // PC: changed here
		free(mu);
	}
	// ----------------------------------------------------------------

	return 1;		// Program successfully terminated
};


void printUsage( char *name )
{
	printf("USAGE\n\n");
	printf("%s L distFile simID verbose #seeds topofile [optPolicyFile]\n\n", name);
	printf("L: number of rounds\n");
	printf("distFile: file containing the encoding distribution\n");
	printf("simID: simulation identifier\n");
	printf("#seeds: number of random seeds to simulate\n");
	printf("topofile: file contatining the topology description\n");
	printf("optPolicyFile: OPTIONAL, contains the optimal policy\n\n");
	
	exit(1);
};








