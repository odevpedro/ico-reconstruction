	.text
	.p2align 3
	.globl	boyAI_sub_15B7D4
	.ent	boyAI_sub_15B7D4
boyAI_sub_15B7D4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	mtc1	$1,$f1
	.word	0x00862018
	lw	$5,4($2)
	mul.s	$f1,$f20,$f1
	beql	$5,$0,loc_0015b7ec
	break	0,7
loc_0015b7ec:
	lwc1	$f0,0($17)
	subu	$3,$3,$4
	div	$0,$3,$5
	.word	0x46020032
	mflo	$3
	mtc1	$3,$f0
	cvt.s.w	$f0,$f0
	nop	
	nop	
	div.s	$f20,$f1,$f0
	bc1f	loc_0015b824
	lw	$2,0x15c($16)
	lwc1	$f0,4($17)
	.word	0x46020032
loc_0015b824:
	addiu	$5,$0,0x190
	lui	$4,0x56
	lw	$3,0x4a0($2)
	addiu	$4,$4,0x5060
	mult	$3,$3,$5
	addu	$3,$3,$4
	lw	$2,0x188($3)
	srl	$2,$2,9
	andi	$2,$2,1
	.word	0x5040000a
	ld	$2,0x20($7)
	lw	$2,0x678($7)
	addiu	$4,$0,0x1a
	dsll32	$4,$4,0
	ori	$4,$4,5
	ld	$3,0x830($2)
	.word	0x14640003
	ld	$2,0x20($7)
	lui	$1,0x41f0
	.set	macro
	.set	reorder
	.end	boyAI_sub_15B7D4
