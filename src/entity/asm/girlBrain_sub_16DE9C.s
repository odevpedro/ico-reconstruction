	.text
	.p2align 3
	.globl	girlBrain_sub_16DE9C
	.ent	girlBrain_sub_16DE9C
girlBrain_sub_16DE9C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jal	0x00243b18
	addiu	$5,$29,0x40
	lui	$1,0x428c
	mtc1	$1,$f12
	addiu	$4,$29,0xb0
	mtc1	$0,$f13
	addiu	$5,$0,0xff
	move	$6,$0
	jal	0x001873a0
	move	$7,$0
	lw	$4,0x30($29)
	beql	$4,$0,loc_0016def0
	lw	$4,-0x6e0c($28)
	jal	0x00182b58
	nop	
	beqz	$2,loc_0016def0
	lw	$4,-0x6e0c($28)
	lw	$4,0($29)
	jal	0x0016ac10
	move	$5,$21
	lw	$4,-0x6e0c($28)
loc_0016def0:
	.word	0x1080000e
	addiu	$3,$0,0x31
	lw	$5,0x164($4)
	lw	$2,0x30($5)
	.word	0x1443000b
	lw	$16,0x94($29)
	lw	$3,0x30($29)
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16DE9C
