	.text
	.p2align 3
	.globl	girlBrain_sub_16C1A0
	.ent	girlBrain_sub_16C1A0
girlBrain_sub_16C1A0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$2,0x3240($18)
	sw	$2,0x370($3)
	lw	$3,0x678($7)
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
	.word	0x10400019
	nop	
	.word	0x16000017
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
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16C1A0
