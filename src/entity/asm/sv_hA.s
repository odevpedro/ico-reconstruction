	.text
	.p2align 3
	.globl	sv_hA
	.ent	sv_hA
sv_hA:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	j	0x123c00
	lw	$4,0x15c($4)
	move	$6,$4
	lw	$2,-0x5c10($28)
	beqz	$2,loc_0010ec8c
	sw	$6,-0x6d6c($28)
	lui	$5,0x55
	lui	$4,0xcccc
	addiu	$5,$5,0x3e90
	j	0x1a6b10
	ori	$4,$4,0xcc00
loc_0010ec8c:
	jr	$31
	nop	
	.set	macro
	.set	reorder
	.end	sv_hA
