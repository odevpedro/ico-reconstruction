	.text
	.p2align 3
	.globl	boyAI_sub_156468
	.ent	boyAI_sub_156468
boyAI_sub_156468:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	ld	$18,0x50($29)
	ld	$17,0x40($29)
	ld	$16,0x30($29)
	jr	$31
	addiu	$29,$29,0xd0
	nop	
	j	0x201dc8
	nop	
	move	$3,$4
	lw	$4,0x164($3)
	lw	$2,0x2c($4)
	beql	$2,$0,loc_001564a0
	lw	$5,0x5d0($4)
	lw	$5,0($2)
loc_001564a0:
	lw	$2,-0x6e0c($28)
	bne	$3,$2,loc_001564c4
	nop	
	lw	$4,0x140($4)
	beqz	$4,loc_001564c4
	nop	
	lw	$2,0x15c($4)
	j	0x1f3388
	sw	$5,0x5f4($2)
loc_001564c4:
	jr	$31
	nop	
	nop	
	addiu	$29,$29,-0x30
	sd	$17,0x10($29)
	sd	$31,0x20($29)
	move	$17,$4
	sd	$16,0($29)
	lw	$16,0x164($17)
	lw	$4,0x140($16)
	beqz	$4,loc_00156504
	lw	$3,-0x6f60($28)
	jal	0x001f2500
	nop	
	sw	$0,0x140($16)
	lw	$3,-0x6f60($28)
loc_00156504:
	addiu	$2,$0,0x53
	beq	$3,$2,loc_00156518
	lw	$2,-0x5b60($28)
	.word	0x1040000c
	ld	$31,0x20($29)
loc_00156518:
	jal	0x0013eb50
	addiu	$4,$0,0x35
	.word	0x10400008
	.set	macro
	.set	reorder
	.end	boyAI_sub_156468
