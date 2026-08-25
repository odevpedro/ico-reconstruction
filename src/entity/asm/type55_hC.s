	.text
	.p2align 3
	.globl	type55_hC
	.ent	type55_hC
type55_hC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x30
	lui	$6,0x62
	sd	$17,0x10($29)
	addiu	$6,$6,-0x70f8
	move	$17,$4
	sd	$16,0($29)
	move	$16,$5
	lw	$4,-0x68e0($28)
	sd	$31,0x20($29)
	addiu	$5,$0,0xc
	jal	0x0013a0f8
	addiu	$7,$0,0x17
	lwc1	$f0,0x28($16)
	move	$3,$2
	lui	$1,0x3f80
	mtc1	$1,$f1
	swc1	$f0,0($3)
	nop	
	nop	
	div.s	$f0,$f1,$f0
	sw	$0,8($3)
	ld	$31,0x20($29)
	ld	$16,0($29)
	swc1	$f0,4($3)
	lw	$4,0x15c($17)
	ld	$17,0x10($29)
	lw	$3,0x840($4)
	swc1	$f1,0x20($3)
	swc1	$f1,0x28($3)
	swc1	$f1,0x24($3)
	jr	$31
	addiu	$29,$29,0x30
	.set	macro
	.set	reorder
	.end	type55_hC
