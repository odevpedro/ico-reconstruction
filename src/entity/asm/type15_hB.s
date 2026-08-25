	.text
	.p2align 3
	.globl	type15_hB
	.ent	type15_hB
type15_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x80
	sd	$21,0x60($29)
	sd	$31,0x70($29)
	move	$21,$4
	sd	$20,0x50($29)
	sd	$19,0x40($29)
	sd	$18,0x30($29)
	sd	$17,0x20($29)
	sd	$16,0x10($29)
	lw	$5,0x15c($21)
	lw	$20,0x800($5)
	lw	$2,0($20)
	addiu	$4,$2,1
	sltiu	$3,$4,5
	beqz	$3,loc_001ea950
	move	$6,$5
	lui	$2,0x62
	sll	$3,$4,2
	addiu	$2,$2,-0x5e90
	addu	$3,$3,$2
	lw	$4,0($3)
	jr	$4
	nop	
	lw	$18,0($5)
	beqz	$18,loc_001ea6a8
	addiu	$2,$0,0x21
	lw	$3,0xc($18)
	beql	$3,$2,loc_001ea6ac
	lw	$18,-0x6e0c($28)
	lw	$17,0x800($6)
	lw	$5,0x20($17)
	blez	$5,loc_001ea798
	move	$16,$0
	lw	$3,0x24($17)
loc_001ea670:
	sll	$2,$16,2
	addu	$2,$2,$3
	lw	$4,0($2)
	beql	$4,$0,loc_001ea694
	addiu	$16,$16,1
	jal	0x001bb800
	move	$5,$18
	lw	$5,0x20($17)
	addiu	$16,$16,1
loc_001ea694:
	slt	$2,$16,$5
	bnel	$2,$0,loc_001ea670
	lw	$3,0x24($17)
	b	loc_001ea79c
	lw	$2,0x2c($20)
loc_001ea6a8:
	lw	$18,-0x6e0c($28)
loc_001ea6ac:
	beqz	$18,loc_001ea708
	lui	$19,0x62
	lw	$17,0x800($5)
	lw	$5,0x20($17)
	blez	$5,loc_001ea708
	move	$16,$0
	lui	$19,0x62
	lw	$3,0x24($17)
	nop	
loc_001ea6d0:
	sll	$2,$16,2
	addu	$2,$2,$3
	lw	$4,0($2)
	beql	$4,$0,loc_001ea6f4
	addiu	$16,$16,1
	jal	0x001bb800
	move	$5,$18
	lw	$5,0x20($17)
	addiu	$16,$16,1
loc_001ea6f4:
	slt	$2,$16,$5
	bnel	$2,$0,loc_001ea6d0
	lw	$3,0x24($17)
	b	loc_001ea70c
	lw	$2,0x34($20)
loc_001ea708:
	lw	$2,0x34($20)
loc_001ea70c:
	addiu	$3,$19,0x3468
	addiu	$5,$0,1
	sll	$2,$2,5
	addu	$3,$3,$2
	lw	$4,0x14($3)
	bnel	$4,$5,loc_001ea79c
	lw	$2,0x2c($20)
	lw	$18,-0x6e08($28)
	beqz	$18,loc_001ea784
	move	$2,$0
	lw	$2,0x15c($21)
	lw	$17,0x800($2)
	lw	$5,0x20($17)
	blez	$5,loc_001ea780
	move	$16,$0
	lw	$3,0x24($17)
	nop	
loc_001ea750:
	sll	$2,$16,2
	addu	$2,$2,$3
	lw	$4,0($2)
	beql	$4,$0,loc_001ea774
	addiu	$16,$16,1
	jal	0x001bb800
	move	$5,$18
	lw	$5,0x20($17)
	addiu	$16,$16,1
loc_001ea774:
	slt	$2,$16,$5
	bnel	$2,$0,loc_001ea750
	lw	$3,0x24($17)
loc_001ea780:
	addiu	$2,$0,1
loc_001ea784:
	bnel	$2,$0,loc_001ea79c
	lw	$2,0x2c($20)
	lui	$4,0x62
	jal	0x001a6e28
	addiu	$4,$4,-0x5f20
loc_001ea798:
	lw	$2,0x2c($20)
loc_001ea79c:
	move	$3,$2
	addiu	$2,$2,1
	sltiu	$3,$3,0xb
	bnez	$3,loc_001ea980
	sw	$2,0x2c($20)
	lw	$2,0x3c($20)
	bnez	$2,loc_001ea7e4
	lw	$2,-0x5318($28)
	lui	$4,0x62
	addiu	$4,$4,-0x5ec0
	move	$5,$2
	addiu	$2,$2,1
	jal	0x001a6e28
	sw	$2,-0x5318($28)
	jal	0x001eb488
	move	$4,$21
	b	loc_001ea980
	sw	$0,0($20)
