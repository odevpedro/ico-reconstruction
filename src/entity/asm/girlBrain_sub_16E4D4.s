	.text
	.p2align 3
	.globl	girlBrain_sub_16E4D4
	.ent	girlBrain_sub_16E4D4
girlBrain_sub_16E4D4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	.word	0x10000031
	sw	$2,0x340($16)
	addiu	$2,$0,2
	beq	$18,$2,loc_0016e500
	slti	$2,$18,3
	bnez	$2,loc_0016e520
	addiu	$2,$0,3
	beq	$18,$2,loc_0016e510
	nop	
	b	loc_0016e520
	nop	
loc_0016e500:
	lui	$1,0x3f00
	mtc1	$1,$f20
	b	loc_0016e524
	nop	
loc_0016e510:
	lui	$1,0x3f80
	mtc1	$1,$f20
	b	loc_0016e524
	nop	
loc_0016e520:
	mtc1	$0,$f20
loc_0016e524:
	lui	$1,0x3f00
	mtc1	$1,$f21
	lw	$4,0($29)
	.word	0x4614a834
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16E4D4
