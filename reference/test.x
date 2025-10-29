
# tac list

0x61f8993eb480	label main
0x61f8993eb4c0	begin
0x61f8993ea7d0	var i
0x61f8993ea870	var maxv
0x61f8993ea8d0	input i
0x61f8993eae10	label L1
0x61f8993eaa70	var t0
0x61f8993eaab0	t0 = (i < 10)
0x61f8993eaf50	ifz t0 goto L2
0x61f8993eab10	output i
0x61f8993eacd0	var t1
0x61f8993ead10	t1 = i + 1
0x61f8993ead50	i = t1
0x61f8993eae50	goto L1
0x61f8993eaf10	label L2
0x61f8993eb190	var t2
0x61f8993eb1d0	actual 8
0x61f8993eb210	actual 5
0x61f8993eb270	t2 = call max
0x61f8993eb2e0	maxv = t2
0x61f8993eb340	output maxv
0x61f8993eb3e0	output L3
0x61f8993eb500	end
0x61f8993ebae0	label max
0x61f8993ebb20	begin
0x61f8993eb600	formal a
0x61f8993eb6a0	formal b
0x61f8993eb7e0	var t3
0x61f8993eb820	t3 = (a > b)
0x61f8993eb9b0	ifz t3 goto L4
0x61f8993eb8b0	return a
0x61f8993eb970	label L4
0x61f8993eba40	return b
0x61f8993ebb60	end
