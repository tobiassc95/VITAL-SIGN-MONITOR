#include "dma.h"
#include "portpin.h"
#include "hardware.h"
#include "math.h"

#define NCHANNELS 16

static DMAMUX_Type* dmamux = DMAMUX;
static DMA_Type* dma = DMA0;
static void (*callback[NCHANNELS][2]) (void);

void DMAconfigure(DMA_CHANNEL ch, DMA_MODE mode, DMA_SOURCE source, uint32_t *saddr, uint16_t soff/*bytes*/, DMA_DATASIZE ssize, uint32_t slast/*bytes*/,
				uint32_t *daddr, uint16_t doff/*bytes*/, DMA_DATASIZE dsize, uint32_t dlast/*bytes*/, uint32_t nbytes, uint16_t citer) { //nbytes >= ssize!!
//	if(mode == DMA_PERIODIC) {
//		if(ch > DMA_CH3) //only the first 4 DMA channels have periodic triggering capability.
//			return;
//	}

	SIM->SCGC6 |= SIM_SCGC6_DMAMUX_MASK; //turn on CLOCK GATING.
	SIM->SCGC7 |= SIM_SCGC7_DMA_MASK; //turn on CLOCK GATING.

	dmamux->CHCFG[ch] = DMAMUX_CHCFG_TRIG(mode) | //Enables the periodic trigger capability for the triggered DMA channel. 0 -> Normal mode, 1 -> Periodic Trigger mode
						DMAMUX_CHCFG_SOURCE(source); //Specifies which DMA source is routed to a particular DMA channel. NOTE: Only the first 4 DMA channels have periodic triggering capability.

	//dma->CR = DMA_CR_EMLM(1);
	dma->CR = 0;
	dma->TCD[ch].SADDR = (uint32_t)saddr; //source address.
	//dma->TCD[ch].SOFF = (uint16_t)1<<ssize; //source offset (bytes).
	dma->TCD[ch].SOFF = soff; //source offset (bytes).
	dma->TCD[ch].ATTR = DMA_ATTR_SSIZE(ssize) | DMA_ATTR_DSIZE(dsize); //source and destination data size.
	dma->TCD[ch].NBYTES_MLNO = nbytes; //number of bytes to transfer within a minor loop///
	dma->TCD[ch].SLAST = -slast; //source offset to beginning (therefore negative).
	dma->TCD[ch].DADDR = (uint32_t)daddr; //destination address.
	//dma->TCD[ch].DOFF = (uint16_t)1<<dsize; //destination offset.
	dma->TCD[ch].DOFF = doff; //destination offset.
	dma->TCD[ch].CITER_ELINKNO = citer; //number of major iterations. CITER=BITER in the beginning.
	//dma->TCD[ch].DLAST_SGA = -nbytes; //destination offset to beginning (therefore negative).
	dma->TCD[ch].DLAST_SGA = -dlast; //destination offset to beginning (therefore negative).
	dma->TCD[ch].BITER_ELINKNO = citer; //number of major iterations. CITER=BITER in the beginning.

	///DMA_CDNE -> TCDn_CSR[DONE]
}

void DMAenable(DMA_CHANNEL ch) {
	dmamux->CHCFG[ch] |= DMAMUX_CHCFG_ENBL_MASK; //Enables the DMA channel. Must be disabled during configuration of the DMAMux.
	dma->ERQ |= (uint16_t)(1<<ch); //Enables or disables the request signal for each channel.
	//dma->SERQ = ch; //Sets a given bit in the ERQ to enable the DMA request for a given channel.
}

void DMAenableIRQ(DMA_CHANNEL ch, DMA_INTERRUPT intrpt, void (*majorfunc) (void), void (*halffunc) (void)) {
	dma->TCD[ch].CSR = intrpt<<1; //enable interruptions.

	if(majorfunc != NULL)
		callback[ch][0] = majorfunc;///
	if(halffunc != NULL)
		callback[ch][1] = halffunc;///

	NVIC_EnableIRQ(DMA0_IRQn + ch);
}

void DMAdisable(DMA_CHANNEL ch) {
	dmamux->CHCFG[ch] &= ~DMAMUX_CHCFG_ENBL_MASK; //Enables the DMA channel. Must be disabled during configuration of the DMAMux.
	dma->ERQ &= ~(uint16_t)(1<<ch); //Enables or disables the request signal for each channel.///DMA_CERQ
	//dma->CERQ = ch; //Clears a given bit in the ERQ to disable the DMA request for a given channel.
	//DMAdisableIRQ(ch);
}

void DMAdisableIRQ(DMA_CHANNEL ch) {
	NVIC_DisableIRQ(DMA0_IRQn + ch);
}

void DMAstart(DMA_CHANNEL ch) {
	dma->TCD[ch].CSR |= DMA_CSR_START_MASK; //The channel is explicitly started via a software initiated service request. The eDMA hardware automatically clears this flag after the channel begins execution.
}

void DMAcancel(void) {
	dma->CR |= DMA_CR_CX_MASK; //Cancel the remaining data transfer. Stop the executing channel and force the minor loop to finish.
}

bool DMAdone(DMA_CHANNEL ch) {
	return dma->TCD[ch].CSR & DMA_CSR_DONE_MASK; //This flag indicates the eDMA has completed the major loop. The eDMA engine sets it as the CITER count reaches zero.
}

void DMA0_IRQHandler(void) {///
	dma->INT = DMA_INT_INT0_MASK; //Clear interrupt flag. ///DMA_CINT
	if((dma->TCD[0].CSR & DMA_CSR_DONE_MASK) && (dma->TCD[0].CSR & DMA_CSR_INTMAJOR_MASK)) //majorloop completed.
		callback[0][0](); //DMA_INTMAJOR.
	if((dma->TCD[0].CSR & DMA_CSR_DONE_MASK) == 0 && (dma->TCD[0].CSR & DMA_CSR_INTHALF_MASK)) //majorloop half completed.
		callback[0][1](); //DMA_INTHALF.

	dma->TCD[0].CSR = (dma->TCD[0].CSR & ~DMA_CSR_DONE_MASK) | DMA_CSR_DONE(0);  			// Clear Channel Done flag
}

void DMA1_IRQHandler(void) {///
	dma->INT = DMA_INT_INT1_MASK; //Clear interrupt flag. ///DMA_CINT
	if((dma->TCD[1].CSR & DMA_CSR_DONE_MASK) != 0 && (dma->TCD[1].CSR & DMA_CSR_INTMAJOR_MASK) != 0) //majorloop completed.
		callback[1][0]();
	if((dma->TCD[1].CSR & DMA_CSR_DONE_MASK) == 0 && (dma->TCD[1].CSR & DMA_CSR_INTHALF_MASK) != 0) //majorloop half completed.
		callback[1][1]();
}

void DMA2_IRQHandler(void) {///
	dma->INT = DMA_INT_INT2_MASK; //Clear interrupt flag. ///DMA_CINT
	if((dma->TCD[2].CSR & DMA_CSR_DONE_MASK) != 0 && (dma->TCD[2].CSR & DMA_CSR_INTMAJOR_MASK) != 0) //majorloop completed.
		callback[2][0]();
	if((dma->TCD[2].CSR & DMA_CSR_DONE_MASK) == 0 && (dma->TCD[2].CSR & DMA_CSR_INTHALF_MASK) != 0) //majorloop half completed.
		callback[2][1]();
}
