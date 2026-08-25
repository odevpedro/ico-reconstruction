	.text
	.p2align 3
	.globl	girlBrain_sub_16E374
	.ent	girlBrain_sub_16E374
girlBrain_sub_16E374:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	swc1	$f0,0x110($16)
	swc1	$f1,0x114($16)
	swc1	$f2,0x118($16)
	lw	$8,0xf4($29)
	.word	0x1100003e
	lwc1	$f0,0x58($29)
	lwc1	$f1,0x3e8($16)
	lui	$1,0x4248
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1f	loc_0016e3d0
	lwc1	$f0,0x5c($29)
	lwc1	$f1,0x3ec($16)
	.word	0x46000834
	nop	
	bc1f	loc_0016e3cc
	addiu	$2,$0,4
	sw	$0,0x90($29)
	sw	$2,0x340($16)
	.word	0x10000075
	sw	$0,0x33c($16)
loc_0016e3cc:
	lwc1	$f1,0x3e8($16)
loc_0016e3d0:
	lui	$1,0x4396
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1f	loc_0016e400
	addiu	$2,$0,2
	lw	$2,0($29)
	lui	$1,0x3f00
	mtc1	$1,$f0
	lw	$3,0x164($2)
	.word	0x10000068
	swc1	$f0,0x33c($3)
loc_0016e400:
	beq	$18,$2,loc_0016e420
	slti	$2,$18,3
	.word	0x1440000d
	addiu	$2,$0,3
	.word	0x12420007
	nop	
	.word	0x10000009
	nop	
loc_0016e420:
	lui	$1,0x3f00
	mtc1	$1,$f20
	.word	0x10000006
	nop	
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16E374
