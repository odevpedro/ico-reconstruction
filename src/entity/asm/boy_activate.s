	.text
	.p2align 3
	.globl	boy_activate
	.ent	boy_activate
boy_activate:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x30
	sd	$16,0($29)
	sd	$31,0x20($29)
	move	$16,$4
	sd	$17,0x10($29)
	lw	$2,0x15c($16)
	jal	0x00109f28
	lw	$17,0x800($2)
	addiu	$3,$0,1
	sw	$0,0x14($17)
	.set	macro
	.set	reorder
	.end	boy_activate
