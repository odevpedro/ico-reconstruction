	.text
	.p2align 3
	.globl	eBrainReturnInit
	.ent	eBrainReturnInit
eBrainReturnInit:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jal	0x0013ebe0
	move	$4,$16
	move	$16,$2
	.word	0x5600ffe0
	lw	$2,0x16c($16)
	addiu	$2,$0,1
	andi	$2,$2,0xff
	beqz	$2,loc_00192308
	lwc1	$f0,0($29)
	lwc1	$f2,4($29)
	.word	0x1000004d
	lwc1	$f1,8($29)
loc_00192308:
	lui	$2,0x56
	move	$19,$0
	addiu	$17,$2,0x4150
	nop	
	lw	$3,0xc($17)
	lw	$2,8($20)
	.word	0x5462003c
	addiu	$19,$19,1
	lwc1	$f0,0($17)
	addiu	$4,$0,0x11
	lwc1	$f1,4($17)
	lwc1	$f2,8($17)
	neg.s	$f0,$f0
	neg.s	$f1,$f1
	neg.s	$f2,$f2
	swc1	$f0,0x10($29)
	swc1	$f1,0x14($29)
	jal	0x0013eb50
	swc1	$f2,0x18($29)
	move	$16,$2
	.word	0x1200002b
	addiu	$2,$0,1
	lwc1	$f23,-0x7944($28)
	lui	$1,0x4248
	mtc1	$1,$f22
	mtc1	$0,$f21
	lui	$1,0x42c8
	mtc1	$1,$f20
	lw	$2,0x16c($16)
	nop	
	.set	macro
	.set	reorder
	.end	eBrainReturnInit
