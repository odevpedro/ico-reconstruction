	.text
	.p2align 3
	.globl	girlBrain_sub_16F620
	.ent	girlBrain_sub_16F620
girlBrain_sub_16F620:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	blez	$23,loc_0016f774
	move	$18,$0
	lui	$3,0x6b
	nop	
loc_0016f630:
	sll	$22,$18,4
	addiu	$2,$3,-0x44c0
	move	$5,$21
	addu	$19,$22,$2
	move	$17,$0
	jal	0x00194398
	move	$4,$19
	lui	$4,0x28
	addiu	$2,$4,0x2ac0
	lw	$3,0x1f60($2)
	blez	$3,loc_0016f6b0
	mov.s	$f20,$f0
	addiu	$20,$18,1
	addiu	$5,$0,0x30
loc_0016f668:
	lui	$2,0x28
	.word	0x02252818
	addiu	$16,$2,0x4a40
	move	$4,$19
	jal	0x00194398
	addu	$5,$5,$16
	.word	0x46140034
	nop	
	bc1t	loc_0016f6a8
	addiu	$17,$17,1
	lw	$2,-0x20($16)
	slt	$2,$17,$2
	bnez	$2,loc_0016f668
	addiu	$5,$0,0x30
	b	loc_0016f6b8
	lui	$4,0x28
loc_0016f6a8:
	b	loc_0016f720
	move	$2,$0
loc_0016f6b0:
	addiu	$20,$18,1
	lui	$4,0x28
loc_0016f6b8:
	addiu	$3,$4,0x2ac0
	lw	$2,0x1f60($3)
	blez	$2,loc_0016f71c
	move	$17,$0
	addiu	$16,$0,0x30
	nop	
loc_0016f6d0:
	lui	$2,0x28
	.word	0x02308018
	addiu	$18,$2,0x4a40
	move	$4,$21
	addu	$16,$16,$18
	jal	0x00194398
	move	$5,$16
	mov.s	$f20,$f0
	move	$5,$16
	jal	0x00194398
	move	$4,$19
	.word	0x46140034
	nop	
	bc1t	loc_0016f6a8
	addiu	$17,$17,1
	lw	$2,-0x20($18)
	slt	$2,$17,$2
	bnez	$2,loc_0016f6d0
	addiu	$16,$0,0x30
loc_0016f71c:
	addiu	$2,$0,1
loc_0016f720:
	andi	$2,$2,0xff
	beqz	$2,loc_0016f764
	lui	$3,0x6b
	lui	$1,0xbf80
	mtc1	$1,$f12
	addiu	$5,$3,-0x44c0
	move	$4,$29
	jal	0x00243b18
	addu	$5,$22,$5
	lui	$1,0x428c
	mtc1	$1,$f12
	move	$4,$29
	mtc1	$0,$f13
	addiu	$5,$0,0xff
	move	$6,$0
	jal	0x001873a0
	move	$7,$0
loc_0016f764:
	move	$18,$20
	slt	$2,$18,$23
	bnez	$2,loc_0016f630
	lui	$3,0x6b
loc_0016f774:
	.word	0x1ae0007c
	move	$18,$0
	lui	$4,0x6b
	sll	$22,$18,4
	nop	
	addiu	$2,$4,-0x44c0
	move	$5,$21
	addu	$20,$22,$2
	move	$17,$0
	jal	0x00194398
	move	$4,$20
	lui	$3,0x28
	addiu	$2,$3,0x2ac0
	lw	$3,0x1f60($2)
	blez	$3,loc_0016f7f0
	mov.s	$f20,$f0
	addiu	$5,$0,0x30
loc_0016f7b8:
	lui	$4,0x28
	.word	0x02252818
	addiu	$16,$4,0x4a40
	move	$4,$20
	jal	0x00194398
	addu	$5,$5,$16
	.word	0x46140034
	nop	
	.word	0x4501005d
	addiu	$17,$17,1
	lw	$2,-0x20($16)
	slt	$2,$17,$2
	bnel	$2,$0,loc_0016f7b8
	addiu	$5,$0,0x30
loc_0016f7f0:
	lui	$2,0x28
	addiu	$3,$2,0x2ac0
	lw	$2,0x1f60($3)
	.word	0x18400015
	move	$17,$0
	addiu	$16,$0,0x30
	lui	$3,0x28
	.word	0x02308018
	addiu	$19,$3,0x4a40
	move	$4,$21
	addu	$16,$16,$19
	jal	0x00194398
	move	$5,$16
	mov.s	$f20,$f0
	move	$5,$16
	jal	0x00194398
	move	$4,$20
	.word	0x46140034
	nop	
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16F620
