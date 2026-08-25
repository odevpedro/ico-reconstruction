	.text
	.p2align 3
	.globl	subGirlBrain_PulledUp
	.ent	subGirlBrain_PulledUp
subGirlBrain_PulledUp:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x170
	sw	$4,0($29)
	sd	$20,0xf0($29)
	sd	$19,0xe0($29)
	addiu	$20,$29,0x10
	sd	$17,0xc0($29)
	move	$4,$20
	sd	$16,0xb0($29)
	lw	$2,0($29)
	sd	$31,0x140($29)
	sd	$30,0x130($29)
	sd	$23,0x120($29)
	sd	$22,0x110($29)
	sd	$21,0x100($29)
	swc1	$f22,0x160($29)
	swc1	$f21,0x158($29)
	swc1	$f20,0x150($29)
	sd	$18,0xd0($29)
	lw	$5,0($29)
	jal	0x0010a498
	lw	$18,0x164($2)
	addiu	$19,$29,0x20
	lw	$5,-0x6e0c($28)
	jal	0x0010a498
	move	$4,$19
	lw	$5,0($29)
	addiu	$17,$29,0x30
	lw	$4,-0x6e0c($28)
	lw	$2,0x164($5)
	lw	$3,0x678($2)
	jal	0x0015ef88
	sw	$0,0x374($3)
	lui	$1,0x4270
	mtc1	$1,$f12
	move	$5,$2
	jal	0x00243b18
	move	$4,$17
	addiu	$16,$29,0x40
	move	$6,$17
	move	$4,$16
	jal	0x00243ad0
	move	$5,$19
	lwc1	$f1,0x44($29)
	move	$4,$16
	lui	$1,0x4248
	mtc1	$1,$f2
	addiu	$5,$29,0x50
	lui	$1,0x447a
	mtc1	$1,$f0
	addiu	$7,$29,0x60
	sub.s	$f1,$f1,$f2
	lwc1	$f3,0x40($29)
	lwc1	$f2,0x48($29)
	lw	$6,-0x6e0c($28)
	add.s	$f0,$f1,$f0
	swc1	$f3,0x50($29)
	mtc1	$0,$f12
	swc1	$f2,0x58($29)
	swc1	$f0,0x54($29)
	jal	0x0014aa28
	swc1	$f1,0x44($29)
	beqz	$2,loc_0016d01c
	lwc1	$f0,0x60($29)
	lwc1	$f1,0x64($29)
	lwc1	$f2,0x68($29)
	lw	$5,-0x6680($28)
	swc1	$f0,0x20($29)
	swc1	$f1,0x24($29)
	beqz	$5,loc_0016d008
	swc1	$f2,0x28($29)
	lw	$3,0xc($5)
	addiu	$2,$0,0x11
	bne	$3,$2,loc_0016d00c
	lwc1	$f0,0x24($29)
	lw	$2,0($29)
	lw	$3,0x164($2)
	lw	$4,0x678($3)
	sw	$5,0x374($4)
loc_0016d008:
	lwc1	$f0,0x24($29)
loc_0016d00c:
	lui	$1,0x4120
	mtc1	$1,$f1
	sub.s	$f0,$f0,$f1
	swc1	$f0,0x24($29)
loc_0016d01c:
	lw	$2,-0x6e0c($28)
	addiu	$4,$0,0x55
	lw	$2,0x164($2)
	lw	$3,0x30($2)
	bne	$3,$4,loc_0016d04c
	nop	
	lwc1	$f0,0x500($2)
	swc1	$f0,0x20($29)
	lwc1	$f1,0x504($2)
	swc1	$f1,0x24($29)
	lwc1	$f0,0x508($2)
	swc1	$f0,0x28($29)
loc_0016d04c:
	lw	$4,0($29)
	move	$5,$20
	move	$6,$19
	move	$7,$0
	move	$8,$0
	jal	0x00200eb0
	move	$9,$0
	bnez	$2,loc_0016d080
	addiu	$2,$18,0x110
loc_0016d070:
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0016d070
	nop	
loc_0016d080:
	lui	$23,0x6b
	mtc1	$0,$f22
	move	$17,$23
	sw	$2,0xa0($29)
	addiu	$30,$29,0x70
	move	$22,$19
	addiu	$21,$29,0x80
	addiu	$20,$29,0x90
loc_0016d0a0:
	lw	$4,0($29)
	jal	0x0015eff8
	nop	
	lwc1	$f0,0($2)
	lw	$4,0($29)
	jal	0x0015eff8
	swc1	$f0,0x70($29)
	lwc1	$f0,4($2)
	lw	$4,0($29)
	jal	0x0015eff8
	swc1	$f0,0x74($29)
	lwc1	$f0,8($2)
	move	$6,$22
	lw	$5,0xa0($29)
	move	$7,$0
	lw	$4,0($29)
	jal	0x00201280
	swc1	$f0,0x78($29)
	lui	$1,0x42c8
	mtc1	$1,$f12
	move	$16,$2
	move	$4,$22
	addiu	$5,$0,0xff
	move	$6,$0
	jal	0x00187328
	move	$7,$0
	bnel	$16,$0,loc_0016d118
	lwc1	$f2,0x3d4($18)
	b	loc_0016d158
	sw	$0,0x33c($18)
