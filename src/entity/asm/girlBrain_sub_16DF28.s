	.text
	.p2align 3
	.globl	girlBrain_sub_16DF28
	.ent	girlBrain_sub_16DF28
girlBrain_sub_16DF28:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	move	$5,$21
	lw	$16,0x94($29)
	addiu	$3,$0,0x4e
	lw	$2,0x30($16)
	bnel	$2,$3,loc_0016df64
	lw	$3,0x340($16)
	lw	$3,0x30($29)
	lw	$2,-0x6e0c($28)
	bnel	$3,$2,loc_0016df64
	lw	$3,0x340($16)
	lw	$4,0($29)
	jal	0x0016ac10
	move	$5,$21
	lw	$16,0x94($29)
	lw	$3,0x340($16)
loc_0016df64:
	addiu	$2,$0,3
	bnel	$3,$2,loc_0016dfd4
	lui	$5,0x29
	addiu	$2,$0,2
	beq	$18,$2,loc_0016df94
	slti	$2,$18,3
	bnez	$2,loc_0016dfb4
	nop	
	beq	$18,$3,loc_0016dfa4
	nop	
	b	loc_0016dfb4
	nop	
loc_0016df94:
	lui	$1,0x3f00
	mtc1	$1,$f1
	b	loc_0016dfb8
	nop	
loc_0016dfa4:
	lui	$1,0x3f80
	mtc1	$1,$f1
	b	loc_0016dfb8
	nop	
loc_0016dfb4:
	mtc1	$0,$f1
loc_0016dfb8:
	mtc1	$0,$f0
	.word	0x46000832
	nop	
	bc1f	loc_0016dfd4
	lui	$5,0x29
	sw	$0,0x90($29)
	sw	$0,0x340($16)
loc_0016dfd4:
	addiu	$3,$0,-1
	addiu	$4,$5,-0x5770
	mtc1	$0,$f22
	lh	$2,0x24($4)
	bnel	$2,$3,loc_0016dfec
	lwc1	$f22,0x20($4)
loc_0016dfec:
	lui	$1,0x3f80
	mtc1	$1,$f21
	jal	0x0014b358
	addiu	$4,$0,0x1b
	mov.s	$f20,$f0
	jal	0x0014b358
	addiu	$4,$0,0x1b
	sub.s	$f0,$f21,$f0
	addiu	$4,$0,0x1b
	mul.s	$f0,$f22,$f0
	jal	0x0014b358
	add.s	$f20,$f20,$f0
	.word	0x4600a034
	bc1f	loc_0016e038
	nop	
	jal	0x0014b358
	addiu	$4,$0,0x1b
	b	loc_0016e04c
	nop	
loc_0016e038:
	.word	0x4614a834
	nop	
	bc1f	loc_0016e04c
	mov.s	$f0,$f20
	mov.s	$f0,$f21
loc_0016e04c:
	lw	$4,0($29)
	mov.s	$f12,$f0
	jal	0x0014b330
	addiu	$5,$0,1
	lui	$6,0x29
	addiu	$3,$0,-1
	addiu	$4,$6,-0x5770
	mtc1	$0,$f0
	lh	$2,0x24($4)
	bnel	$2,$3,loc_0016e078
	lwc1	$f0,0x20($4)
loc_0016e078:
	lui	$1,0x4080
	mtc1	$1,$f20
	addiu	$4,$0,0x19
	jal	0x0014b358
	mul.s	$f20,$f0,$f20
	mov.s	$f21,$f0
	jal	0x0014b358
	addiu	$4,$0,0x1a
	mov.s	$f1,$f0
	sub.s	$f0,$f1,$f21
	mul.s	$f20,$f20,$f0
	add.s	$f20,$f21,$f20
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16DF28
