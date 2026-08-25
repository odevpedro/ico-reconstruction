	.text
	.p2align 3
	.globl	boyAI_sub_15122C
	.ent	boyAI_sub_15122C
boyAI_sub_15122C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	move	$19,$3
	addu	$2,$21,$19
	lw	$3,0($2)
	.word	0x1464ffcd
	move	$5,$21
	lw	$4,0($29)
	jal	0x0014b048
	nop	
	bnez	$2,loc_00151294
	nop	
	lui	$1,0x447a
	mtc1	$1,$f0
	.word	0x4600a034
	bc1f	loc_00151274
	nop	
	lw	$4,0($29)
	jal	0x0014b228
	addiu	$5,$0,2
loc_00151274:
	lui	$1,0x4396
	mtc1	$1,$f0
	.word	0x4600a034
	bc1f	loc_00151294
	nop	
	lw	$4,0($29)
	jal	0x0014b228
	addiu	$5,$0,3
loc_00151294:
	lui	$1,0x447a
	mtc1	$1,$f0
	.word	0x4600a034
	bc1f	loc_001512bc
	nop	
	lw	$4,0($29)
	mov.s	$f12,$f20
	addiu	$5,$0,0xf
	jal	0x0014b270
	move	$6,$0
loc_001512bc:
	lw	$4,0($29)
	jal	0x0014b248
	addiu	$5,$0,3
	bnez	$2,loc_001512e4
	nop	
	lw	$4,0($29)
	jal	0x0014b248
	addiu	$5,$0,2
	.word	0x10400011
	nop	
loc_001512e4:
	lw	$4,0($29)
	jal	0x0014b248
	addiu	$5,$0,0x14
	.word	0x10400004
	nop	
	.set	macro
	.set	reorder
	.end	boyAI_sub_15122C
