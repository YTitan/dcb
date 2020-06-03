#include "banking.h"
#include "ipc.h"
#include "ipcHandler.h"

#include <string.h>
#include <stdio.h>

void transfer(void* parent_data, local_id src, local_id dst, balance_t amount){
	// create transferOrder struct
	TransferOrder transfer;
	transfer.s_src = src;
	transfer.s_dst = dst;
	transfer.s_amount = amount;

	// create message struct
	Message m;
	m.s_header.s_magic = MESSAGE_MAGIC;
	m.s_header.s_payload_len = sizeof(TransferOrder);
	m.s_header.s_type = TRANSFER;
	m.s_header.s_local_time = get_physical_time();
	memcpy(m.s_payload, &transfer, sizeof(transfer));

	// send message to source
	IPCHelper* ipc = (IPCHelper*) parent_data;
	send(ipc, src, &m);

	// wait for ack message from dst
	do {
		receive(ipc, dst, &m);
	} while (m.s_header.s_type != ACK);
	
}
