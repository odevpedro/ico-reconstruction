	.text
	.p2align 3
	.globl	boyAI_sub_15697C
	.ent	boyAI_sub_15697C
boyAI_sub_15697C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$21,$29,0x40
	jal	0x00190638
	lw	$4,0x180($17)
	jal	0x0018ecc8
	lw	$4,0x180($17)
	jal	0x00190620
	lw	$4,0x180($17)
	lw	$4,0($29)
	jal	0x00156688
	lw	$5,0x180($17)
	move	$3,$2
	addiu	$2,$0,1
	beq	$3,$2,loc_001569c4
	addiu	$2,$0,2
	beq	$3,$2,loc_001569d8
	nop	
	b	loc_001569e4
	nop	
loc_001569c4:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x9c
	b	loc_001569e4
	nop	
loc_001569d8:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x9b
loc_001569e4:
	lw	$16,0($29)
	addiu	$4,$0,0x190
	lw	$3,0x15c($16)
	lw	$2,0x4a0($3)
	.set	macro
	.set	reorder
	.end	boyAI_sub_15697C
