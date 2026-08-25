	.text
	.p2align 3
	.globl	boyAI_sub_143950
	.ent	boyAI_sub_143950
boyAI_sub_143950:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	.word	0x00a32818
	sd	$17,0x10($29)
	lui	$2,0x5f
	move	$17,$4
	addiu	$2,$2,0x3120
	sd	$30,0x80($29)
	sd	$23,0x70($29)
	lui	$3,0x6b
	sd	$22,0x60($29)
	addiu	$6,$2,4
	sd	$21,0x50($29)
	lui	$4,0x55
	sd	$20,0x40($29)
	slti	$23,$17,2
	sd	$19,0x30($29)
	addu	$22,$5,$2
	sd	$18,0x20($29)
	addu	$21,$5,$6
	sd	$16,0($29)
	addiu	$20,$4,0x7cc8
	sd	$31,0x90($29)
	addiu	$16,$3,-0x6a50
	move	$19,$0
	move	$18,$0
	lui	$30,0x63
	lw	$2,0($16)
	.word	0x1040002c
	addiu	$3,$0,1
	lhu	$2,6($16)
	.word	0x5443002a
	lui	$3,0x6b
	lhu	$2,4($16)
	.word	0x54510027
	lui	$3,0x6b
	addiu	$18,$0,1
	beql	$17,$18,loc_00143a0c
	lhu	$3,0($16)
	beqz	$23,loc_001439fc
	addiu	$2,$0,2
	.word	0x1220001f
	move	$4,$20
	b	loc_00143a54
	nop	
loc_001439fc:
	beq	$17,$2,loc_00143a34
	move	$4,$20
	b	loc_00143a54
	nop	
loc_00143a0c:
	lw	$2,0($22)
	.word	0x10430016
	move	$4,$16
	jal	0x00141e00
	addiu	$19,$19,1
	addiu	$4,$0,1
	jal	0x00141868
	addiu	$5,$0,1
	.word	0x10000010
	lui	$3,0x6b
loc_00143a34:
	lhu	$3,0($16)
	lw	$2,0($21)
	.word	0x1043000c
	lui	$3,0x6b
	jal	0x00141e00
	move	$4,$16
	.word	0x10000008
	lui	$3,0x6b
loc_00143a54:
	jal	0x001ad768
	addiu	$5,$0,0x6c8
	move	$4,$20
	addiu	$5,$0,0x6c8
	.set	macro
	.set	reorder
	.end	boyAI_sub_143950
