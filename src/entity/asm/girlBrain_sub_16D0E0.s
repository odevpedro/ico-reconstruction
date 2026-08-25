	.text
	.p2align 3
	.globl	girlBrain_sub_16D0E0
	.ent	girlBrain_sub_16D0E0
girlBrain_sub_16D0E0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jal	0x00201280
	swc1	$f0,0x78($29)
	lui	$1,0x42c8
	mtc1	$1,$f12
	move	$16,$2
	move	$4,$22
	addiu	$5,$0,0xff
	move	$6,$0
	jal	0x00187328
	move	$7,$0
	bnel	$16,$0,loc_0016d118
	lwc1	$f2,0x3d4($18)
	b	loc_0016d158
	sw	$0,0x33c($18)
loc_0016d118:
	lw	$2,0($29)
	lwc1	$f1,0x3d0($18)
	move	$4,$2
	lwc1	$f0,0x3d8($18)
	lw	$16,0x164($2)
	swc1	$f1,0x110($18)
	swc1	$f2,0x114($18)
	lui	$1,0x3f80
	mtc1	$1,$f21
	lui	$1,0x3f00
	mtc1	$1,$f20
	jal	0x00201d50
	swc1	$f0,0x118($18)
	beql	$2,$0,loc_0016d158
	swc1	$f21,0x33c($16)
	swc1	$f20,0x33c($16)
loc_0016d158:
	move	$4,$19
	jal	0x00194400
	move	$5,$30
	mov.s	$f2,$f0
	lwc1	$f1,0x24($29)
	lwc1	$f0,0x74($29)
	sub.s	$f1,$f1,$f0
	.word	0x46160834
	nop	
	bc1tl	loc_0016d184
	neg.s	$f1,$f1
loc_0016d184:
	lw	$4,-0x6e0c($28)
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16D0E0
