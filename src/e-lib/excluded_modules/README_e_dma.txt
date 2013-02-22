These dma utilities include four user-functions.

void edma_init(void)
	Must be called once before any of the other dma utilities are used.

int edma_xfer(unsigned chan, void* dst, void* src, size_t size, IsrFxn usr_isr)
	This function will schedule a dma transfer to be performed by DMA channel "chan".
	It will copy "size" bytes from "src" to "dest".  The user may specify an
	interrupt service routine (usr_isr) to be called when this transfer completes,
	but this is often unneccessary, as the edma_busy() function can be used instead.
	The application should pass NULL as the usr_isr argument if no user ISR is needed.
	The function returns a transaction id which can be passed to edma_busy() which
	will return 0 when the specified transaction is finished.  There is no need to
	wait for a previous DMA to complete before invoking edma_xfer() again.  If the
	channel is busy, the function will queue up the DMA.  Queued  DMA's wil be
	serviced when the current DMA completes, in the order in which they were queued.
	If the ISR detects multiple DMA requests on its pending queue, it will convert
	them into a chained DMA request.  No interrupts will be generated except for the
	last DMA in the chain.  This eliminates the overhead of an interrupt which would
	otherwise manually chain the requests.  If the application has specified a usr_isr,
	automatic chaining will be avoided, thus ensuring that the interrupt is triggered
	so that the user ISR will be called.

	Note that as currently implemented, if the size of the transfer is less than
	approximately 1700 bytes, memcpy() will be a faster option.  Future optimizations
	are expected to reduce this figure.

int edma_xfer_tcb(unsigned chan, Ep_TCB* usr_tcb, IsrFxn usr_isr)
	This function is similar to edma_xfer() except that instead of taking dst, src,
	and size arguments, it takes a pointer to a pre-built TCB of type Ep_TCB.  This
	function should be used for 2D DMA's, or DMA's with non-unity strides.  The user's
	TCB need not be located in internal memory and has no alignment restrictions.
	It will be copied into a properly located and aligned TCB before use.

int edma_busy(int tranaction_id)
	This function will return a non-zero value if the transaction identified by
	transaction_id has not yet completed.  If it has completed, it will return 0.
	The transaction_id is returned by edma_xfer() or edma_xfer_tcb().
