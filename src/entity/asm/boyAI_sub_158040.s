	.text
	.p2align 3
	.globl	boyAI_sub_158040
	.ent	boyAI_sub_158040
boyAI_sub_158040:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	bne	$2,$4,loc_0015805c
	addiu	$4,$0,4
	jal	0x001ae6f8
	addiu	$5,$0,0xd7d
	addiu	$4,$0,0x21
	jal	0x001ae6f8
	addiu	$5,$0,0xd7e
loc_0015805c:
	jal	0x0013eb50
	addiu	$4,$0,0x2e
	move	$5,$2
	bnez	$5,loc_0015807c
	nop	
	jal	0x0013eb50
	addiu	$4,$0,0x40
	move	$5,$2
loc_0015807c:
	lw	$2,0($29)
	addiu	$4,$0,4
	lw	$3,0xc($2)
	bne	$3,$4,loc_001580a4
	nop	
	beqz	$5,loc_001580a4
	move	$4,$5
	lw	$6,0($29)
	jal	0x0013ff88
	addiu	$5,$0,0x12
loc_001580a4:
	lw	$2,0($29)
	addiu	$4,$0,0xd7d
	lw	$3,8($2)
	bne	$3,$4,loc_001580c8
	lui	$21,0x27
	lw	$4,0($29)
	jal	0x00193f48
	nop	
	nop	
loc_001580c8:
	lw	$4,4($29)
	beqz	$4,loc_001580ec
	nop	
	lw	$4,0($29)
	jal	0x0015eff8
	nop	
	lw	$4,0($29)
	jal	0x00154e60
	move	$5,$2
loc_001580ec:
	lw	$2,0($29)
	addiu	$4,$0,4
	lw	$3,0xc($2)
	.word	0x1464006a
	addiu	$20,$23,1
	mtc1	$0,$f20
	jal	0x00203aa0
	addiu	$4,$0,1
	move	$22,$21
	lw	$4,0($29)
	jal	0x0015eff8
	nop	
	lw	$3,0($29)
	move	$4,$2
	lw	$5,0x164($3)
	.set	macro
	.set	reorder
	.end	boyAI_sub_158040
