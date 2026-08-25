	.text
	.p2align 3
	.globl	ap1_hB
	.ent	ap1_hB
ap1_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x30
	addiu	$3,$0,5
	sd	$17,0x10($29)
	sd	$31,0x20($29)
	move	$17,$4
	sd	$16,0($29)
	lw	$2,0x15c($17)
	lw	$16,0x800($2)
	lw	$4,8($16)
	beq	$4,$3,loc_001ba3ec
	slti	$2,$4,6
	beqz	$2,loc_001ba374
	addiu	$2,$0,4
	beq	$4,$2,loc_001ba3f8
	addiu	$2,$0,6
	b	loc_001ba38c
	lw	$3,0x274($16)
loc_001ba374:
	addiu	$2,$0,6
	beq	$4,$2,loc_001ba400
	addiu	$2,$0,7
	beq	$4,$2,loc_001ba404
	nop	
	lw	$3,0x274($16)
loc_001ba38c:
	slti	$2,$3,0xa
	beqz	$2,loc_001ba3a8
	addiu	$2,$3,1
	move	$4,$17
	jal	0x001ba2e0
	sw	$2,0x274($16)
	lw	$4,8($16)
loc_001ba3a8:
	lui	$3,0x4c
	sll	$2,$4,3
	addiu	$3,$3,-0x11a0
	addu	$3,$3,$2
	lw	$2,4($3)
	jalr	$2
	move	$4,$17
	sw	$2,8($16)
	lw	$3,0x15c($17)
	lw	$4,0x800($3)
	lw	$2,0x270($4)
	addiu	$2,$2,1
	slti	$3,$2,0x21
	bnez	$3,loc_001ba404
	sw	$2,0x270($4)
	b	loc_001ba404
	sw	$0,0x270($4)
loc_001ba3ec:
	addiu	$2,$0,4
	b	loc_001ba404
	sw	$2,8($16)
loc_001ba3f8:
	b	loc_001ba404
	sw	$2,8($16)
loc_001ba400:
	sw	$0,0x16c($17)
loc_001ba404:
	jal	0x001ba090
	move	$4,$17
	jal	0x001b99b0
	move	$4,$17
	jal	0x00105278
	nop	
	lw	$3,0x15c($17)
	lui	$6,0x4c
	move	$4,$2
	addiu	$6,$6,-0x1160
	jal	0x001185d0
	lw	$5,0xc($3)
	jal	0x00105278
	nop	
	lw	$4,0x19c($16)
	lui	$1,0x3f80
	mtc1	$1,$f12
	jal	0x001cf930
	move	$5,$2
	lw	$3,4($16)
	beql	$3,$0,loc_001ba4a4
	lw	$3,0x15c($17)
	jal	0x00105278
	nop	
	lw	$3,0x15c($17)
	move	$4,$2
	jal	0x00105f20
	lw	$5,0xc($3)
	jal	0x00105038
	addiu	$4,$0,0x4000
	jal	0x00104f48
	addiu	$4,$0,0x4000
	jal	0x00105278
	lw	$16,0x15c($17)
	lui	$6,0x4c
	lw	$4,0xc($16)
	move	$5,$2
	jal	0x001185d0
	addiu	$6,$6,-0x10e0
	lw	$3,0x15c($17)
loc_001ba4a4:
	mtc1	$0,$f2
	lw	$2,0xc($3)
	lwc1	$f1,0x54($3)
	lwc1	$f0,0x34($2)
	sub.s	$f1,$f1,$f0
	.word	0x46020834
	bc1f	loc_001ba4e4
	nop	
	neg.s	$f1,$f1
	lwc1	$f0,-0x77c4($28)
	.word	0x46010034
	nop	
	bc1t	loc_001ba4f8
	addiu	$2,$0,0x800
	b	loc_001ba51c
	ld	$31,0x20($29)
loc_001ba4e4:
	lwc1	$f0,-0x77c0($28)
	.word	0x46010034
	nop	
	bc1f	loc_001ba518
	addiu	$2,$0,0x800
loc_001ba4f8:
	lui	$4,0x62
	ld	$31,0x20($29)
	addiu	$4,$4,-0x7a70
	ld	$17,0x10($29)
	ld	$16,0($29)
	sw	$2,0x5f8($3)
	j	0x1a6e28
	addiu	$29,$29,0x30
loc_001ba518:
	ld	$31,0x20($29)
loc_001ba51c:
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x30
	.set	macro
	.set	reorder
	.end	ap1_hB
