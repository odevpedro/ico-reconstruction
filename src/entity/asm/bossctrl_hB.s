	.text
	.p2align 3
	.globl	bossctrl_hB
	.ent	bossctrl_hB
bossctrl_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lui	$2,0x6d
	lb	$3,0x35e0($2)
	beqz	$3,loc_00197fe0
	nop	
	j	0x17dd60
	nop	
loc_00197fe0:
	j	0x17dcd8
	nop	
	lui	$4,0x6d
	move	$5,$0
	addiu	$4,$4,0x35e0
	j	0x2641d8
	addiu	$6,$0,0xd50
	nop	
	addiu	$29,$29,-0x60
	lui	$2,0x6d
	.set	macro
	.set	reorder
	.end	bossctrl_hB