loc_001ea7e4:
	lui	$4,0x62
	addiu	$4,$4,-0x5eb0
	move	$5,$2
	addiu	$2,$2,1
	jal	0x001a6e28
	sw	$2,-0x5318($28)
	jal	0x001eba18
	move	$4,$21
	b	loc_001ea980
	sw	$0,0($20)
	lw	$3,0x38($20)
	addiu	$2,$0,-1
	beq	$3,$2,loc_001ea8c0
	addiu	$2,$0,1
	lw	$18,0x800($5)
	move	$17,$0
	lw	$19,0x20($18)
	blez	$19,loc_001ea86c
	sw	$2,0x28($18)
	lw	$2,0x24($18)
	nop	
loc_001ea838:
	sll	$16,$17,2
	addiu	$17,$17,1
	addu	$2,$16,$2
	jal	0x001baf48
	lw	$4,0($2)
	lw	$2,0x24($18)
	addiu	$5,$0,1
	addu	$16,$16,$2
	jal	0x001ba5d0
	lw	$4,0($16)
	slt	$2,$17,$19
	bnel	$2,$0,loc_001ea838
	lw	$2,0x24($18)
loc_001ea86c:
	b	loc_001ea8a8
	lw	$17,0x38($20)
	nop	
loc_001ea878:
	sll	$16,$17,2
	addiu	$5,$0,0xcf
	addiu	$17,$17,1
	addu	$2,$16,$2
	lw	$4,0($2)
	jal	0x0013ff88
	move	$6,$4
	lw	$3,0x24($20)
	move	$5,$0
	addu	$16,$16,$3
	jal	0x001ba5d0
	lw	$4,0($16)
loc_001ea8a8:
	lw	$2,0x20($20)
	slt	$2,$17,$2
	bnel	$2,$0,loc_001ea878
	lw	$2,0x24($20)
	b	loc_001ea948
	addiu	$2,$0,1
loc_001ea8c0:
	lw	$16,0($5)
	beqz	$16,loc_001ea980
	addiu	$2,$0,0x21
	lw	$3,0xc($16)
	beql	$3,$2,loc_001ea984
	lw	$2,0x30($20)
	jal	0x001bb7e0
	move	$4,$16
	bnel	$2,$0,loc_001ea984
	lw	$2,0x30($20)
	lw	$2,0x164($16)
	lw	$5,0x50($2)
	beqz	$5,loc_001ea93c
	move	$4,$29
	jal	0x001921f8
	move	$16,$0
	lw	$2,0x15c($21)
	lw	$18,0x800($2)
	lw	$17,0x20($18)
	blez	$17,loc_001ea93c
	move	$19,$29
	lw	$3,0x24($18)
loc_001ea918:
	sll	$2,$16,2
	move	$5,$19
	addiu	$16,$16,1
	addu	$2,$2,$3
	jal	0x00104240
	lw	$4,0($2)
	slt	$2,$16,$17
	bnel	$2,$0,loc_001ea918
	lw	$3,0x24($18)
loc_001ea93c:
	jal	0x001ea3e0
	move	$4,$21
	addiu	$2,$0,1
loc_001ea948:
	b	loc_001ea980
	sw	$2,0($20)
loc_001ea950:
	lw	$4,0($5)
	beqz	$4,loc_001ea980
	addiu	$2,$0,0x21
	lw	$3,0xc($4)
	beql	$3,$2,loc_001ea984
	lw	$2,0x30($20)
	jal	0x001bb7e0
	nop	
	beql	$2,$0,loc_001ea984
	lw	$2,0x30($20)
	jal	0x001ea480
	move	$4,$21
loc_001ea980:
	lw	$2,0x30($20)
loc_001ea984:
	move	$3,$2
	addiu	$2,$2,1
	sltiu	$3,$3,0x1f
	bnez	$3,loc_001ea9a4
	sw	$2,0x30($20)
	sw	$0,0x30($20)
	jal	0x001ae460
	move	$4,$21
loc_001ea9a4:
	ld	$31,0x70($29)
	ld	$21,0x60($29)
	ld	$20,0x50($29)
	ld	$19,0x40($29)
	ld	$18,0x30($29)
	ld	$17,0x20($29)
	ld	$16,0x10($29)
	jr	$31
	addiu	$29,$29,0x80
	.set	macro
	.set	reorder
	.end	type15_hB
