import time

for num in range(10):
	time.sleep(2)
	with file('ber.dat', 'r') as original:
		data = original.read()
	with file('ber.dat', 'w') as modified:
		modified.write("12345 0.00000"+str(num+1)+"\n" + data)
