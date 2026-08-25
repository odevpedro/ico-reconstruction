	.text
	.p2align 3
	.globl	girlBrain_sub_16DDD8
	.ent	girlBrain_sub_16DDD8
girlBrain_sub_16DDD8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lui	$1,0x4120
	mtc1	$1,$f1
	sub.s	$f0,$f0,$f1
	b	loc_0016ddf8
	swc1	$f0,0x44($29)
	lw	$5,0x30($29)
	jal	0x0010a498
	move	$4,$19
loc_0016ddf8:
	lw	$2,-0x6e0c($28)
	lw	$4,0x164($2)
	ld	$3,0x20($4)
	dsll	$3,$3,6
	dsra32	$3,$3,0
	andi	$3,$3,1
	beqz	$3,loc_0016de90
	nop	
	jal	0x0016d488
	lw	$4,0x104($29)
	move	$3,$2
	addiu	$2,$0,1
	beq	$3,$2,loc_0016de40
	addiu	$2,$0,2
	beq	$3,$2,loc_0016de74
	lwc1	$f0,0xd0($29)
	b	loc_0016de90
	nop	
loc_0016de40:
	lwc1	$f0,0x10($29)
	lwc1	$f1,0x14($29)
	lwc1	$f2,0x18($29)
	lw	$2,0x30($29)
	swc1	$f0,0x40($29)
	swc1	$f1,0x44($29)
	beqz	$2,loc_0016de90
	swc1	$f2,0x48($29)
	lw	$4,0($29)
	jal	0x0016ac10
	move	$5,$21
	b	loc_0016de90
	nop	
loc_0016de74:
	addiu	$3,$0,1
	lwc1	$f1,0xd4($29)
	lwc1	$f2,0xd8($29)
	sw	$3,0xf4($29)
	swc1	$f0,0x40($29)
	swc1	$f1,0x44($29)
	swc1	$f2,0x48($29)
loc_0016de90:
	lui	$1,0xbf80
	mtc1	$1,$f12
	addiu	$4,$29,0xb0
	jal	0x00243b18
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16DDD8
