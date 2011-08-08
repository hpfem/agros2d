# model
newdocument(name="Scripting test", type="planar", physicfield="electrostatic",
			analysistype="steadystate",
			numberofrefinements=1, polynomialorder=1)
# variables
x = 0.025;
y = 0.05;
dx = 0.025;
r = 0.025;

# boundaries
addboundary("Dirichlet", "electrostatic_potential", {"V" : 0})

# materials
addmaterial("Material", {"epsr" : 1, "rho" : 0})

# edges
addedge(0, 0, x, 0, boundary="Dirichlet")
addedge(x, 0, x, y, boundary="Dirichlet")
addedge(x, y, 0, y, boundary="Dirichlet")
addedge(0, y, 0, 0, boundary="Dirichlet")

# labels
addlabel(x/2, y/2, material="Material")

# moveselection, selectedge, selectlabel
selectedge([1])
moveselection(dx, 0, False)
selectlabel([0])
moveselection(dx/2, 0, False)
zoombestfit()
solve()
volume = volumeintegral([0])
testMoveSelection1 = test("moveselection()", volume["S"], (x+dx)*y)

selectedge([1])
moveselection(-dx, 0, False)
selectlabel([0])
moveselection(-dx/2, 0, False)
zoombestfit()
solve()
volume = volumeintegral([0])
testMoveSelection2 = test("moveselection()", volume["S"], x*y)

# scaleselection, selectnode
selectnode([0, 1, 2, 3])
scaleselection(0, 0, 2, False)
selectlabel([0])
scaleselection(0, 0, 2, False)
zoombestfit()
solve()
volume = volumeintegral([0])
testScaleSelection1 = test("scaleselection()", volume["S"], (2*x)*(2*y))

selectnode([0, 1, 2, 3])
scaleselection(0, 0, 1.0/2, False)
selectlabel([0])
scaleselection(0, 0, 1.0/2, False)
zoombestfit()
solve()
volume = volumeintegral([0])
testScaleSelection2 = test("scaleselection()", volume["S"], x*y)

# rotateselection, mode, slectall
mode("node")
selectall()
rotateselection(0, 0, 90, False)
mode("label")
selectall()
rotateselection(0, 0, 90, False)
zoombestfit()
solve()
volume = volumeintegral([0])
testRotateSelection1 = test("rotateselection()", volume["S"], x*y)

mode("edge")
selectall()
rotateselection(0, 0, -90, False)
mode("label")
selectall()
rotateselection(0, 0, -90, False)
zoombestfit()
solve()
volume = volumeintegral([0])
testRotateSelection2 = test("rotateselection()", volume["S"], x*y)

# addrect
selectnode([0, 1, 2, 3])
deleteselection()
addrect(0, 0, x, y, boundary="Dirichlet")
zoombestfit()
solve()
volume = volumeintegral([0])
testAddRect = test("addrect()", volume["S"], x*y)

# addcircle, deleteselection
selectnode([0, 1, 2, 3])
deleteselection()
selectlabel([0])
deleteselection()
addcircle(0, 0, radius=r, boundary="Dirichlet")
addlabel(0, 0, material="Material")
zoombestfit()
solve()
volume = volumeintegral([0])
testAddCircle = test("addcircle()", volume["S"], pi*(r**2))

# addsemicircle
selectnode([0, 1, 2, 3])
deleteselection()
selectlabel([0])
deleteselection()
addsemicircle(0, 0, radius=r, boundary="Dirichlet")
addlabel(r/2, 0, material="Material")
zoombestfit()
solve()
volume = volumeintegral([0])
testAddSemiCircle = test("addsemicircle()", volume["S"], (pi*(r**2))/2)

# savedocument, opendocument
import tempfile, os
fn = tempfile.gettempdir() + "/test.a2d"
savedocument(fn)
opendocument(fn)
solve()
volume = volumeintegral([0])
testSaveDocument = test("addsemicircle()", volume["S"], (pi*(r**2))/2)
closedocument()
os.remove(fn)

print("Test: Scripting: " + str(testMoveSelection1 and testMoveSelection2 and testScaleSelection1 and testScaleSelection2 and testRotateSelection1 and testRotateSelection2 and testAddRect and testAddCircle and testAddSemiCircle and testSaveDocument))

# modifyboundary(), modifymaterial()
newdocument(name="Scripting test", type="planar", physicfield="electrostatic", analysistype="steadystate", numberofrefinements=1, polynomialorder=1)
addboundary("B", "electrostatic_potential", {"V" : 0})
modifyboundary("B", "electrostatic_potential", {"V" : 2})
modifyboundary("B", "electrostatic_surface_charge_density", {"sigma" : 1})
addmaterial("M", {"epsr" : 1, "rho" : 0})
modifymaterial("M", {"epsr" : 2, "rho" : 1})
