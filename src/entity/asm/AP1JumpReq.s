	.text
	.p2align 3
	.globl	AP1JumpReq
	.ent	AP1JumpReq
AP1JumpReq:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lui       $2, 0x4b
	addiu     $5, $0, -0x2
	addiu     $3, $2, 0x3d10
	addiu     $4, $0, 0xb5
	.word	0xDC620000
	addiu     $4, $4, -0x1
	and       $2, $2, $5
	.word	0xFC620000
	addiu     $3, $3, 0x40
	.word	0x0481FFFA
	nop
	.word	0x03E00008
	nop
	.set	macro
	.set	reorder
	.set	at
	.end	AP1JumpReq
