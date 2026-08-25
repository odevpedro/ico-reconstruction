	.text
	.p2align 3
	.globl	type22_hA
	.ent	type22_hA
type22_hA:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x30
	sd	$16,0($29)
	sd	$31,0x20($29)
	move	$16,$4
	sd	$17,0x10($29)
	lw	$2,0x15c($16)
	jal	0x0010ecb8
	lw	$17,0x800($2)
	jal	0x00104f20
	nop	
	jal	0x00105278
	nop	
	move	$5,$16
	jal	0x00104140
	move	$4,$2
	jal	0x00105038
	lh	$4,2($17)
	jal	0x00104f48
	lh	$4,0($17)
	jal	0x00105278
	lw	$16,0x10($17)
	lw	$4,0xc($16)
	jal	0x00105f20
	move	$5,$2
	jal	0x0010ec98
	lw	$4,0x10($17)
	ld	$31,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x105268
	addiu	$29,$29,0x30
	nop	
	addiu	$29,$29,-0x40
	sd	$18,0x20($29)
	sd	$16,0($29)
	move	$18,$4
	sd	$31,0x30($29)
	move	$16,$5
	sd	$17,0x10($29)
	lw	$2,0x15c($16)
	jal	0x00105278
	lw	$17,0x800($2)
	move	$5,$16
	jal	0x00104140
	move	$4,$2
	jal	0x00105038
	lh	$4,2($17)
	mtc1	$0,$f13
	lui	$1,0xc248
	mtc1	$1,$f12
	jal	0x00105308
	mov.s	$f14,$f13
	jal	0x00105278
	nop	
	move	$4,$18
	ld	$31,0x30($29)
	ld	$18,0x20($29)
	addiu	$5,$2,0x30
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x105f00
	addiu	$29,$29,0x40
	addiu	$29,$29,-0x130
	sd	$17,0xd0($29)
	sd	$31,0x120($29)
	move	$17,$4
	sd	$21,0x110($29)
	sd	$20,0x100($29)
	sd	$19,0xf0($29)
	sd	$18,0xe0($29)
	sd	$16,0xc0($29)
	lw	$2,0x15c($17)
	lw	$16,0x800($2)
	lw	$3,0x14($16)
	bnel	$3,$0,loc_001bc5e8
	lw	$2,4($16)
	lw	$2,0x18($16)
	move	$3,$2
	addiu	$2,$2,1
	slti	$3,$3,3
	bnez	$3,loc_001bc5e4
	sw	$2,0x18($16)
	move	$4,$29
	jal	0x00104508
	move	$5,$17
	addiu	$4,$29,0x10
	jal	0x00105f00
	move	$5,$29
	lwc1	$f1,4($29)
	move	$4,$29
	lwc1	$f0,0x14($29)
	lui	$1,0x4120
	mtc1	$1,$f2
	lui	$1,0x4348
	mtc1	$1,$f3
	sub.s	$f1,$f1,$f2
	add.s	$f0,$f0,$f3
	swc1	$f1,4($29)
	jal	0x00168538
	swc1	$f0,0x14($29)
	lw	$2,0x94($29)
	beqz	$2,loc_001bc5dc
	move	$4,$17
	jal	0x0019f4e8
	addiu	$5,$29,0x8c
loc_001bc5dc:
	addiu	$2,$0,1
	sw	$2,0x14($16)
loc_001bc5e4:
	lw	$2,4($16)
loc_001bc5e8:
	beqz	$2,loc_001bc718
	lui	$18,0x27
	lw	$4,8($16)
	addiu	$20,$0,0xa
	addiu	$19,$18,0x4ec0
	addiu	$4,$4,1
	addiu	$21,$0,0x3c
	sw	$4,8($16)
	lw	$2,0x4ec0($18)
	lw	$3,4($19)
	mult	$2,$2,$20
	beql	$3,$0,loc_001bc61c
	break	0,7
