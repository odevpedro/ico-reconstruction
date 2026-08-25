	.text
	.p2align 3
	.globl	boyAI_sub_158124
	.ent	boyAI_sub_158124
boyAI_sub_158124:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$5,0x164($3)
	jal	0x001ce8f0
	addiu	$5,$5,0x1b0
	lw	$4,0($29)
	jal	0x001ce7c8
	nop	
	lw	$4,0($29)
	jal	0x00165270
	nop	
	lw	$4,0($29)
	jal	0x00144928
	nop	
	lui	$5,0x27
	lw	$2,0x4ec0($21)
	addiu	$5,$5,0x4ec0
	addiu	$4,$0,6
	lw	$3,4($5)
	addiu	$5,$0,0xa
	mult	$2,$2,$5
	beql	$3,$0,loc_00158178
	break	0,7
loc_00158178:
	addiu	$5,$0,0x3c
	subu	$2,$5,$2
	div	$0,$2,$3
	mflo	$2
	.word	0x00442018
	mtc1	$4,$f0
	cvt.s.w	$f0,$f0
	.word	0x4600a034
	nop	
	.word	0x4500003c
	addiu	$20,$23,1
	lw	$2,0x4ec0($21)
	addiu	$17,$0,0xa
	addiu	$16,$21,0x4ec0
	addiu	$18,$0,0x3c
	mult	$2,$2,$17
	lw	$3,4($16)
	addiu	$19,$0,6
	add.s	$f1,$f20,$f20
	lwc1	$f2,-0x7ca8($28)
	beql	$3,$0,loc_001581d0
	break	0,7
loc_001581d0:
	subu	$2,$18,$2
	div	$0,$2,$3
	mflo	$2
	mult	$2,$2,$19
	.set	macro
	.set	reorder
	.end	boyAI_sub_158124
