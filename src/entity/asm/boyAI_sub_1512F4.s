	.text
	.p2align 3
	.globl	boyAI_sub_1512F4
	.ent	boyAI_sub_1512F4
boyAI_sub_1512F4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	nop	
	lw	$4,0($29)
	jal	0x0014b228
	addiu	$5,$0,0x17
	lw	$4,0($29)
	jal	0x0014b248
	addiu	$5,$0,0x13
	beqz	$2,loc_00151324
	nop	
	lw	$4,0($29)
	jal	0x0014b228
	addiu	$5,$0,0x16
loc_00151324:
	lw	$4,0($29)
	jal	0x0014b048
	nop	
	beqz	$2,loc_00151344
	nop	
	lw	$4,0($29)
	jal	0x0014b228
	addiu	$5,$0,4
loc_00151344:
	lw	$4,0($29)
	jal	0x00145648
	nop	
	lw	$2,-0x6e08($28)
	.word	0x14400037
	lw	$4,0x78($29)
	jal	0x0013eb50
	addiu	$4,$0,0xe
	move	$16,$2
	beql	$16,$0,loc_001513a0
	move	$16,$0
	addiu	$17,$0,5
	nop	
loc_00151378:
	jal	0x001f40c8
	move	$4,$16
	beq	$2,$17,loc_001513a0
	nop	
	jal	0x0013ebe0
	move	$4,$16
	move	$16,$2
	bnez	$16,loc_00151378
	nop	
	move	$16,$0
loc_001513a0:
	.word	0x52000024
	lw	$4,0x78($29)
	lw	$2,0x2d0($20)
	andi	$2,$2,8
	bnez	$2,loc_001513c4
	lbu	$2,-0x4c06($28)
	sb	$0,-0x4c05($28)
	.word	0x1000001c
	sb	$0,-0x4c06($28)
loc_001513c4:
	bnel	$2,$0,loc_00151400
	addiu	$2,$0,1
	jal	0x0015eff8
	move	$4,$16
	move	$5,$2
	jal	0x00195510
	move	$4,$21
	mtc1	$0,$f1
	.word	0x46000834
	nop	
	bc1t	loc_001513f8
	addiu	$2,$0,1
	move	$2,$0
loc_001513f8:
	sb	$2,-0x4c05($28)
	addiu	$2,$0,1
loc_00151400:
	addiu	$4,$0,0x1000
	sb	$2,-0x4c06($28)
	ld	$3,0x20($20)
	andi	$3,$3,0x3000
	.word	0x14640008
	lw	$4,0x78($29)
	addiu	$3,$0,1
	lw	$4,0($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_1512F4
