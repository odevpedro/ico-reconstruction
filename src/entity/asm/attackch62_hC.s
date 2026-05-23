	.text
	.p2align 3
	.globl	attackch62_hC
	.ent	attackch62_hC
attackch62_hC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x20
	lui	$6,0x62
	sd	$16,0($29)
	addiu	$6,$6,-0x7a18
	move	$16,$5
	lw	$4,-0x68e0($28)
	sd	$31,0x10($29)
	addiu	$5,$0,0xc
	jal	0x0013a0f8
	addiu	$7,$0,0x1b
	lw	$4,0x30($16)
	move	$3,$2
	sw	$0,4($3)
	sw	$0,0($4)
	ld	$31,0x10($29)
	ld	$16,0($29)
	sw	$4,0($3)
	sw	$0,8($3)
	jr	$31
	addiu	$29,$29,0x20
	lw	$2,0x15c($4)
	lw	$2,0($2)
	beqz	$2,loc_001bbebc
	nop	
	lw	$2,0x16c($2)
	beql	$2,$0,loc_001bbebc
	sw	$0,0x16c($4)
loc_001bbebc:
	jr	$31
	nop	
	nop	
	addiu	$29,$29,-0x20
	lw	$3,-0x5c28($28)
	sd	$16,0($29)
	sd	$31,0x10($29)
	move	$16,$4
	lw	$2,0x15c($16)
	.word	0x10600021
	lw	$2,0x800($2)
	lw	$2,4($2)
	.word	0x1440001f
	ld	$31,0x10($29)
	jal	0x001118b8
	addiu	$4,$0,0xb
	jal	0x00112148
	addiu	$4,$0,1
	addiu	$4,$0,1
	addiu	$5,$0,5
	jal	0x00111fa8
	addiu	$6,$0,0x80
	jal	0x00105278
	nop	
	jal	0x00118678
	move	$4,$2
	jal	0x00105278
	nop	
	lw	$3,0x15c($16)
	addiu	$4,$2,0x30
	lw	$5,0xc($3)
	jal	0x00105f00
	addiu	$5,$5,0x30
	jal	0x001bbfd0
	move	$4,$16
	lui	$4,0x4c
	mov.s	$f12,$f0
	.set	macro
	.set	reorder
	.end	attackch62_hC
