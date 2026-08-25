	.text
	.p2align 3
	.globl	type62_hB
	.ent	type62_hB
type62_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$2,0x15c($4)
	lw	$2,0($2)
	beqz	$2,loc_001bbebc
	nop	
	lw	$2,0x16c($2)
	beql	$2,$0,loc_001bbebc
	sw	$0,0x16c($4)
loc_001bbebc:
	jr	$31
	nop	
	.set	macro
	.set	reorder
	.end	type62_hB
