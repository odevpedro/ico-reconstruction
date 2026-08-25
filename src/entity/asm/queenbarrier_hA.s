	.text
	.p2align 3
	.globl	queenbarrier_hA
	.ent	queenbarrier_hA
queenbarrier_hA:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$2,0x15c($4)
	lw	$2,0x800($2)
	lb	$3,0x12($2)
	beqz	$3,loc_0019ae90
	nop	
	lwc1	$f12,0x18($2)
	cvt.s.w	$f12,$f12
	lui	$1,0x40a0
	mtc1	$1,$f0
	lui	$1,0x3f80
	mtc1	$1,$f1
	nop	
	nop	
	div.s	$f12,$f12,$f0
	j	0x19c280
	sub.s	$f12,$f1,$f12
loc_0019ae90:
	jr	$31
	nop	
	.set	macro
	.set	reorder
	.end	queenbarrier_hA
