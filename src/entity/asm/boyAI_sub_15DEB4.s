	.text
	.p2align 3
	.globl	boyAI_sub_15DEB4
	.ent	boyAI_sub_15DEB4
boyAI_sub_15DEB4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sw	$4,0($29)
	lui	$4,0x56
	sd	$31,0x10($29)
	jal	0x001a6e28
	addiu	$4,$4,-0x7728
	lw	$2,0($29)
	lw	$4,0($29)
	lw	$5,0x164($2)
	jal	0x00106ab8
	addiu	$5,$5,0x1b0
	lw	$4,0($29)
	jal	0x00157dc8
	nop	
	lw	$4,0($29)
	addiu	$5,$0,3
	lw	$2,0x164($4)
	lw	$3,0x670($2)
	lw	$4,0x1dc($3)
	bne	$4,$5,loc_0015df10
	nop	
	jal	0x00203aa0
	addiu	$4,$0,0x168
	nop	
loc_0015df10:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xb7
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0015df10
	nop	
	nop	
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	lui	$4,0x56
	sd	$31,0x10($29)
	jal	0x001a6e28
	addiu	$4,$4,-0x7460
	jal	0x00203aa0
	move	$4,$0
	ld	$31,0x10($29)
	jr	$31
	addiu	$29,$29,0x20
	nop	
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	lui	$4,0x56
	sd	$31,0x10($29)
	jal	0x001a6e28
	addiu	$4,$4,-0x7450
loc_0015df78:
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0015df78
	nop	
	addiu	$29,$29,-0x50
	lui	$3,0x16
	sw	$4,0($29)
	addiu	$3,$3,-0x940
	sd	$16,0x20($29)
	addiu	$4,$29,0x10
	sd	$31,0x40($29)
	sd	$17,0x30($29)
	lw	$5,0($29)
	lw	$16,0($29)
	lw	$2,0x164($5)
	lw	$5,0x164($16)
	lui	$1,0xbf80
	mtc1	$1,$f12
	lw	$17,0x5ec($2)
	addiu	$5,$5,0x4a0
	.set	macro
	.set	reorder
	.end	boyAI_sub_15DEB4
