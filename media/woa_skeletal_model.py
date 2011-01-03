#!BPY

"""
Name: 'World of Apo skeletal model (.sm2)'
Blender: 249.2
Group: 'Export'
Tooltip: 'No tips thx'
"""

import Blender
import bpy

def write_obj(filepath):
	out = file(filepath, 'w')
	
	sce = bpy.data.scenes.active
	
	out.write('MODEL\n')
	
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
			
			if bone.parent == None:
				out.write('ROOT\n')
			#out.write('PARENT %s\n' % (bone.parent,) )
			out.write('CHILDREN %i\n' % (len(bone.children),) )
			
			for child in bone.children:
				out.write('\t%s' % (child.name,))
			out.write('\nEND_BONE\n')
	
	
	
	
	ob = sce.objects.active
	mesh = ob.getData(mesh=1)
	for vert in mesh.verts:
		out.write("VERTEX\nCOORDINATE\n\t")
		out.write( '%f %f %f\n' % (vert.co.x, vert.co.y, vert.co.z) )
		
		out.write("NORMAL\n\t")
		out.write( '%f %f %f\n' % (vert.no.x, vert.no.y, vert.no.z) )
		
		out.write("END_VERTEX\n")
	
	for face in mesh.faces:
		if len(face.v) != 3:
			out.write('FACE\n')
			out.write("TRIANGLE\n\t")
			out.write( '%i %i %i' % (face.v[0].index, face.v[1].index, face.v[2].index) );
			out.write("UV_COORDINATES\n")
			out.write("\t%d %f\n" % (face.uv[0][0], face.uv[0][1]))
			out.write("\t%d %f\n" % (face.uv[1][0], face.uv[1][1]))
			out.write("\t%d %f\n" % (face.uv[2][0], face.uv[2][1]))
			out.write('END_FACE\n')
			
			out.write('FACE\n')
			out.write("TRIANGLE\n\t")
			out.write( '%i %i %i' % (face.v[0].index, face.v[2].index, face.v[3].index) );
			out.write("UV_COORDINATES\n")
			out.write("\t%d %f\n" % (face.uv[0][0], face.uv[0][1]))
			out.write("\t%d %f\n" % (face.uv[2][0], face.uv[2][1]))
			out.write("\t%d %f\n" % (face.uv[3][0], face.uv[3][1]))
			out.write('END_FACE\n')
			continue;
		
		out.write('FACE\n')
		
		out.write("TRIANGLE\n\t")
		for vert in face.v:
			out.write( '%i ' % (vert.index,) )
		
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
		
	out.close()

Blender.Window.FileSelector(write_obj, "Export")