loc_001bc61c:
	subu	$2,$21,$2
	div	$0,$2,$3
	mflo	$2
	slt	$2,$2,$4
	bnel	$2,$0,loc_001bc6a0
	lh	$2,2($16)
	jal	0x00118a68
	nop	
	lw	$3,0x4ec0($18)
	add.s	$f0,$f0,$f0
	lw	$4,4($19)
	mult	$3,$3,$20
	lui	$1,0x3f80
	mtc1	$1,$f1
	lui	$1,0x4300
	mtc1	$1,$f2
	sub.s	$f0,$f0,$f1
	subu	$3,$21,$3
	div	$0,$3,$4
	mul.s	$f0,$f0,$f2
	cvt.w.s	$f1,$f0
	mfc1	$2,$f1
	sh	$2,0($16)
	beql	$4,$0,loc_001bc680
	break	0,7
loc_001bc680:
	lw	$2,8($16)
	mflo	$3
	bnel	$2,$3,loc_001bc76c
	lw	$3,0x15c($17)
	jal	0x001bc0b0
	move	$4,$17
	b	loc_001bc76c
	lw	$3,0x15c($17)
loc_001bc6a0:
	beqz	$2,loc_001bc6cc
	nop	
	mtc1	$2,$f0
	cvt.s.w	$f0,$f0
	lwc1	$f1,-0x77ac($28)
	sh	$0,0($16)
	mul.s	$f0,$f0,$f1
	cvt.w.s	$f1,$f0
	mfc1	$2,$f1
	b	loc_001bc768
	sh	$2,2($16)
loc_001bc6cc:
	jal	0x0010d530
	move	$4,$29
	move	$4,$29
	jal	0x0010e0b8
	addiu	$5,$0,0x4000
	jal	0x00105278
	nop	
	move	$4,$2
	jal	0x00104140
	move	$5,$17
	jal	0x00105278
	nop	
	addiu	$5,$2,0x30
	move	$4,$0
	jal	0x001e8b48
	move	$6,$29
	sw	$0,8($16)
	b	loc_001bc768
	sw	$0,4($16)
loc_001bc718:
	lh	$2,2($16)
	bltzl	$2,loc_001bc724
	negu	$2,$2
loc_001bc724:
	slti	$2,$2,0x3e9
	beqz	$2,loc_001bc74c
	addiu	$2,$0,1
	lh	$2,0($16)
	bltzl	$2,loc_001bc73c
	negu	$2,$2
loc_001bc73c:
	slti	$2,$2,0x3e9
	bnel	$2,$0,loc_001bc76c
	lw	$3,0x15c($17)
	addiu	$2,$0,1
loc_001bc74c:
	sw	$0,8($16)
	sw	$2,4($16)
	jal	0x001bc0a8
	move	$4,$17
	move	$4,$17
	jal	0x0023d750
	addiu	$5,$0,-1
loc_001bc768:
	lw	$3,0x15c($17)
loc_001bc76c:
	lw	$3,0x800($3)
	lh	$2,2($3)
	bltzl	$2,loc_001bc77c
	negu	$2,$2
loc_001bc77c:
	slti	$2,$2,0xbb9
	beqz	$2,loc_001bc7a0
	move	$4,$0
	lh	$2,0($3)
	bltzl	$2,loc_001bc794
	negu	$2,$2
loc_001bc794:
	slti	$2,$2,0xbb9
	bnez	$2,loc_001bc7a4
	nop	
loc_001bc7a0:
	addiu	$4,$0,1
loc_001bc7a4:
	beqz	$4,loc_001bc7bc
	move	$4,$17
	jal	0x0023d750
	move	$5,$0
	b	loc_001bc7c4
	nop	
loc_001bc7bc:
	jal	0x0023d750
	addiu	$5,$0,1
loc_001bc7c4:
	jal	0x00102858
	move	$4,$17
	ld	$31,0x120($29)
	ld	$21,0x110($29)
	ld	$20,0x100($29)
	ld	$19,0xf0($29)
	ld	$18,0xe0($29)
	ld	$17,0xd0($29)
	ld	$16,0xc0($29)
	jr	$31
	addiu	$29,$29,0x130
	.set	macro
	.set	reorder
	.end	type22_hA
