	.text
	.p2align 3
	.globl	boyAI_sub_14BBCC
	.ent	boyAI_sub_14BBCC
boyAI_sub_14BBCC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	nop	
	lui	$1,0x3f80
	mtc1	$1,$f12
	jal	0x0010a4e0
	.set	macro
	.set	reorder
	.end	boyAI_sub_14BBCC
