	.text
	.p2align 3
	.globl	boyAI_sub_1518E8
	.ent	boyAI_sub_1518E8
boyAI_sub_1518E8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$4,4($21)
	addiu	$5,$0,0x64
	mult	$2,$2,$3
	lui	$1,0x42a0
	mtc1	$1,$f2
	beql	$4,$0,loc_00151904
	break	0,7
loc_00151904:
	mov.s	$f1,$f0
	.word	0x46011034
	lw	$3,0x48($20)
	subu	$2,$16,$2
	div	$0,$2,$4
	mflo	$2
	mult	$2,$2,$5
	div	$0,$2,$16
	mflo	$2
	slt	$2,$2,$3
	.word	0x10400025
	nop	
	lui	$1,0x4120
	mtc1	$1,$f3
	.word	0x45000021
	nop	
	sub.s	$f1,$f1,$f2
	mtc1	$0,$f0
	lui	$1,0x3f80
	mtc1	$1,$f2
	nop	
	nop	
	div.s	$f1,$f1,$f3
	.word	0x46000834
	nop	
	bc1t	loc_00151984
	mov.s	$f12,$f0
	.word	0x46011034
	nop	
	bc1f	loc_00151984
	mov.s	$f12,$f1
	mov.s	$f12,$f2
loc_00151984:
	jal	0x00263fb0
	nop	
	lui	$1,0x56
	ld	$5,-0x79e0($1)
	jal	0x00262be8
	move	$4,$2
	lui	$1,0x56
	ld	$4,-0x79d8($1)
	jal	0x00262b80
	move	$5,$2
	jal	0x002633b8
	.set	macro
	.set	reorder
	.end	boyAI_sub_1518E8
