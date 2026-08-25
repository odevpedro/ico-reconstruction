	.text
	.p2align 3
	.globl	boyAI_sub_153568
	.ent	boyAI_sub_153568
boyAI_sub_153568:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	.word	0x00c43018
	lw	$5,4($2)
	lw	$2,0x164($18)
	move	$4,$18
	beql	$5,$0,loc_00153580
	break	0,7
loc_00153580:
	lw	$7,0x670($2)
	subu	$3,$3,$6
	div	$0,$3,$5
	mflo	$3
	mtc1	$3,$f1
	cvt.s.w	$f1,$f1
	mul.s	$f0,$f0,$f1
	nop	
	nop	
	div.s	$f0,$f0,$f2
	cvt.w.s	$f1,$f0
	swc1	$f1,0x254($7)
	jal	0x0014b580
	nop	
	sw	$0,0x170($19)
	move	$4,$18
	sw	$0,0x174($19)
	jal	0x0014b1d0
	sw	$0,-0x6624($28)
	jal	0x0014b260
	move	$4,$18
	jal	0x00203aa0
	addiu	$4,$0,1
	sw	$18,-0x6df0($28)
	jal	0x0014b358
	addiu	$4,$0,0x11
	sw	$0,0x44($19)
	swc1	$f0,0x1d0($19)
	lw	$4,-0x5520($21)
	beql	$4,$0,loc_00153650
	addiu	$17,$21,-0x5520
	jal	0x0013eae8
	nop	
	move	$16,$2
	beqz	$16,loc_00153648
	move	$4,$16
	move	$5,$18
	jal	0x001f3f58
	addiu	$6,$0,0x16
	sw	$16,0x140($19)
	ori	$3,$0,0x8000
	dsll	$3,$3,0x13
	ld	$2,8($17)
	and	$2,$2,$3
	beqz	$2,loc_00153650
	addiu	$17,$21,-0x5520
	jal	0x001f4150
	move	$4,$16
	b	loc_00153650
	addiu	$17,$21,-0x5520
loc_00153648:
	sw	$0,-0x5520($21)
	addiu	$17,$21,-0x5520
loc_00153650:
	lw	$4,4($17)
	.word	0x1080000f
	addiu	$20,$21,-0x5520
	jal	0x0013eae8
	nop	
	move	$16,$2
	.word	0x12000006
	move	$4,$18
	jal	0x0015bcc8
	addiu	$5,$0,0x35
	.set	macro
	.set	reorder
	.end	boyAI_sub_153568
