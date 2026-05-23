	.text
	.p2align 3
	.globl	eBrainInit
	.ent	eBrainInit
eBrainInit:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$2,0x6d
	sw	$0,-0x6178($28)
	addiu	$2,$2,0x710
	sw	$0,-0x617c($28)
	sw	$0,-0x4b44($28)
	addiu	$2,$2,0x37c
	sw	$0,-0x4b40($28)
	addiu	$3,$0,0x1f
loc_001918c8:
	sw	$0,0($2)
	addiu	$3,$3,-1
	addiu	$2,$2,-0x1c
	nop	
	nop	
	bgez	$3,loc_001918c8
	nop	
	jr	$31
	nop	
	nop	
	addiu	$29,$29,-0x10
	addiu	$2,$0,4
	.set	macro
	.set	reorder
	.end	eBrainInit
