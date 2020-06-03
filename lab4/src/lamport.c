#include "banking.h"

static timestamp_t lamportTime = 0;

timestamp_t get_lamport_time(){
	return lamportTime;
}

void IncrementLamportTime(timestamp_t other){
	lamportTime = (other > lamportTime) ? other : lamportTime;
	++lamportTime;
}
