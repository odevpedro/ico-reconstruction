	.text
	.p2align 3
	.globl	chandelier_hB
	.ent	chandelier_hB
chandelier_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x30
	sd	$16,0($29)
	move	$16,$4
	sd	$17,0x10($29)
	sd	$31,0x20($29)
	jal	0x0013eb50
	addiu	$4,$0,0x14
	move	$17,$2
	beqz	$17,loc_001c345c
	ld	$31,0x20($29)
	jal	0x00105278
	nop	
	lw	$3,0x15c($16)
	move	$4,$2
	jal	0x00105f20
	lw	$5,0xc($3)
	mtc1	$0,$f12
	lui	$1,0x4248
	mtc1	$1,$f13
	lui	$1,0x437a
	mtc1	$1,$f14
	jal	0x00105308
	nop	
	jal	0x00105278
	nop	
	move	$4,$17
	ld	$31,0x20($29)
	ld	$17,0x10($29)
	addiu	$5,$2,0x30
	ld	$16,0($29)
	move	$6,$0
	j	0x1e93d0
	addiu	$29,$29,0x30
loc_001c345c:
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x30
	.set	macro
	.set	reorder
	.end	chandelier_hB