loc_0016d118:
	lw	$2,0($29)
	lwc1	$f1,0x3d0($18)
	move	$4,$2
	lwc1	$f0,0x3d8($18)
	lw	$16,0x164($2)
	swc1	$f1,0x110($18)
	swc1	$f2,0x114($18)
	lui	$1,0x3f80
	mtc1	$1,$f21
	lui	$1,0x3f00
	mtc1	$1,$f20
	jal	0x00201d50
	swc1	$f0,0x118($18)
	beql	$2,$0,loc_0016d158
	swc1	$f21,0x33c($16)
	swc1	$f20,0x33c($16)
loc_0016d158:
	move	$4,$19
	jal	0x00194400
	move	$5,$30
	mov.s	$f2,$f0
	lwc1	$f1,0x24($29)
	lwc1	$f0,0x74($29)
	sub.s	$f1,$f1,$f0
	.word	0x46160834
	nop	
	bc1tl	loc_0016d184
	neg.s	$f1,$f1
loc_0016d184:
	lw	$4,-0x6e0c($28)
	addiu	$5,$0,0x55
	lui	$1,0x42c8
	mtc1	$1,$f3
	lw	$2,0x164($4)
	lw	$3,0x30($2)
	bne	$3,$5,loc_0016d1ac
	nop	
	lui	$1,0x4348
	mtc1	$1,$f3
loc_0016d1ac:
	lui	$1,0x41f0
	mtc1	$1,$f0
	.word	0x46001034
	bc1f	loc_0016d2a0
	nop	
	lui	$1,0x4348
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1f	loc_0016d2a0
	move	$4,$21
	sw	$0,0x33c($18)
	addiu	$6,$0,0x2c
	lw	$5,0($29)
	jal	0x00145900
	nop	
	jal	0x0015eff8
	lw	$4,-0x6e0c($28)
	lw	$4,0($29)
	jal	0x0015eff8
	move	$16,$2
	move	$5,$16
	move	$6,$2
	jal	0x001947d0
	move	$4,$20
	move	$4,$21
	jal	0x00194508
	move	$5,$20
	move	$3,$2
	bgez	$3,loc_0016d22c
	move	$2,$3
	negu	$2,$2
loc_0016d22c:
	slti	$2,$2,0x15
	bnel	$2,$0,loc_0016d28c
	ld	$2,0x18($18)
	blez	$3,loc_0016d250
	lw	$2,-0x4220($17)
	addiu	$4,$0,3
	lw	$3,0($29)
	b	loc_0016d25c
	addiu	$5,$0,2
loc_0016d250:
	addiu	$4,$0,3
	lw	$3,0($29)
	addiu	$5,$0,1
loc_0016d25c:
	slt	$2,$4,$2
	bnez	$2,loc_0016d2dc
	addiu	$3,$17,-0x4220
	lwc1	$f0,0x90($29)
	lwc1	$f2,0x94($29)
	lwc1	$f1,0x98($29)
	sw	$4,-0x4220($23)
	swc1	$f1,0x18($3)
	sw	$5,4($3)
	swc1	$f0,0x10($3)
	b	loc_0016d2dc
	swc1	$f2,0x14($3)
loc_0016d28c:
	ori	$3,$0,0x8000
	dsll32	$3,$3,2
	or	$2,$2,$3
	b	loc_0016d2dc
	sd	$2,0x18($18)
loc_0016d2a0:
	.word	0x46031034
	bc1f	loc_0016d2dc
	nop	
	lwc1	$f1,0x33c($18)
	.word	0x46160834
	mtc1	$0,$f0
	bc1tl	loc_0016d2dc
	swc1	$f0,0x33c($18)
	lui	$1,0x3f00
	mtc1	$1,$f0
	.word	0x46010034
	nop	
	bc1fl	loc_0016d2d8
	mov.s	$f0,$f1
loc_0016d2d8:
	swc1	$f0,0x33c($18)
loc_0016d2dc:
	lw	$4,0($29)
	addiu	$5,$0,0x1a
	lui	$1,0xbf80
	mtc1	$1,$f12
	jal	0x0014b270
	move	$6,$0
	lw	$4,0($29)
	jal	0x0015eff8
	nop	
	move	$4,$2
	jal	0x001943c8
	move	$5,$19
	lw	$2,0($29)
	addiu	$4,$0,1
	lw	$3,0x164($2)
	lw	$5,0x678($3)
	jal	0x00203aa0
	swc1	$f0,0x32c($5)
	b	loc_0016d0a0
	nop	
	nop	
	.set	macro
	.set	reorder
	.end	subGirlBrain_PulledUp
