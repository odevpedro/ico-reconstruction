	.text
	.p2align 3
	.globl	girlBrain_sub_16C198
	.ent	girlBrain_sub_16C198
girlBrain_sub_16C198:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	bnez	$5,loc_0016c1ac
	lw	$3,0x678($7)
	lw	$2,0x3240($18)
	sw	$2,0x370($3)
	lw	$3,0x678($7)
loc_0016c1ac:
	lw	$2,0x370($3)
	bnel	$2,$0,loc_0016c1c4
	lw	$2,0x678($7)
	lw	$2,0x3240($18)
	sw	$2,0x370($3)
	lw	$2,0x678($7)
loc_0016c1c4:
	lw	$3,0x370($2)
	sw	$3,0x80($17)
	lw	$4,0x370($2)
	bnez	$8,loc_0016c1e8
	sw	$4,0x84($17)
	jal	0x00193ee0
	nop	
	beqz	$2,loc_0016c1ec
	move	$16,$0
loc_0016c1e8:
	addiu	$16,$0,1
loc_0016c1ec:
	ld	$4,0x20($20)
	dsll	$2,$4,0x11
	dsra32	$2,$2,0
	andi	$2,$2,1
	beqz	$2,loc_0016c264
	nop	
	bnez	$16,loc_0016c264
	nop	
	jal	0x0014b358
	move	$4,$0
	lw	$5,0x4ec0($22)
	addiu	$4,$0,0xa
	addiu	$6,$22,0x4ec0
	addiu	$3,$0,0x3c
	.word	0x00a42818
	lw	$2,4($6)
	lw	$4,0($29)
	beql	$2,$0,loc_0016c238
	break	0,7
loc_0016c238:
	lw	$6,0x164($4)
	subu	$3,$3,$5
	div	$0,$3,$2
	lw	$4,0x678($6)
	mflo	$3
	mtc1	$3,$f1
	cvt.s.w	$f1,$f1
	mul.s	$f0,$f0,$f1
	cvt.w.s	$f1,$f0
	swc1	$f1,0x380($4)
	ld	$4,0x20($20)
loc_0016c264:
	lui	$3,0xffff
	ori	$3,$3,0x7fff
	dsll	$2,$16,0xf
	and	$3,$4,$3
	lw	$4,0($29)
	or	$3,$3,$2
	sd	$3,0x20($20)
	lw	$2,0x164($4)
	lw	$3,0x678($2)
	lw	$4,0x380($3)
	.word	0x10800031
	nop	
	lw	$2,0($29)
	move	$4,$0
	lw	$3,0x164($2)
	lw	$5,0x678($3)
	lw	$2,0x380($5)
	addiu	$2,$2,-1
	jal	0x0014b358
	sw	$2,0x380($5)
	lw	$6,0x4ec0($22)
	addiu	$2,$0,0xa
	addiu	$5,$22,0x4ec0
	addiu	$4,$0,0x3c
	.word	0x00c23018
	lw	$3,4($5)
	lw	$2,0($29)
	beql	$3,$0,loc_0016c2d8
	break	0,7
loc_0016c2d8:
	lui	$1,0x4120
	mtc1	$1,$f3
	lw	$5,0x164($2)
	subu	$4,$4,$6
	lui	$1,0x4000
	mtc1	$1,$f4
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16C198
