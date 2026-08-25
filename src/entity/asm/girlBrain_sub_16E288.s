	.text
	.p2align 3
	.globl	girlBrain_sub_16E288
	.ent	girlBrain_sub_16E288
girlBrain_sub_16E288:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	nop	
	bc1f	loc_0016e2a8
	lw	$16,0x94($29)
	lw	$3,0x94($29)
	addiu	$2,$0,2
	sw	$0,0x90($29)
	.word	0x10000196
	sw	$2,0x340($3)
loc_0016e2a8:
	bnel	$17,$0,loc_0016e2b8
	ld	$2,0x3e0($16)
	.word	0x10000087
	addiu	$2,$0,2
loc_0016e2b8:
	dsll	$2,$2,0x10
	dsra32	$2,$2,0
	andi	$2,$2,1
	.word	0x14400082
	addiu	$2,$0,2
	.word	0x56e00081
	sw	$0,0x90($29)
	bnel	$20,$0,loc_0016e2ec
	lw	$3,0x30($16)
	lui	$1,0x42c8
	mtc1	$1,$f3
	b	loc_0016e36c
	lwc1	$f0,0x3d0($16)
loc_0016e2ec:
	addiu	$2,$0,3
	bne	$3,$2,loc_0016e364
	addiu	$2,$0,2
	beq	$18,$2,loc_0016e318
	slti	$2,$18,3
	bnez	$2,loc_0016e338
	nop	
	beq	$18,$3,loc_0016e328
	nop	
	b	loc_0016e338
	nop	
loc_0016e318:
	lui	$1,0x3f00
	mtc1	$1,$f12
	b	loc_0016e33c
	nop	
loc_0016e328:
	lui	$1,0x3f80
	mtc1	$1,$f12
	b	loc_0016e33c
	nop	
loc_0016e338:
	mtc1	$0,$f12
loc_0016e33c:
	jal	0x00263fb0
	nop	
	lui	$1,0x56
	ld	$5,-0x6998($1)
	jal	0x00263110
	move	$4,$2
	lui	$1,0x42c8
	mtc1	$1,$f3
	bgtzl	$2,loc_0016e36c
	lwc1	$f0,0x3d0($16)
loc_0016e364:
	mtc1	$0,$f3
	lwc1	$f0,0x3d0($16)
loc_0016e36c:
	lwc1	$f1,0x3d4($16)
	lwc1	$f2,0x3d8($16)
	swc1	$f0,0x110($16)
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16E288
