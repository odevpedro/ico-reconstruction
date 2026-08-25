	.text
	.p2align 3
	.globl	boyAI_sub_159354
	.ent	boyAI_sub_159354
boyAI_sub_159354:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	.word	0x46140034
	nop	
	bc1tl	loc_00159364
	neg.s	$f0,$f0
loc_00159364:
	.word	0x46000834
	nop	
	.word	0x4501001c
	lwc1	$f1,0x54($29)
	lwc1	$f0,0x64($29)
	.word	0x46000834
	.word	0x45010018
	nop	
	jal	0x00190658
	move	$4,$16
	lwc1	$f3,0x64($29)
	lui	$1,0x4348
	mtc1	$1,$f1
	add.s	$f0,$f3,$f0
	lwc1	$f2,0x54($29)
	add.s	$f0,$f0,$f1
	.word	0x46020034
	.word	0x4501000d
	nop	
	lwc1	$f21,0x70($29)
	mtc1	$0,$f0
	.word	0x4600a834
	nop	
	bc1f	loc_001593cc
	lwc1	$f22,0x78($29)
	neg.s	$f21,$f21
loc_001593cc:
	lwc1	$f0,0x60($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_159354
