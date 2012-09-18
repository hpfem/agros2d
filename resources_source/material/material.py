def Zinc_tc_0(t) :
	# C.Y. Ho, R.W. Powell and P.E. Liley, J. Phys. Chem. Ref. Data, 
	# v1, p279 (1972)
	# well-annealed with residual resistivity of 0.00128 uohm-cm; error 
	# is 3% near RT, 10-15% at others
	# data is in units of W/(m-K)
	# t must be in degrees Kelvin for these equations
	if t >= 0.0 and t < 9.0 :
		return 2.706808e-01*t*t*t*t*t -2.655161e+00*t*t*t*t -2.718274e+01*t*t*t +1.389875e+02*t*t +1.750994e+03*t
	elif t >= 9.0 and t < 30.0 :
		return -4.348847e-03*t*t*t*t*t +5.035428e-01*t*t*t*t -2.348013e+01*t*t*t +5.571501e+02*t*t -6.855608e+03*t +3.644601e+04
	elif t >= 30.0 and t < 100.0 :
		return -1.029048e-06*t*t*t*t*t +3.785208e-04*t*t*t*t -5.554990e-02*t*t*t +4.111120e+00*t*t -1.566486e+02*t +2.658764e+03
	elif t >= 100.0 and t <= 693.0 :
		return -5.778175e-05*t*t +1.274300e-02*t +1.168392e+02
	else :
		return 1.000000e+100

def Zinc_spec_heat_0(t) :
	# B.J. McBride, S. Gordon and M.A. Reno, NASA Technical Paper 3287 
	# (1993)
	# data is in units of J/(kg-K)
	# t must be in degrees Kelvin for these equations
	if t >= 100.0 and t < 320.0 :
		return -1.258998e-08*t*t*t*t +1.945167e-05*t*t*t -1.099511e-02*t*t +2.827810e+00*t +1.066220e+02
	elif t >= 320.0 and t <= 693.0 :
		return 7.366442e-07*t*t*t -7.026091e-04*t*t +3.410779e-01*t +3.297327e+02
	else :
		return 1.000000e+100

def Zinc_density_0(t) :
	# R.W. Meyerhoff and J.F. Smith, J. Applied Physics, v33, p219 
	# (1962) and E.A. Owen and E.L. Yates, Phil. Mag. S7, v17, p113 
	# (1934)
	# data is in units of kg/cm^3
	# t must be in degrees Kelvin for these equations
	if t >= 0.0 and t < 300.0 :
		return 1000.0 * (-1.986628e-11*t*t*t*t +1.518314e-08*t*t*t -4.442600e-06*t*t +1.347873e-05*t +7.280967e+00)
	elif t >= 300.0 and t <= 890.0 :
		return 1000.0 * (-1.783429e-07*t*t -5.059331e-04*t +7.302000e+00)
	else :
		return 1000.0 * 1.000000e+100

def Copper_tc_0_0(t) :
	# N.J. Simon, E.S. Drexler, R.P. Reed, NIST Monograph 177, Properties 
	# of Copper and Copper Alloys at Cryogenic Temperatures (1992) 
	# and C.Y. Ho, R.W. Powell and P.E. Liley, J. Phys. Chem. Ref. 
	# Data, v1, p279 (1972)
	# data valid for copper with a residual resistivity ratio -269C/0C 
	# of 30
	# data is in units of W/(m-K)
	# t must be in degrees Kelvin for these equations
	if t >= 1.0 and t < 40.0 :
		return 6.997990e-04*t*t*t*t -7.168113e-02*t*t*t +1.387207e+00*t*t +3.666487e+01*t +1.255868e+01
	elif t >= 40.0 and t < 70.0 :
		return -9.504397e-04*t*t*t +3.738471e-01*t*t -4.525448e+01*t +2.174919e+03
	elif t >= 70.0 and t < 100.0 :
		return 9.226190e-06*t*t*t*t -4.470238e-03*t*t*t +8.176488e-01*t*t -6.753869e+01*t +2.545870e+03
	elif t >= 100.0 and t < 300.0 :
		return -1.266667e-05*t*t*t +8.971429e-03*t*t -2.116905e+00*t +5.554000e+02
	elif t >= 300.0 and t <= 1358.0 :
		return -2.641980e-13*t*t*t*t*t +1.062220e-09*t*t*t*t -1.570451e-06*t*t*t +1.013916e-03*t*t -3.133575e-01*t +4.237411e+02
	else :
		return 1.000000e+100

def Copper_spec_heat_0(t) :
	# G.K. White and S.J. Collocott, J. Phys. Chem. Ref. Data, vol 
	# 13, no 4, p1251 (1984)
	# error less than 2%
	# data is in units of J/(kg-K)
	# t must be in degrees Kelvin for these equations
	if t >= 1.0 and t < 18.0 :
		return 2.246429e-05*t*t*t*t +2.847033e-04*t*t*t +3.441219e-03*t*t +1.044570e-03*t +8.168055e-03
	elif t >= 18.0 and t < 60.0 :
		return 3.010206e-07*t*t*t*t -1.048364e-03*t*t*t +1.540539e-01*t*t -3.767169e+00*t +2.905972e+01
	elif t >= 60.0 and t < 300.0 :
		return -1.357031e-07*t*t*t*t +1.291112e-04*t*t*t -4.732108e-02*t*t +8.236392e+00*t -2.152814e+02
	elif t >= 300.0 and t <= 1300.0 :
		return 1.140747e-10*t*t*t*t -1.971221e-07*t*t*t +5.535252e-05*t*t +1.338348e-01*t +3.427640e+02
	else :
		return 1.000000e+100

def Copper_density_0(t) :
	# N.J. Simon, E.S. Drexler, R.P. Reed, NIST Monograph 177, Properties 
	# of Copper and Copper Alloys at Cryogenic Temperatures (1992) 
	# and T.A. Hahn, J. Applied Physics, v41, p5096 (1970) and G.K. 
	# White, R.B. Roberts, High Temperatures-High Pressures, v12, p311 
	# (1980); calculated from the linear expansion
	# 8% error
	# data is in units of kg/m^3
	# t must be in degrees Kelvin for these equations
	if t >= 4.0 and t < 90.0 :
		return -8.227902e-06*t*t*t -4.310034e-04*t*t +1.936185e-03*t +9.028155e+03
	elif t >= 90.0 and t < 250.0 :
		return 1.736657e-06*t*t*t -1.406238e-03*t*t -5.885933e-02*t +9.034264e+03
	elif t >= 250.0 and t <= 1250.0 :
		return -8.730517e-05*t*t -3.926111e-01*t +9.062604e+03
	else :
		return 1.000000e+100


Tmin = 4.0
Tmax = 1250.0

fce = Copper_density_0

N = 50
xK = ""
xC = ""
yK = ""
for i in range(0, N):
    xK = xK + "," + str(Tmin + i * (Tmax-Tmin)/(N-1))
    xC = xC + "," + str(Tmin + i * (Tmax-Tmin)/(N-1) - 273.16)
    yK = yK + "," + str(fce(Tmin + i * (Tmax-Tmin)/(N-1)))
    
print("T (K)", xK)
print("\n\n")
print("T (deg.)", xC)
print("\n\n")
print("fce (K)", yK)


