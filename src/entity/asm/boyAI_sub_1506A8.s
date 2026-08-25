	.text
	.p2align 3
	.globl	boyAI_sub_1506A8
	.ent	boyAI_sub_1506A8
boyAI_sub_1506A8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$4,0x678($3)
	lwc1	$f12,0x460($4)
	cvt.s.w	$f12,$f12
	jal	0x00150348
	nop	
	lw	$2,0($29)
	lw	$3,0x164($2)
	lw	$4,0x678($3)
	lw	$2,0x460($4)
	addiu	$2,$2,-1
	sw	$2,0x460($4)
	lw	$4,0($29)
	jal	0x0014cee8
	nop	
	lw	$4,0($29)
	jal	0x0014e300
	nop	
	lw	$4,0($29)
	jal	0x00148340
	nop	
	ld	$2,0x18($20)
	addiu	$4,$0,1
	ld	$3,0x20($20)
	dsrl32	$2,$2,0xd
	dsll	$3,$3,0x19
	dsra32	$3,$3,0
	andi	$2,$2,1
	andi	$3,$3,1
	movz	$4,$2,$3
	.word	0x14800029
	nop	
	lwc1	$f2,0x33c($20)
	mtc1	$0,$f0
	.word	0x46001032
	bc1t	loc_00150770
	nop	
	lw	$5,0($29)
	move	$4,$22
	jal	0x0014d6b0
	lw	$6,0x7c($29)
	beqz	$2,loc_0015076c
	lwc1	$f0,0x10($29)
	lwc1	$f2,0x33c($20)
	swc1	$f0,0x110($20)
	lwc1	$f1,0x14($29)
	swc1	$f1,0x114($20)
	lwc1	$f0,0x18($29)
	b	loc_00150770
	swc1	$f0,0x118($20)
loc_0015076c:
	lwc1	$f2,0x33c($20)
loc_00150770:
	lwc1	$f0,-0x7d04($28)
	.word	0x46020034
	nop	
	.set	macro
	.set	reorder
	.end	boyAI_sub_1506A8
