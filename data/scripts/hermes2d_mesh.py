# model
newdocument("Hermes2D Mesh", "planar", "general", 0, 1, "disabled", 1, 0, 0, "steadystate", 1.0, 1.0, 0)

# boundaries
addboundary("Inner", "general_value", 0)
addboundary("Outer", "general_value", 10)
addboundary("Border", "general_derivative", 0)

# materials
addmaterial("Material", 0, 1)

# edges
addedge(-2, 0.5, -2, -0.5, 0, "Border")
addedge(1, 0.5, -2, 0.5, 0, "Outer")
addedge(-2, -0.5, 0, -0.5, 0, "Inner")
addedge(0, -0.5, 0, -1, 0, "Inner")
addedge(0, -1, 1, -1, 0, "Border")
addedge(1, -1, 1, 0.5, 0, "Outer")

# labels
addlabel(-0.138626, -0.0639812, 0.1, 0, "Material")

# mesh
zoombestfit()
mesh()

# hermes2d
try:
	from hermes2d import *
	
	msh = Mesh()
	msh.load(meshfilename())
	
	mview = MeshView("Hello world!", 100, 100, 500, 500)
	mview.show(msh, lib="mpl", method="simple", notebook=False)
	
	solve()
	print(solutionfilename())
	
	sln = Solution()
	sln.load(solutionfilename())
	
	view = ScalarView("Solution")
	view.show(sln, lib="mayavi")
except ImportError as err:
	print("Script error: " + err.message)