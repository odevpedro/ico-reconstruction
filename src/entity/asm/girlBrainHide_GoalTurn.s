	.text
	.p2align 3
	.globl	girlBrainHide_GoalTurn
	.ent	girlBrainHide_GoalTurn
girlBrainHide_GoalTurn:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x70
	sd	$17,0x30($29)
	sd	$19,0x50($29)
	sd	$18,0x40($29)
	andi	$19,$5,0xff
	lw	$17,-0x6e08($28)
	move	$18,$4
	sd	$31,0x60($29)
	move	$4,$29
	sd	$16,0x20($29)
	jal	0x00104748
	move	$5,$17
	move	$4,$29
	jal	0x00194508
	move	$5,$18
	bltzl	$2,loc_0016ebac
	negu	$2,$2
loc_0016ebac:
	slti	$2,$2,0x2e
	bnez	$2,loc_0016ec5c
	ld	$31,0x60($29)
	lw	$3,0x164($17)
	addiu	$16,$29,0x10
	lwc1	$f1,0($18)
	move	$4,$16
	lw	$2,0x678($3)
	move	$5,$17
	swc1	$f1,0x3d0($2)
	lwc1	$f0,4($18)
	swc1	$f0,0x3d4($2)
	lwc1	$f1,8($18)
	jal	0x0016a5f0
	swc1	$f1,0x3d8($2)
	move	$4,$16
	jal	0x00194508
	move	$5,$18
	blezl	$2,loc_0016ec30
	lui	$4,0x56
	lui	$4,0x56
	jal	0x001a6e28
	addiu	$4,$4,-0x6990
	beqz	$19,loc_0016ec20
	move	$4,$17
	jal	0x0015bcc8
	addiu	$5,$0,0xdc
	b	loc_0016ec5c
	ld	$31,0x60($29)
loc_0016ec20:
	jal	0x0015bcc8
	addiu	$5,$0,0xde
	b	loc_0016ec5c
	ld	$31,0x60($29)
loc_0016ec30:
	jal	0x001a6e28
	addiu	$4,$4,-0x6980
	beqz	$19,loc_0016ec50
	move	$4,$17
	jal	0x0015bcc8
	addiu	$5,$0,0xdb
	b	loc_0016ec5c
	ld	$31,0x60($29)
loc_0016ec50:
	jal	0x0015bcc8
	addiu	$5,$0,0xdd
	ld	$31,0x60($29)
loc_0016ec5c:
	ld	$19,0x50($29)
	ld	$18,0x40($29)
	ld	$17,0x30($29)
	ld	$16,0x20($29)
	jr	$31
	addiu	$29,$29,0x70
	nop	
	.set	macro
	.set	reorder
	.end	girlBrainHide_GoalTurn
