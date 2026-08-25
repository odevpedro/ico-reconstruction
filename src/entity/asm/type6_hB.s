	.text
	.p2align 3
	.globl	type6_hB
	.ent	type6_hB
type6_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	move	$5,$4
	lw	$3,0x15c($5)
	sw	$0,0x2b0($3)
	lw	$2,0x15c($5)
	sw	$0,0x310($2)
	lw	$3,0x15c($5)
	sw	$0,0x3b8($3)
	lw	$2,0x15c($5)
	j	0x1e3fc8
	sw	$0,0x3bc($2)
	lw	$2,0x15c($4)
	mtc1	$0,$f0
	lw	$2,0x840($2)
	.word	0x46006034
	move	$3,$2
	bc1f	loc_001ce738
	swc1	$f12,0x30($2)
	swc1	$f0,0x30($2)
loc_001ce738:
	lwc1	$f0,0x30($3)
	lui	$1,0x3f80
	mtc1	$1,$f1
	.word	0x46000834
	nop	
	bc1tl	loc_001ce754
	swc1	$f1,0x30($3)
loc_001ce754:
	jr	$31
	nop	
	.set	macro
	.set	reorder
	.end	type6_hB
