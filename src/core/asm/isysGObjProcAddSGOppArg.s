	.text
	.p2align 3
	.globl	isysGObjProcAddSGOppArg
	.ent	isysGObjProcAddSGOppArg
isysGObjProcAddSGOppArg:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x20
	addiu	$2,$0,1
	sd	$31,0x10($29)
	sd	$16,0($29)
	lw	$3,0($4)
	beq	$3,$2,2f
	slti	$2,$3,2
	beqz	$2,0f
	addiu	$2,$0,2
	beqz	$3,1f
	lui	$16,0x55
	b	4f
	addiu	$5,$0,0x27b
0:
	beq	$3,$2,3f
	lui	$16,0x55
	b	4f
	addiu	$5,$0,0x27b
1:
	addiu	$5,$0,0x272
	addiu	$16,$16,0x7cc8
	jal	0x001ad768
	move	$4,$16
	lui	$6,0x63
	move	$4,$16
	addiu	$6,$6,0x2220
	jal	0x00263ff0
	addiu	$5,$0,0x272
	b	5f
	move	$2,$0
2:
	lui	$16,0x55
	addiu	$5,$0,0x275
	addiu	$16,$16,0x7cc8
	jal	0x001ad768
	move	$4,$16
	lui	$6,0x63
	move	$4,$16
	addiu	$6,$6,0x2220
	jal	0x00263ff0
	addiu	$5,$0,0x275
	b	5f
	move	$2,$0
3:
	jal	0x00140e48
	nop	
	b	6f
	ld	$31,0x10($29)
4:
	addiu	$16,$16,0x7cc8
	jal	0x001ad768
	move	$4,$16
	lui	$6,0x63
	move	$4,$16
	addiu	$6,$6,0x2220
	jal	0x00263ff0
	addiu	$5,$0,0x27b
	move	$2,$0
5:
	ld	$31,0x10($29)
6:
	ld	$16,0($29)
	jr	$31
	.set	macro
	.set	reorder
	.end	isysGObjProcAddSGOppArg
