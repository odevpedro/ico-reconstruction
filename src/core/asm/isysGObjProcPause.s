	.text
	.p2align 3
	.globl	isysGObjProcPause
	.ent	isysGObjProcPause
isysGObjProcPause:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	jr	$31
	.set	macro
	.set	reorder
	.end	isysGObjProcPause
