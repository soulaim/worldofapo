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
	
	for armadon in bpy.data.armatures:
		for bone in armadon.bones.values():
			
			out.write('BONE %s\n' % (bone.name,) )
			out.write('START_POS\n\t')
			out.write('%f %f %f\n' % (bone.head['ARMATURESPACE'].x, bone.head['ARMATURESPACE'].y, bone.head['ARMATURESPACE'].z) )
			
			out.write('END_POS\n\t')
			out.write('%f %f %f\n' % (bone.tail['ARMATURESPACE'].x, bone.tail['ARMATURESPACE'].y, bone.tail['ARMATURESPACE'].z) )
			
			#out.write('DEFORM_DIST\n\t')
			#out.write('%f\n' % (bone.deformDist,) )
			
			out.write('TAIL_RADIUS\n\t')
			out.write('%f\n' % (bone.tailRadius,) )
			
			out.write('HEAD_RADIUS\n\t')
			out.write('%f\n' % (bone.headRadius,) )
			
			out.write('BONE_WEIGHT\n\t')
			out.write('%f\n' % (bone.weight,) )
			
			#out.write('PARENT %s\n' % (bone.parent.name,) )
			out.write('CHILDREN %i\n' % (len(bone.children),) )
			
			for child in bone.children:
				out.write('\t%s' % (child.name,))
			out.write('\nEND_BONE\n')
	
	for mesh in bpy.data.meshes:
		out.write("MESH\n")
		for vert in mesh.verts:
			out.write("VERTEX\nCOORDINATE\n\t")
			out.write( '%f %f %f\n' % (vert.co.x, vert.co.y, vert.co.z) )
			
			out.write("NORMAL\n\t")
			out.write( '%f %f %f\n' % (vert.no.x, vert.no.y, vert.no.z) )
			
			out.write("END_VERTEX\n")
		
		for face in mesh.faces:
			out.write('FACE\n')
			
			out.write("TRIANGLE\n\t")
			for vert in face.v:
				out.write( '%i ' % (vert.index + 1,) )
			
			out.write("\nNORMAL\n\t")
			out.write( '%f %f %f\n' % (vert.no.x, vert.no.y, vert.no.z) )
			
			if(mesh.faceUV):
				out.write("UV_COORDINATES\n")
				for uv in face.uv:
					out.write('\t%f %f\n' % (uv[0], uv[1]) )
			
			if(mesh.vertexColors):
				out.write("COLORS\n")
				for col in face.col:
					out.write('\t%f %f %f %f\n' % (col.r, col.g, col.b, col.a) )
			
			out.write("AREA\n")
			out.write('\t%f\n' % (face.area,) )
			
			#out.write("CENTER\n")
			#out.write('\t%fblarg \n\n' % (face.cent,) )
			
			out.write("END_FACE\n")
		out.write('END_MESH\n')
	out.close()

Blender.Window.FileSelector(write_obj, "Export")