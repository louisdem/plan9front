#define NPRIVATES	16

GLOBL	_tos(SB), $8
GLOBL	_errnoloc(SB), $8
GLOBL	_privates(SB), $8
GLOBL	_nprivates(SB), $8

TEXT	_main(SB), 1, $(32+NPRIVATES*8)

	/* _tos = arg */
	MOVQ	AX, _tos(SB)
	LEAQ	24(SP), AX
	MOVQ	AX, _errnoloc(SB)
	LEAQ	32(SP), AX
	MOVQ	AX, _privates(SB)
	MOVQ	$NPRIVATES, _nprivates(SB)
	CALL	_envsetup(SB)
	MOVL	inargc-8(FP), RARG
	LEAQ	inargv+0(FP), AX
	MOVQ	AX, 8(SP)
	MOVQ	environ(SB), AX
	MOVQ	AX, 16(SP)
	CALL	main(SB)
	MOVQ	AX, RARG
	CALL	exit(SB)
	RET
