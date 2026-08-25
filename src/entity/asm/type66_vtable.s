	.text
	.p2align 3
	.globl	type66_vtable
	.ent	type66_vtable
type66_vtable:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x20
	lw	$2,-0x4b3c($28)
	sd	$31,0x10($29)
	blez	$2,loc_00191cdc
	sd	$16,0($29)
	lui	$16,0x6d
	jal	0x00118a68
	addiu	$16,$16,0xb90
	lui	$1,0x4120
	mtc1	$1,$f1
	lw	$4,-0x4b3c($28)
	mul.s	$f0,$f0,$f1
	beql	$4,$0,loc_00191cb8
	break	0,7
loc_00191cb8:
	cvt.w.s	$f1,$f0
	mfc1	$3,$f1
	div	$0,$3,$4
	mfhi	$2
	sll	$2,$2,2
	addu	$2,$2,$16
	lw	$3,0($2)
	b	loc_00191ce4
	sw	$3,-0x4b38($28)
loc_00191cdc:
	addiu	$2,$0,-1
	sw	$2,-0x4b38($28)
loc_00191ce4:
	lw	$2,-0x4b34($28)
	ld	$31,0x10($29)
	addiu	$2,$2,1
	ld	$16,0($29)
	sw	$0,-0x4b3c($28)
	sw	$2,-0x4b34($28)
	jr	$31
	addiu	$29,$29,0x20
	.set	macro
	.set	reorder
	.end	type66_vtable
