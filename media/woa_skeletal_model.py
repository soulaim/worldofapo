#!BPY

"""
Name: 'World of Apo skeletal model'
Blender: 249.2
Group: 'Export'
Tooltip: 'No tips thx'
"""

import Blender
import bpy

def write_obj(filepath):
	out = file(filepath, 'w')
	sce = bpy.data.scenes.active
	ob = sce.objects.active
	mesh = ob.getData(mesh=1)
	
	for vert in mesh.verts:
		out.write("VERTEX\n\t")
		out.write( '%f %f %f\n' % (vert.co.x, vert.co.y, vert.co.z) )
		
		out.write("NORMAL\n\t")
		out.write( '%f %f %f\n' % (vert.no.x, vert.no.y, vert.no.z) )
		
		out.write("END_VERTEX\n")
	
	for face in mesh.faces:
		out.write('FACE\n')
		
		out.write("TRIANGLE\n\t")
		for vert in face.v:
			out.write( '%i ' % (vert.index + 1) )
		
		out.write("NORMAL\n\t")
		out.write( '%f %f %f\n' % (vert.no.x, vert.no.y, vert.no.z) )
		
		if(mesh.faceUV):
			out.write("UV_COORDINATES\n")
			for uv in mesh.uv:
				out.write('\t%f %f\n' % (uv[0], uv[1]) )
		
		if(mesh.vertexColors):
			out.write("COLORS\n")
			for col in face.col:
				out.write('\t%f %f %f %f\n' % (col.r, col.g, col.b, col.a) )
		
		out.write("AREA\n")
		out.write('\t%f' % face.area)
		
		out.write("CENTER\n")
		out.write('\t%f' % face.cent)
		
		out.write("END_FACE\n")
		
		out.write('\n')
	out.close()

Blender.Window.FileSelector(write_obj, "Export")