	.text
	.p2align 3
	.globl	type67_hC
	.ent	type67_hC
type67_hC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$3,0x15c($4)
	move	$2,$0
	sw	$4,-0x5518($28)
	jr	$31
	sw	$0,0x74($3)
	.set	macro
	.set	reorder
	.end	type67_hC
