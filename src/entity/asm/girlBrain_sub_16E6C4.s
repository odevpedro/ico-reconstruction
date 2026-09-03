	.text
	.p2align 3
	.globl	girlBrain_sub_16E6C4
	.ent	girlBrain_sub_16E6C4
girlBrain_sub_16E6C4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sw	$2,0x340($7)
	lw	$4,0x90($29)
	lw	$5,0xe8($29)
	slt	$2,$5,$4
	bnez	$2,loc_0016e6f0
	addiu	$2,$0,2
	lui	$6,0x28
	addiu	$2,$6,0x2ac0
	lbu	$3,0x58e3($2)
	beqz	$3,loc_0016e760
	addiu	$2,$0,2
loc_0016e6f0:
	beq	$18,$2,loc_0016e710
	slti	$2,$18,3
	bnez	$2,loc_0016e730
	addiu	$2,$0,3
	beq	$18,$2,loc_0016e720
	nop	
	b	loc_0016e730
	nop	
loc_0016e710:
	lui	$1,0x3f00
	mtc1	$1,$f1
	b	loc_0016e734
	nop	
loc_0016e720:
	lui	$1,0x3f80
	mtc1	$1,$f1
	b	loc_0016e734
	nop	
loc_0016e730:
	mtc1	$0,$f1
loc_0016e734:
	mtc1	$0,$f0
	.word	0x46000832
	nop	
	bc1t	loc_0016e764
	lw	$7,0xec($29)
	addiu	$2,$0,1
	sw	$0,0x90($29)
	sw	$2,0x340($16)
	b	loc_0016e760
	move	$4,$0
	lw	$4,0x90($29)
loc_0016e760:
	lw	$7,0xec($29)
loc_0016e764:
	addiu	$2,$4,1
	slt	$3,$4,$7
	beqz	$3,loc_0016e79c
	sw	$2,0x90($29)
	lw	$2,0x94($29)
	move	$6,$0
	lw	$4,0($29)
	addiu	$7,$0,2
	lw	$5,0x30($29)
	addiu	$8,$0,1
	jal	0x0014b168
	sw	$0,0x33c($2)
	b	loc_0016e7c8
	lw	$4,0x94($29)
loc_0016e79c:
	lw	$4,0($29)
	move	$5,$0
	move	$6,$0
	addiu	$7,$0,6
	jal	0x0014b168
	addiu	$8,$0,1
	lw	$3,0x94($29)
	addiu	$2,$0,8
	sw	$0,0x90($29)
	sw	$2,0x340($3)
	lw	$4,0x94($29)
loc_0016e7c8:
	ori	$2,$0,0xc000
	dsll32	$2,$2,8
	ld	$3,0x18($4)
	and	$2,$3,$2
	beqz	$2,loc_0016e8fc
	nop	
	ori	$2,$0,0x8000
	dsll32	$2,$2,9
	or	$2,$3,$2
	b	loc_0016e8fc
	sd	$2,0x18($4)
	lw	$2,0x50($29)
	bnez	$2,loc_0016e888
	addiu	$2,$0,5
	b	loc_0016e888
	addiu	$2,$0,7
	lui	$8,0x27
	addiu	$3,$0,0xa
	lw	$2,0x4ec0($8)
	addiu	$4,$8,0x4ec0
	lw	$5,4($4)
	addiu	$7,$0,0x3c
	mult	$2,$2,$3
	addiu	$8,$0,0xc8
	lw	$4,0x90($29)
	beql	$5,$0,loc_0016e834
	break	0,7
loc_0016e834:
	addiu	$3,$0,0x3c
	move	$6,$4
	subu	$2,$7,$2
	addiu	$4,$4,1
	div	$0,$2,$5
	mflo	$2
	mult	$2,$2,$8
	div	$0,$2,$7
	mflo	$2
	slt	$6,$6,$2
	beqz	$6,loc_0016e884
	sw	$4,0x90($29)
	lw	$2,0x50($29)
	sw	$0,0x33c($16)
	sw	$2,0x40($16)
	lw	$4,0($29)
	jal	0x0015bcc8
	lw	$5,0x54($29)
	b	loc_0016e8fc
	nop	
loc_0016e884:
	addiu	$2,$0,8
loc_0016e888:
	sw	$0,0x90($29)
	b	loc_0016e8fc
	sw	$2,0x340($16)
	lw	$2,0x90($29)
	addiu	$3,$0,2
	addiu	$2,$2,1
	bne	$2,$3,loc_0016e8b4
	sw	$2,0x90($29)
	lw	$4,0($29)
	jal	0x0016ac10
	move	$5,$21
loc_0016e8b4:
	lui	$2,0x27
	addiu	$5,$0,0xa
	lw	$4,0x4ec0($2)
	addiu	$6,$2,0x4ec0
	lw	$3,4($6)
	addiu	$2,$0,0x3c
	.word	0x00852018
	beql	$3,$0,loc_0016e8d8
	break	0,7
loc_0016e8d8:
	lw	$5,0x90($29)
	subu	$2,$2,$4
	div	$0,$2,$3
	mflo	$2
	slt	$2,$2,$5
	beqz	$2,loc_0016e8fc
	lw	$2,0x94($29)
	sw	$0,0x90($29)
	sw	$0,0x340($2)
loc_0016e8fc:
	jal	0x00203aa0
	addiu	$4,$0,1
	.word	0x1000fc82
	nop	
	nop	
	addiu	$29,$29,-0x160
	lui	$2,0x28
	sd	$17,0xe0($29)
	swc1	$f21,0x158($29)
	move	$17,$4
	lw	$2,0x5cf0($2)
	mov.s	$f21,$f12
	sd	$31,0x140($29)
	sd	$22,0x130($29)
	sd	$21,0x120($29)
	sd	$20,0x110($29)
	sd	$19,0x100($29)
	sd	$18,0xf0($29)
	sd	$16,0xd0($29)
	.word	0x1040007b
	swc1	$f20,0x150($29)
	sw	$0,0($17)
	move	$18,$0
	sw	$0,4($17)
	sw	$0,8($17)
	.word	0x1840002b
	lwc1	$f20,0($17)
	lui	$22,0x29
	addiu	$20,$29,0x10
	addiu	$21,$29,0x20
	addiu	$16,$0,0x30
	addiu	$5,$22,-0x7d00
	.word	0x02508018
	addiu	$6,$5,-0x25f0
	addiu	$19,$5,-0x5840
	move	$4,$29
	jal	0x00243ae8
	addu	$6,$16,$6
	sw	$0,4($29)
	move	$4,$29
	jal	0x00243978
	move	$5,$29
	addu	$16,$16,$19
	lui	$1,0x3f80
	mtc1	$1,$f1
	lwc1	$f0,0x3260($16)
	.word	0x46010034
	nop	
	.word	0x45030001
	mov.s	$f0,$f1
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16E6C4
