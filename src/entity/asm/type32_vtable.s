	.text
	.p2align 3
	.globl	type32_vtable
	.ent	type32_vtable
type32_vtable:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0xa0
	sd	$20,0x60($29)
	sd	$19,0x50($29)
	sd	$18,0x40($29)
	move	$19,$0
	sd	$31,0x90($29)
	move	$18,$4
	sd	$22,0x80($29)
	addiu	$20,$18,0x54
	sd	$21,0x70($29)
	sd	$17,0x30($29)
	sd	$16,0x20($29)
	lw	$16,0x164($18)
	lw	$2,0x15c($18)
	lw	$3,4($20)
	blez	$3,loc_0019589c
	lw	$22,0x800($2)
	addiu	$21,$29,0x10
	sll	$2,$19,3
	nop	
loc_00195738:
	move	$4,$29
	addiu	$2,$2,8
	addu	$17,$20,$2
	jal	0x00104508
	lw	$5,4($17)
	move	$4,$21
	jal	0x00104508
	move	$5,$18
	move	$4,$29
	jal	0x00118578
	move	$5,$21
	lw	$3,0($17)
	addiu	$2,$0,0x185
	beq	$3,$2,loc_00195874
	mov.s	$f1,$f0
	sltiu	$2,$3,0x186
	beqz	$2,loc_001957bc
	addiu	$2,$0,0xa
	beq	$3,$2,loc_00195854
	sltiu	$2,$3,0xb
	beqz	$2,loc_001957a0
	addiu	$2,$0,7
	beq	$3,$2,loc_00195878
	move	$4,$18
	b	loc_0019588c
	lw	$2,4($20)
loc_001957a0:
	sltiu	$2,$3,0x1c
	beqz	$2,loc_00195888
	sltiu	$2,$3,0x1a
	bnel	$2,$0,loc_0019588c
	lw	$2,4($20)
	b	loc_00195878
	move	$4,$18
loc_001957bc:
	addiu	$2,$0,0x188
	beq	$3,$2,loc_00195874
	sltiu	$2,$3,0x189
	beqz	$2,loc_001957e8
	addiu	$2,$0,0x186
	beq	$3,$2,loc_00195874
	addiu	$2,$0,0x187
	beq	$3,$2,loc_00195878
	move	$4,$18
	b	loc_0019588c
	lw	$2,4($20)
loc_001957e8:
	addiu	$2,$0,0x189
	beq	$3,$2,loc_00195804
	addiu	$2,$0,0x18a
	beq	$3,$2,loc_00195834
	nop	
	b	loc_0019588c
	lw	$2,4($20)
loc_00195804:
	lui	$1,0x4348
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1f	loc_00195888
	addiu	$2,$0,1
	addiu	$4,$22,0x20
	sb	$2,0x10($22)
	jal	0x00104508
	lw	$5,4($17)
	b	loc_0019588c
	lw	$2,4($20)
loc_00195834:
	lui	$1,0x437a
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1f	loc_00195888
	move	$4,$18
	b	loc_0019587c
	addiu	$5,$0,0x121
loc_00195854:
	lw	$3,0x15c($18)
	lw	$2,0x4a0($3)
	addiu	$2,$2,-0x436
	sltiu	$2,$2,3
	beqz	$2,loc_00195888
	move	$4,$18
	b	loc_0019587c
	addiu	$5,$0,0x122
loc_00195874:
	move	$4,$18
loc_00195878:
	addiu	$5,$0,0x121
loc_0019587c:
	jal	0x001e29e8
	addiu	$6,$16,0x610
	sw	$2,0x120($16)
loc_00195888:
	lw	$2,4($20)
loc_0019588c:
	addiu	$19,$19,1
	slt	$2,$19,$2
	bnez	$2,loc_00195738
	sll	$2,$19,3
loc_0019589c:
	sw	$0,4($20)
	ld	$31,0x90($29)
	lw	$2,0x15c($18)
	ld	$22,0x80($29)
	ldl	$3,0x187($2)
	ldr	$3,0x180($2)
	ldl	$4,0x18f($2)
	ldr	$4,0x188($2)
	ldl	$5,0x197($2)
	ldr	$5,0x190($2)
	ldl	$6,0x19f($2)
	ldr	$6,0x198($2)
	sdl	$3,0x617($16)
	sdr	$3,0x610($16)
	sdl	$4,0x61f($16)
	sdr	$4,0x618($16)
	sdl	$5,0x627($16)
	sdr	$5,0x620($16)
	sdl	$6,0x62f($16)
	sdr	$6,0x628($16)
	ld	$21,0x70($29)
	ld	$20,0x60($29)
	ld	$19,0x50($29)
	ld	$18,0x40($29)
	ld	$17,0x30($29)
	ld	$16,0x20($29)
	jr	$31
	addiu	$29,$29,0xa0
	.set	macro
	.set	reorder
	.end	type32_vtable
