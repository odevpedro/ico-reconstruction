	.text
	.p2align 3
	.globl	boyAI_sub_157EA0
	.ent	boyAI_sub_157EA0
boyAI_sub_157EA0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,0x20
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x20
	addiu	$29,$29,-0x50
	sw	$4,0($29)
	sd	$31,0x40($29)
	lui	$4,0x56
	sd	$17,0x30($29)
	addiu	$4,$4,-0x7710
	lw	$2,0($29)
	sd	$16,0x20($29)
	jal	0x001a6e28
	lw	$16,0x164($2)
	lui	$2,0x16
	lw	$4,0($29)
	addiu	$2,$2,-0xbb0
	lw	$3,-0x6e0c($28)
	bne	$4,$3,loc_00157f00
	sw	$2,0x18($16)
	lui	$1,0x447a
	mtc1	$1,$f12
	jal	0x00181f38
	nop	
loc_00157f00:
	lw	$3,0($29)
	lw	$2,-0x6e08($28)
	bne	$3,$2,loc_00157f44
	nop	
	lw	$2,0($29)
	addiu	$16,$29,0x10
	lui	$1,0xbf80
	mtc1	$1,$f12
	move	$4,$16
	lw	$5,0x164($2)
	jal	0x00243b18
	addiu	$5,$5,0x1b0
	lw	$4,0($29)
	jal	0x00106ab8
	move	$5,$16
	b	loc_00157f58
	nop	
loc_00157f44:
	lw	$2,0($29)
	lw	$4,0($29)
	lw	$5,0x164($2)
	jal	0x00106ab8
	addiu	$5,$5,0x1b0
loc_00157f58:
	lw	$4,0($29)
	addiu	$17,$0,4
	jal	0x00157e68
	addiu	$16,$0,1
	lw	$2,0($29)
	lw	$3,0xc($2)
	beq	$3,$17,loc_00157f84
	addiu	$5,$0,0xb7
	lw	$2,0($29)
	lw	$3,0x164($2)
	sb	$16,0x1ca($3)
loc_00157f84:
	lw	$4,0($29)
	jal	0x0015bcc8
	nop	
	jal	0x00203aa0
	addiu	$4,$0,1
	.set	macro
	.set	reorder
	.end	boyAI_sub_157EA0
