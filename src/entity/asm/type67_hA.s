	.text
	.p2align 3
	.globl	type67_hA
	.ent	type67_hA
type67_hA:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$2,-0x5b3c($28)
	beqz	$2,loc_001d03bc
	lw	$4,-0x5518($28)
	beqz	$4,loc_001d03bc
	nop	
	j	0x167528
	move	$5,$0
loc_001d03bc:
	jr	$31
	nop	
	.set	macro
	.set	reorder
	.end	type67_hA
