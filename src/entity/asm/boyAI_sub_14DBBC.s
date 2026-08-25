	.text
	.p2align 3
	.globl	boyAI_sub_14DBBC
	.ent	boyAI_sub_14DBBC
boyAI_sub_14DBBC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	.word	0x1040fffa
	lw	$2,-0x6e08($28)
	b	loc_0014dbe0
	addiu	$5,$0,0x5d
	nop	
loc_0014dbd0:
	jal	0x00203aa0
	nop	
	lw	$2,-0x6e08($28)
	addiu	$5,$0,0x5d
loc_0014dbe0:
	beqz	$2,loc_0014dbf0
	move	$4,$2
	jal	0x0013ff88
	lw	$6,-0x6714($28)
loc_0014dbf0:
	lw	$2,0xd0($18)
	andi	$2,$2,8
	beqz	$2,loc_0014dbd0
	addiu	$4,$0,1
	sw	$0,0x14($18)
	nop	
loc_0014dc08:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x43
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0014dc08
	nop	
	nop	
	addiu	$29,$29,-0x90
	sw	$4,0($29)
	sd	$21,0x70($29)
	lui	$4,0x56
	sd	$20,0x60($29)
	addiu	$4,$4,-0x7d50
	sd	$19,0x50($29)
	sd	$17,0x30($29)
	sd	$16,0x20($29)
	lw	$2,0($29)
	sd	$31,0x80($29)
	sd	$18,0x40($29)
	jal	0x001a6e28
	lw	$18,0x164($2)
	.set	macro
	.set	reorder
	.end	boyAI_sub_14DBBC
