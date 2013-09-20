TEXT	_mulv(SB), $0
	MOVL	r+0(FP), CX
	MOVL	a+4(FP), AX
	MULL	b+12(FP)
	MOVL	AX, 0(CX)
	MOVL	DX, BX
	MOVL	a+4(FP), AX
	MULL	b+16(FP)
	ADDL	AX, BX
	MOVL	a+8(FP), AX
	MULL	b+12(FP)
	ADDL	AX, BX
	MOVL	BX, 4(CX)
	RET

TEXT	_mul64by32(SB), $0
	MOVL	r+0(FP), CX
	MOVL	a+4(FP), AX
	MULL	b+12(FP)
	MOVL	AX, 0(CX)
	MOVL	DX, BX
	MOVL	a+8(FP), AX
	MULL	b+12(FP)
	ADDL	AX, BX
	MOVL	BX, 4(CX)
	RET

TEXT	_div64by32(SB), $0
	MOVL	r+12(FP), CX
	MOVL	a+0(FP), AX
	MOVL	a+4(FP), DX
	DIVL	b+8(FP)
	MOVL	DX, 0(CX)
	RET

TEXT	_addv(SB),1,$0	/* used in profiler, can't be profiled */
	MOVL	r+0(FP), CX
	MOVL	a+4(FP), AX
	MOVL	a+8(FP), BX
	ADDL	b+12(FP), AX
	ADCL	b+16(FP), BX
	MOVL	AX, 0(CX)
	MOVL	BX, 4(CX)
	RET

TEXT	_subv(SB),1,$0		/* used in profiler, can't be profiled */
	MOVL	r+0(FP), CX
	MOVL	a+4(FP), AX
	MOVL	a+8(FP), BX
	SUBL	b+12(FP), AX
	SBBL	b+16(FP), BX
	MOVL	AX, 0(CX)
	MOVL	BX, 4(CX)
	RET
