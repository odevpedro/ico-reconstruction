	.text
	.p2align 3
	.globl	boyAI_sub_14C05C
	.ent	boyAI_sub_14C05C
boyAI_sub_14C05C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	negu	$2,$2
	mtc1	$2,$f0
	cvt.s.w	$f0,$f0
	.word	0x46140034
	nop	
	.word	0x4501000c
	addiu	$2,$0,1
	.word	0x1000000a
	move	$2,$0
	.set	macro
	.set	reorder
	.end	boyAI_sub_14C05C
