	.text
	.p2align 3
	.globl	girlBrain_sub_1759A8
	.ent	girlBrain_sub_1759A8
girlBrain_sub_1759A8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	nop	
	nop	
	addiu	$29,$29,-0x50
	sw	$4,0($29)
	sd	$16,0x20($29)
	lui	$4,0x56
	sd	$31,0x40($29)
	addiu	$4,$4,-0x6ba8
	lw	$2,0($29)
	sd	$17,0x30($29)
	jal	0x001a6e28
	lw	$17,0x164($2)
	addiu	$16,$29,0x10
	lui	$1,0xbf80
	mtc1	$1,$f12
	move	$4,$16
	jal	0x00243b18
	addiu	$5,$17,0x4a0
	lw	$4,0($29)
	jal	0x00106ab8
	move	$5,$16
	lw	$4,0($29)
	jal	0x00104360
	addiu	$5,$17,0x580
	lui	$2,0x17
	sw	$0,0xd0($17)
	addiu	$2,$2,0x5a58
	sw	$2,0x14($17)
loc_00175a18:
	jal	0x00203aa0
	addiu	$4,$0,1
	lw	$2,0xd0($17)
	andi	$2,$2,0x10
	beqz	$2,loc_00175a18
	lui	$4,0x56
	jal	0x001a6e28
	addiu	$4,$4,-0x6b90
loc_00175a38:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x59
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_00175a38
	nop	
	nop	
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	lui	$4,0x56
	sd	$31,0x10($29)
	jal	0x001a6e28
	addiu	$4,$4,-0x6bc0
	lw	$6,-0x6714($28)
	addiu	$5,$0,0x59
	jal	0x0013ff88
	lw	$4,-0x6e0c($28)
	jal	0x0014a3a8
	nop	
	ld	$31,0x10($29)
	jr	$31
	addiu	$29,$29,0x20
	nop	
	addiu	$29,$29,-0x30
	sw	$4,0($29)
	sd	$31,0x20($29)
	lui	$4,0x56
	lw	$2,0($29)
	addiu	$4,$4,-0x6b60
	sd	$16,0x10($29)
	jal	0x001a6e28
	lw	$16,0x164($2)
	lui	$2,0x17
	addiu	$3,$0,0x50
	addiu	$2,$2,0x5b18
	sw	$3,0x30($16)
	sw	$2,0x14($16)
	sw	$0,0xd0($16)
	nop	
loc_00175ad8:
	jal	0x00203aa0
	addiu	$4,$0,1
	lw	$2,0xd0($16)
	andi	$2,$2,0x10
	beqz	$2,loc_00175ad8
	lui	$4,0x56
	jal	0x001a6e28
	addiu	$4,$4,-0x6b90
loc_00175af8:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x5e
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_00175af8
	nop	
	nop	
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	lui	$4,0x56
	sd	$31,0x10($29)
	jal	0x001a6e28
	addiu	$4,$4,-0x6bc0
	lw	$6,-0x6714($28)
	addiu	$5,$0,0x5e
	jal	0x0013ff88
	lw	$4,-0x6e0c($28)
	jal	0x0014a3a8
	nop	
	ld	$31,0x10($29)
	jr	$31
	.set	macro
	.set	reorder
	.end	girlBrain_sub_1759A8
