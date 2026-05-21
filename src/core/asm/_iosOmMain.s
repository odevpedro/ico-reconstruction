	.text
	.p2align 3
	.globl	_iosOmMain
	.ent	_iosOmMain
_iosOmMain:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0xa0
	lui	$2,0x28
	sd	$23,0x70($29)
	move	$5,$0
	sd	$18,0x20($29)
	lui	$23,0x27
	sd	$17,0x10($29)
	addiu	$18,$2,0x1a70
	sd	$31,0x90($29)
	addiu	$17,$23,0x4ec0
	sd	$30,0x80($29)
	sd	$22,0x60($29)
	sd	$21,0x50($29)
	sd	$20,0x40($29)
	sd	$19,0x30($29)
	sd	$16,0($29)
	lw	$4,-0x6724($28)
	sll	$2,$5,2
loc_0013fa18:
	srav	$3,$4,$5
	addu	$2,$2,$18
	andi	$3,$3,1
	beqz	$3,loc_0013fa80
	lw	$16,0($2)
	beqz	$16,loc_0013fa80
	addiu	$21,$5,1
	lw	$2,0x14($17)
loc_0013fa38:
	beqz	$2,loc_0013fa4c
	sw	$16,-0x6714($28)
	lw	$2,0x170($16)
	beql	$2,$0,loc_0013fa70
	lw	$16,0x10($16)
loc_0013fa4c:
	lw	$2,0x16c($16)
	beql	$2,$0,loc_0013fa70
	lw	$16,0x10($16)
	lw	$2,0x28($16)
	beqz	$2,loc_0013fa6c
	move	$4,$16
	jalr	$2
	nop	
loc_0013fa6c:
	lw	$16,0x10($16)
loc_0013fa70:
	bnel	$16,$0,loc_0013fa38
	lw	$2,0x14($17)
	b	loc_0013fa84
	lw	$4,-0x6724($28)
loc_0013fa80:
	addiu	$21,$5,1
loc_0013fa84:
	move	$5,$21
	slti	$2,$5,8
	bnez	$2,loc_0013fa18
	sll	$2,$5,2
	lui	$2,0x28
	move	$5,$0
	addiu	$30,$2,0x1a70
	sll	$2,$5,2
	nop	
loc_0013faa8:
	srav	$3,$4,$5
	addu	$2,$2,$30
	andi	$3,$3,1
	beqz	$3,loc_0013fbb4
	lw	$18,0($2)
	beqz	$18,loc_0013fbb4
	addiu	$21,$5,1
	addiu	$3,$23,0x4ec0
loc_0013fac8:
	sw	$18,-0x6714($28)
	lw	$2,0x14($3)
	beql	$2,$0,loc_0013fae8
	lw	$2,0x16c($18)
	lw	$2,0x170($18)
	beql	$2,$0,loc_0013fba4
	lw	$18,0x10($18)
	lw	$2,0x16c($18)
loc_0013fae8:
	beql	$2,$0,loc_0013fba4
	lw	$18,0x10($18)
	addiu	$19,$0,0x13
	addiu	$22,$0,0x22
	lw	$16,0x2c($18)
	nop	
loc_0013fb00:
	beqz	$16,loc_0013fb90
	addiu	$20,$19,1
	lw	$2,0x14($16)
	nop	
loc_0013fb10:
	bnel	$2,$19,loc_0013fb80
	lw	$16,8($16)
	lw	$2,0x18($16)
	beql	$2,$0,loc_0013fb80
	lw	$16,8($16)
	lw	$2,0x10($16)
	bnez	$2,loc_0013fb64
	sw	$16,-0x6710($28)
	addiu	$17,$16,0x24
	jal	0x0013d8a0
	move	$4,$17
	beq	$2,$22,loc_0013fb54
	nop	
	jal	0x0013d928
	move	$4,$17
	b	loc_0013fb7c
	sw	$0,-0x6710($28)
loc_0013fb54:
	jal	0x0013f6b8
	move	$4,$16
	b	loc_0013fb7c
	sw	$0,-0x6710($28)
loc_0013fb64:
	lw	$2,0x1c($16)
	beql	$2,$0,loc_0013fb7c
	sw	$0,-0x6710($28)
	jalr	$2
	move	$4,$18
	sw	$0,-0x6710($28)
loc_0013fb7c:
	lw	$16,8($16)
loc_0013fb80:
	bnel	$16,$0,loc_0013fb10
	lw	$2,0x14($16)
	b	loc_0013fb94
	move	$19,$20
loc_0013fb90:
	move	$19,$20
loc_0013fb94:
	slti	$2,$19,0x1b
	bnel	$2,$0,loc_0013fb00
	lw	$16,0x2c($18)
	lw	$18,0x10($18)
loc_0013fba4:
	bnez	$18,loc_0013fac8
	addiu	$3,$23,0x4ec0
	b	loc_0013fbb8
	lw	$4,-0x6724($28)
loc_0013fbb4:
	addiu	$21,$5,1
loc_0013fbb8:
	move	$5,$21
	slti	$2,$5,8
	bnel	$2,$0,loc_0013faa8
	sll	$2,$5,2
	ld	$31,0x90($29)
	ld	$30,0x80($29)
	ld	$23,0x70($29)
	ld	$22,0x60($29)
	ld	$21,0x50($29)
	ld	$20,0x40($29)
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0xa0
	j	0x13f9d0
	nop	
	.set	macro
	.set	reorder
	.end	_iosOmMain
