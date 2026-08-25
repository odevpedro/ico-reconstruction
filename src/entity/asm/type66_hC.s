	.text
	.p2align 3
	.globl	type66_hC
	.ent	type66_hC
type66_hC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$2,$0,-1
	sw	$0,-0x4b3c($28)
	sw	$2,-0x4b38($28)
	move	$2,$0
	jr	$31
	sw	$0,-0x4b34($28)
	.set	macro
	.set	reorder
	.end	type66_hC
