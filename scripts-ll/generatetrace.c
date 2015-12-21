#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main(int argc, const char* argv[]){

long STEP_SIM; // the random range used to add each read address randomly.

if(argc>1){
	printf("#step set to 1024*%d\n",atoi(argv[1]));
	STEP_SIM=1024L*atoi(argv[1]);// the random range used to add each read address randomly.
}else{
	printf("#step set to 1024*8\n");
	STEP_SIM=1024L*8;// the random range used to add each read address randomly.
}

const int INST_SIM=10;//the random range of inst per line of the trace file.

const int WRITE_PROB=50;//the probababilty of writing occurrance: 1/WRITE_PROB

const  long capacity=2147483648L;//the capacity of the DRAM

//const long STEP_SIM=1024L*8;// the random range used to add each read address randomly.

int inst;

long addrR=0,addrW=0;

time_t t;

srand( time(&t) ); 
//simulate scanning all the memory for the DRAM
while(addrR<capacity){

	//randomly decide the inst
//	srand(time(NULL));
	inst=rand()%INST_SIM;
	printf("%d ", inst);

	//print read address.
	printf("%ld ", addrR);

	//randomly decide whether to have a write.
//	srand(time(NULL));
	if((rand()%WRITE_PROB)<=1){
		//decide the writing address according to the read address;
		//currently the 1M ahead of the writing address;
		addrW=(abs(addrR-1024*1024))%capacity; 
		printf("%ld", addrW);
	}
	printf("\n");//finished one line for the trace.

	//determine the next read address:
	addrR+=rand()%STEP_SIM;
 }
}
