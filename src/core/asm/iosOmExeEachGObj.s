	.text
	.p2align 3
	.globl	iosOmExeEachGObj
	.ent	iosOmExeEachGObj
iosOmExeEachGObj:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$sp,$sp,-0x40
	lui	$2,0x28
	sd	$18,0x20($sp)
	sll	$4,$4,0x2
	sd	$17,0x10($sp)
	addiu	$2,$2,0x1a70
	sd	$31,0x30($sp)
	addu	$4,$4,$2
	sd	$16,0x0($sp)
	move	$18,$5
	lw	$16,0x0($4)
	beqz	$16,1f
	move	$17,$6
2:	move	$4,$16
	jalr	$18
	move	$5,$17
	lw	$16,0x10($16)
	bnez	$16,2b
	move	$4,$16
1:	ld	$31,0x30($sp)
	ld	$18,0x20($sp)
	ld	$17,0x10($sp)
	ld	$16,0x0($sp)
	jr	$31
	addiu	$sp,$sp,0x40
	.end	iosOmExeEachGObj
