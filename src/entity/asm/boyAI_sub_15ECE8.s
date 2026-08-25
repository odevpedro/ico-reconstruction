	.text
	.p2align 3
	.globl	boyAI_sub_15ECE8
	.ent	boyAI_sub_15ECE8
boyAI_sub_15ECE8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	nop	
	lui	$1,0x4348
	mtc1	$1,$f12
	move	$7,$0
	addiu	$8,$0,0xff
	move	$4,$2
	move	$5,$16
	jal	0x001873a8
	addiu	$6,$0,0xff
	lw	$4,0($29)
	jal	0x00106ab8
	move	$5,$16
	div	$0,$17,$19
	beql	$19,$0,loc_0015ed24
	break	0,7
loc_0015ed24:
	mfhi	$2
	bnez	$2,loc_0015ed50
	nop	
	lwc1	$f0,0x33c($21)
	.word	0x4600b034
	nop	
	bc1t	loc_0015ed50
	addiu	$5,$0,0x13a
	lw	$4,0($29)
	jal	0x0015bcc8
	nop	
loc_0015ed50:
	jal	0x00203aa0
	addiu	$4,$0,1
	.word	0x1000ffd1
	move	$4,$16
	addiu	$29,$29,-0x30
	sw	$4,0($29)
	sd	$31,0x20($29)
	addiu	$4,$0,6
	sd	$16,0x10($29)
	lw	$2,0($29)
	lw	$3,0x164($2)
	jal	0x00203aa0
	lw	$16,0x5ec($3)
	move	$4,$16
	jal	0x001c0870
	move	$5,$0
	lui	$4,0x63
	jal	0x001a6e28
	addiu	$4,$4,0x2328
	jal	0x00203aa0
	move	$4,$0
	ld	$31,0x20($29)
	ld	$16,0x10($29)
	jr	$31
	.set	macro
	.set	reorder
	.end	boyAI_sub_15ECE8
