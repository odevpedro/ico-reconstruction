	.text
	.p2align 3
	.globl	type49_hA
	.ent	type49_hA
type49_hA:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x20
	sd	$16,0($29)
	sd	$31,0x10($29)
	move	$16,$4
	lw	$3,0x15c($16)
	sw	$16,-0x5474($28)
	lw	$2,0x8c($3)
	beqz	$2,loc_001e08a8
	sw	$2,-0x5478($28)
	jal	0x0010ecb8
	nop	
	lw	$2,-0x5c28($28)
	beqz	$2,loc_001e08ac
	ld	$31,0x10($29)
	move	$4,$16
	ld	$16,0($29)
	j	0x1e0808
	addiu	$29,$29,0x20
loc_001e08a8:
	ld	$31,0x10($29)
loc_001e08ac:
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x20
	.set	macro
	.set	reorder
	.end	type49_hA
