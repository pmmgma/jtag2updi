#include <avr/io.h>	

char debug_txcnt=0x7F;
char debug_rxcnt=0x7F;

#define DEBUG_TXCNT GPIOR0
#define DEBUG_TX    GPIOR1

#define DEBUG_RXCNT GPIOR2
#define DEBUG_RX    GPIOR3


void debug_init() {
	DEBUG_RXCNT=0x7f;
	DEBUG_TXCNT=0x7f;
}

void debug_SendChar(char ch) {
    unsigned long laux;

	DEBUG_TX=ch;
	DEBUG_TXCNT=debug_txcnt=(debug_txcnt+1) | 0x80;	
	
    for (laux=0; laux<540; laux++) asm("nop");       	
}

char debug_RecvRdy() {
	char rxcnt=DEBUG_RXCNT;
	if (!(rxcnt & 0x80)) {
		debug_rxcnt=rxcnt;		
		return 0;
	}
	if (rxcnt == debug_rxcnt) return 0;
	return 1;
}

char debug_GetChar() {
	while (!debug_RecvRdy());
	debug_rxcnt=DEBUG_RXCNT;
	return DEBUG_RX;
}	
	
	
//*******************************************************************************
// Entry procedure
//*******************************************************************************
int main(void){	
	debug_init();
	for (;;) {
		if (debug_RecvRdy()) {
			debug_SendChar(debug_GetChar());
		}
	}
	
	return 0;
}