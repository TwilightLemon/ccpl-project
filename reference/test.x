
# tac list

0x5592e72c8ff0	label main
0x5592e72c9030	begin
0x5592e72c87d0	var i
0x5592e72c8830	input i
0x5592e72c8d70	label L1
0x5592e72c89d0	var t0
0x5592e72c8a10	t0 = (i < 10)
0x5592e72c8eb0	ifz t0 goto L2
0x5592e72c8a70	output i
0x5592e72c8c30	var t1
0x5592e72c8c70	t1 = i + 1
0x5592e72c8cb0	i = t1
0x5592e72c8db0	goto L1
0x5592e72c8e70	label L2
0x5592e72c8f50	output L3
0x5592e72c9070	end
