	.text
	.p2align 3
	.globl	girlBrain_sub_16B688
	.ent	girlBrain_sub_16B688
girlBrain_sub_16B688:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	slti	$2,$2,0xc9
	beqz	$2,loc_0016b6fc
	addiu	$4,$29,0x70
	addiu	$5,$16,0x10
	jal	0x00243ae8
	move	$6,$19
	addiu	$4,$29,0x70
	sw	$0,0x74($29)
	sw	$0,0x7c($29)
	addiu	$5,$29,0x30
	jal	0x002438b8
	move	$6,$4
	lwc1	$f0,0x78($29)
	.word	0x4600b034
	nop	
	bc1fl	loc_0016b700
	addiu	$17,$17,1
	.word	0x46140034
	nop	
	bc1f	loc_0016b6fc
	lwc1	$f1,0x70($29)
	lwc1	$f0,0x74($29)
	mul.s	$f1,$f1,$f1
	mul.s	$f0,$f0,$f0
	add.s	$f1,$f1,$f0
	.word	0x46150834
	nop	
	bc1t	loc_0016b710
	addiu	$2,$0,1
loc_0016b6fc:
	addiu	$17,$17,1
loc_0016b700:
	slt	$2,$17,$18
	.word	0x1440ffd8
	addiu	$16,$16,0x30
	move	$2,$0
loc_0016b710:
	andi	$2,$2,0xff
	bnez	$2,loc_0016b724
	addiu	$4,$23,0x2ac0
	b	loc_0016b750
	addiu	$22,$0,3
loc_0016b724:
	lw	$2,0x1f60($4)
	beqz	$2,loc_0016b74c
	addiu	$5,$4,0x1f80
	jal	0x00194398
	addiu	$4,$4,0x5810
	lwc1	$f1,-0x7b98($28)
	.word	0x46010034
	nop	
	bc1t	loc_0016b754
	move	$2,$22
loc_0016b74c:
	addiu	$22,$0,4
loc_0016b750:
	move	$2,$22
loc_0016b754:
	ld	$31,0x100($29)
	ld	$23,0xf0($29)
	ld	$22,0xe0($29)
	ld	$21,0xd0($29)
	ld	$20,0xc0($29)
	ld	$19,0xb0($29)
	ld	$18,0xa0($29)
	ld	$17,0x90($29)
	ld	$16,0x80($29)
	lwc1	$f22,0x120($29)
	lwc1	$f21,0x118($29)
	lwc1	$f20,0x110($29)
	jr	$31
	addiu	$29,$29,0x130
	nop	
	addiu	$29,$29,-0xd0
	sd	$20,0x70($29)
	lw	$20,-0x6e08($28)
	sd	$30,0xb0($29)
	sd	$23,0xa0($29)
	move	$30,$0
	sd	$22,0x90($29)
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16B688
