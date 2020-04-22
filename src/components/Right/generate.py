kernel=[[[1,2,3],[4,5,6],[7,8,9]],[[1,2,3],[4,5,6],[7,8,9]],[[1,2,3],[4,5,6],[7,8,9]]]

for pady in range(0,80,10):
	for padx in range(1,9):
		sum = 0;
		for k in range(0,201,100):
			for i in range(pady,pady+21,10):
				for j in range(padx,padx+3):
					sum=sum+((k+i+j)*kernel[k/100][(i-pady)/10][(j-padx)])
		# 			print str(k+i+j), 
		# 		print ""
		# 	print ""
		# print ""

		print("pad[" + str(pady/10) + "][" + str(padx-1) + "] = " + str(sum))
