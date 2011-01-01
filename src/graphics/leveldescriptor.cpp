

#include "graphics.h"
#include "texturehandler.h"
#include "texturecoordinate.h"
#include "level.h"
#include "frustum/frustumr.h"

#include <numeric>
#include <iostream>

using namespace std;

extern int TRIANGLES_DRAWN_THIS_FRAME;


LevelDescriptor::LevelDescriptor()
{
	debugMode = false;
	level = 0;
	buffers_loaded = false;

	for(size_t i = 0; i < BUFFERS; ++i)
	{
		locations[i] = -1;
	}
	
	strVals["TERRAIN_LOW"] = "";
	strVals["TERRAIN_MID"] = "";
	strVals["TERRAIN_HIGH"] = "";
}

void LevelDescriptor::setLevel(Level* lvl)
{
	level = lvl;
}

void LevelDescriptor::drawFR(size_t lights, Shaders& shaders) const
{
	drawLevel(*level, lights, shaders);
}

void LevelDescriptor::drawDeferred(Shaders& shaders) const
{
	drawLevelDeferred(*level, shaders);
}

void LevelDescriptor::drawLevel(const Level& lvl, size_t light_count, Shaders& shaders) const
{
	// Draw terrain with forward rendering, apply lights right away.
	for(size_t i = 0; i < light_count; i += 4)
	{
		int pass = int(i);
		if(operator[]("DRAW_LEVEL") == 2)
		{
			// drawLevelFR_new(lvl, pass);
		}
		else
		{
			drawLevelFR(lvl, pass, shaders);
		}
	}
	
	if(operator[]("DRAW_LEVEL") == 2)
	{
		// drawLevelFR_new(lvl, -1);
		cerr << "WARNING" << endl;
	}
	else
	{
		drawLevelFR(lvl, -1, shaders);
	}
}



void LevelDescriptor::world_tick(FrustumR& frustum)
{
	level_triangles.clear();
	
	Location pos;
	level->splitBTT(pos, frustum);
	level->getLevelTriangles(level_triangles);
}

void LevelDescriptor::drawDebugLevelNormals() const
{
	const Level& lvl = *level;

	Vec3 points[3];
	
	glBegin(GL_LINES);
	for(size_t k=0; k<level_triangles.size(); k++)
	{
		const BTT_Triangle& tri = level_triangles[k];
		for(size_t i = 0; i < 3; ++i)
		{
			points[i].x = tri.points[i].x * Level::BLOCK_SIZE;
			points[i].z = tri.points[i].z * Level::BLOCK_SIZE;
			points[i].y = lvl.getVertexHeight(tri.points[i].x, tri.points[i].z).getFloat();
		}
		
		Location n;
		
		n = lvl.getNormal(tri.points[0].x, tri.points[0].z) * 10;
		Location start;
		start.x = FixedPoint(int(points[0].x));
		start.y = lvl.getVertexHeight(tri.points[0].x, tri.points[0].z);
		start.z = FixedPoint(int(points[0].z));
		
		Location end = start + n;
		
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(start.x.getFloat(), start.y.getFloat(), start.z.getFloat());
		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(end.x.getFloat(), end.y.getFloat(), end.z.getFloat());
	}
	glEnd();
}


void LevelDescriptor::drawDebugHeightDots(const Vec3& location) const
{
	const Level& lvl = *level;

	TextureHandler::getSingleton().bindTexture(0, "");
	
	Vec3 points[3];
	
	// Draw triangles with lines.
	glPointSize(1.0f);
	glColor3f(0,0,1);
	glBegin(GL_LINES);
	for(size_t k=0; k<level_triangles.size(); k++)
	{
		const BTT_Triangle& tri = level_triangles[k];
		for(size_t i = 0; i < 3; ++i)
		{
			points[i].x = tri.points[i].x * Level::BLOCK_SIZE;
			points[i].z = tri.points[i].z * Level::BLOCK_SIZE;
			points[i].y = lvl.getVertexHeight(tri.points[i].x, tri.points[i].z).getFloat();
		}
		
		// TODO: this could also be done by loading a geometry shader that transforms a polygon into three lines.
		for(size_t i = 0; i < 3; ++i)
		{
			glVertex3f(points[i].x, points[i].y, points[i].z);
			int next = (i+1+3) % 3;
			glVertex3f(points[next].x, points[next].y, points[next].z);
		}
	}
	glEnd();
	
	// Draw heights with dots.
	glColor3f(1,0,0);
	glPointSize(2.0f);
	glBegin(GL_POINTS);
	for(FixedPoint x; x < lvl.max_x(); x += 2)
	{
		for(FixedPoint z; z < lvl.max_z(); z += 2)
		{
			Vec3 v(x.getFloat(), 0, z.getFloat());
			v.y = lvl.getHeight(x,z).getFloat();
			if((location - v).lengthSquared() < 100.0f * 100.0f)
			{
				glVertex3f(v.x, v.y, v.z);
			}
		}
	}
	glEnd();
	glColor3f(0,1,0);
	glPointSize(4.0f);
	glBegin(GL_POINTS);
	
	for(int x = 0; x < lvl.max_block_x(); ++x)
	{
		for(int z = 0; z < lvl.max_block_z(); ++z)
		{
			Vec3 v(x * Level::BLOCK_SIZE, 0, z * Level::BLOCK_SIZE);
			v.y = lvl.getVertexHeight(x,z).getFloat();
			if((location - v).lengthSquared() < 200.0f * 200.0f)
			{
				glVertex3f(v.x, v.y, v.z);
			}
		}
	}
	
	glEnd();
}

void LevelDescriptor::unload()
{
	if(buffers_loaded)
	{
		vertices.clear();
		normals.clear();
		texture_coordinates1.clear();
		texture_coordinates2.clear();
//		texture_coordinates3.clear();
		indices.clear();

		glDeleteBuffers(BUFFERS, locations);
		for(size_t i = 0; i < BUFFERS; ++i)
		{
			locations[i] = -1;
		}
		buffers_loaded = false;
	}
}

void LevelDescriptor::preload()
{
	assert(!buffers_loaded);

	const Level& lvl = *level;

	size_t height = lvl.pointheight_info.size();
	size_t width = lvl.pointheight_info[0].size();
	assert(height*width > 0);
	
	vertices.reserve(height * width);
	normals.reserve(height * width);
	for(size_t x = 0; x < height; ++x)
	{
		for(size_t z = 0; z < width; ++z)
		{
			Vec3 point(x*8, lvl.getVertexHeight(x, z).getFloat(), z*8);
			vertices.push_back(point);
			
			Location normal = lvl.getNormal(x, z);
			normals.push_back(Vec3(normal.x.getFloat(), normal.y.getFloat(), normal.z.getFloat()));
			
			 // TODO: These coordinates are like :G
			const int divisions = 25;
//				TextureCoordinate tc1 = { float(x % (height/divisions)) / (height/divisions), float(z % (width/divisions)) / (width/divisions) };
			TextureCoordinate tc1 = TextureCoordinate( float(x) / (height/divisions), float(z) / (width/divisions) );
			texture_coordinates1.push_back(tc1);
			texture_coordinates2.push_back(tc1);
		}
	}

	int buffer = 0;
	glGenBuffers(BUFFERS, locations);
	
	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vec3), &vertices[0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ARRAY_BUFFER, texture_coordinates1.size() * sizeof(TextureCoordinate), &texture_coordinates1[0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ARRAY_BUFFER, texture_coordinates2.size() * sizeof(TextureCoordinate), &texture_coordinates2[0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(Vec3), &normals[0], GL_STATIC_DRAW);
	
	assert(buffer <= BUFFERS);

	buffers_loaded = true;
}


void LevelDescriptor::drawLevelFR(const Level& lvl, int pass, Shaders& shaders) const
{
	if(pass == -1)
	{
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);
		
		TextureHandler::getSingleton().bindTexture(2, "");
		TextureHandler::getSingleton().bindTexture(1, "");
		TextureHandler::getSingleton().bindTexture(0, "");
		return;
	}

	glUseProgram(shaders["level_program"]);

//	size_t height = lvl.pointheight_info.size();
	size_t width = lvl.pointheight_info[0].size();
	
	assert(buffers_loaded);

	// Load dynamic indices.
	if(pass == 0)
	{
		indices.clear();
		for(size_t k = 0; k < level_triangles.size(); ++k)
		{
			Vec3 points[3];
			const BTT_Triangle& tri = level_triangles[k];
			for(size_t i = 0; i < 3; ++i)
			{
				points[2-i].x = tri.points[i].x * Level::BLOCK_SIZE;
				points[2-i].z = tri.points[i].z * Level::BLOCK_SIZE;
				points[2-i].y = lvl.getVertexHeight(tri.points[i].x, tri.points[i].z).getFloat();
				
				indices.push_back( tri.points[2-i].x * width + tri.points[2-i].z  );
			}
		}
		
		if(debugMode)
		{
			TextureHandler::getSingleton().bindTexture(0, "chessboard");
			TextureHandler::getSingleton().bindTexture(1, "chessboard");
			TextureHandler::getSingleton().bindTexture(2, "chessboard");
		}
		else
		{
			TextureHandler::getSingleton().bindTexture(0, "grass");
			TextureHandler::getSingleton().bindTexture(1, "hill");
			TextureHandler::getSingleton().bindTexture(2, "highground");
		}
		
		if(debugMode)
		{
			glUniform4f(shaders.uniform("lvl_ambientLight"), 0.4f, 0.4f, 0.4f, 1.f);
		}
		else
		{
			float r = operator[]("AMBIENT_RED")   / 255.0f;
			float g = operator[]("AMBIENT_GREEN") / 255.0f;
			float b = operator[]("AMBIENT_BLUE")  / 255.0f;
			glUniform4f(shaders.uniform("lvl_ambientLight"), r, g, b, 1.0f);
		}

		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);
	}
	else
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDepthFunc(GL_EQUAL);
		glDepthMask(GL_FALSE);
		
		glUniform4f(shaders.uniform("lvl_ambientLight"), 0.f, 0.f, 0.f, 1.0f);
	}
	
	// TODO: check what happens when the number of lights is not 0 mod 4.
	glUniform4f(shaders.uniform("lvl_activeLights"), float(pass), float(pass+1), float(pass+2), float(pass+3));

	drawBuffers();
	
	glUseProgram(0);
}

void LevelDescriptor::drawBuffers() const
{
	assert(texture_coordinates1.size() == vertices.size());
	assert(texture_coordinates2.size() == vertices.size());
	assert(normals.size() == vertices.size());
	
	int buffer = 0;
	// Bind static data and send dynamic data to graphics card.
	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	glEnableClientState(GL_VERTEX_ARRAY);
	
	glClientActiveTexture(GL_TEXTURE1);
	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glTexCoordPointer(2, GL_FLOAT, 0, 0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glClientActiveTexture(GL_TEXTURE0);
	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glTexCoordPointer(2, GL_FLOAT, 0, 0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glNormalPointer(GL_FLOAT, 0, 0);
	glEnableClientState(GL_NORMAL_ARRAY);
	
	buffer++;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), &indices[0], GL_STREAM_DRAW);
	
	assert(buffer == BUFFERS);
	
	// Draw sent data.
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	TRIANGLES_DRAWN_THIS_FRAME += level_triangles.size();
}

/*
#include "level_square_part_block.h"

void fill_level_part(VisualLevelPart& part, size_t part_min_x, size_t part_max_x, size_t part_min_z, size_t part_max_z, size_t max_x, size_t max_z, const Level& lvl)
{
	assert(part.vertices.empty());
	assert(part.indices.empty());
	for(size_t x = part_min_x; x < part_max_x; ++x)
	{
		for(size_t z = part_min_z; z < part_max_z; ++z)
		{
			Vec3 point(x * Level::BLOCK_SIZE, lvl.getVertexHeight(x, z).getFloat(), z * Level::BLOCK_SIZE);
			part.vertices.push_back(point);
			
			Location normal = lvl.getNormal(x, z);
			part.normals.push_back(Vec3(normal.x.getFloat(), normal.y.getFloat(), normal.z.getFloat()));
			
			 // TODO: These coordinates are like :G
			const int divisions = 25;
			TextureCoordinate tc1 = TextureCoordinate( float(x) / (max_x/divisions), float(z) / (max_z/divisions) );
			part.texture_coordinates1.push_back(tc1);
			part.texture_coordinates2.push_back(tc1);

			const size_t row_length = part_max_z - part_min_z;
			if(x < part_max_x - 1 && z < part_max_z - 1)
			{
				size_t index = part.vertices.size() - 1;
				assert(index == (x - part_min_x) * row_length + (z - part_min_z));

				if((x + z) % 2 == 1)
				{
					part.indices.push_back(index);
					part.indices.push_back(index+1);
					part.indices.push_back(index + row_length);

					part.indices.push_back(index+1);
					part.indices.push_back(index+1 + row_length);
					part.indices.push_back(index   + row_length);
				}
				else
				{
					part.indices.push_back(index);
					part.indices.push_back(index+1 + row_length);
					part.indices.push_back(index   + row_length);

					part.indices.push_back(index);
					part.indices.push_back(index+1);
					part.indices.push_back(index+1 + row_length);
				}
			}
		}
	}
	cerr << "Filled from: " << part_min_x << " - " << part_max_x << " to " << part_min_z << " - " << part_max_z << " with " << part.indices.size()/3 << " triangles" << endl;
}


void LevelDescriptor::drawLevelFR_new(const Level& lvl, int pass, Shaders& shaders)
{
	if(pass == -1)
	{
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);
		
		TextureHandler::getSingleton().bindTexture(2, "");
		TextureHandler::getSingleton().bindTexture(1, "");
		TextureHandler::getSingleton().bindTexture(0, "");
		return;
	}

	glUseProgram(shaders["level_program"]);

	size_t max_x = lvl.max_block_x();
	size_t max_z = lvl.max_block_z();
	assert(max_x > 0);
	assert(max_z > 0);

	static bool level_buffers_loaded = false;
	static vector<VisualLevelPart> parts;

	// Load static buffers.
	if(!level_buffers_loaded) // TODO: Move initialization somewhere else?
	{
		cerr << "CREATING LEVEL PARTS: " << max_x << " - " << max_z << endl;

		level_buffers_loaded = true;

		int divisions = 20;
		for(int i = 0; i < divisions + 1; ++i)
		{
			for(int j = 0; j < divisions + 1; ++j)
			{
				size_t part_min_x = max_x * i / divisions;
				size_t part_min_z = max_z * j / divisions;
				size_t part_max_x = max_x * (i + 1) / divisions + 1;
				size_t part_max_z = max_z * (j + 1) / divisions + 1;
				part_max_x = min(part_max_x, max_x);
				part_max_z = min(part_max_z, max_z);
				if(part_min_x >= part_max_x || part_min_z >= part_max_z)
				{
					continue;
				}

				parts.push_back(VisualLevelPart());
				fill_level_part(parts.back(), part_min_x, part_max_x, part_min_z, part_max_z, max_x, max_z, lvl);
			}
		}
	}

	if(pass == 0)
	{
		if(debugMode)
		{
			TextureHandler::getSingleton().bindTexture(0, "chessboard");
			TextureHandler::getSingleton().bindTexture(1, "chessboard");
			TextureHandler::getSingleton().bindTexture(2, "chessboard");
		}
		else
		{
			TextureHandler::getSingleton().bindTexture(0, "grass");
			TextureHandler::getSingleton().bindTexture(1, "hill");
			TextureHandler::getSingleton().bindTexture(2, "highground");
		}
		
		if(debugMode)
		{
			glUniform4f(shaders.uniform("lvl_ambientLight"), 0.4f, 0.4f, 0.4f, 1.f);
		}
		else
		{
			float r = operator[]("AMBIENT_RED")   / 255.0f;
			float g = operator[]("AMBIENT_GREEN") / 255.0f;
			float b = operator[]("AMBIENT_BLUE")  / 255.0f;
			glUniform4f(shaders.uniform("lvl_ambientLight"), r, g, b, 1.0f);
		}
		
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);
	}
	else
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDepthFunc(GL_EQUAL);
		glDepthMask(GL_FALSE);
		
		glUniform4f(shaders.uniform("lvl_ambientLight"), 0.f, 0.f, 0.f, 1.0f);
	}
	
	
	glUniform4f(shaders.uniform("lvl_activeLights"), float(pass), float(pass+1), float(pass+2), float(pass+3));

	for(size_t i = 0; i < parts.size(); ++i)
	{
		if(frustum.sphereInFrustum(parts[i].bounding_sphere_center, parts[i].bounding_sphere_radius) != FrustumR::OUTSIDE)
		{
			parts[i].preload();
			parts[i].draw();
		}
	}
	
	glUseProgram(0);
}
*/



void LevelDescriptor::drawLevelDeferred(const Level& lvl, Shaders& shaders) const
{
	// Draw terrain with deferred rendering, apply lights later.
	if(!debugMode &&
		(
		strVals.find("TERRAIN_LOW") == strVals.end() ||
		strVals.find("TERRAIN_MID") == strVals.end() ||
		strVals.find("TERRAIN_HIGH") == strVals.end()
		)
	  )
	{
		// can't draw yet..
		return;
	}
	
	glUseProgram(shaders["deferred_level_program"]);

//	size_t height = lvl.pointheight_info.size();
	size_t width = lvl.pointheight_info[0].size();

	assert(buffers_loaded);

	// Load dynamic indices.
	indices.clear();
	for(size_t k = 0; k < level_triangles.size(); ++k)
	{
		Vec3 points[3];
		const BTT_Triangle& tri = level_triangles[k];
		for(size_t i = 0; i < 3; ++i)
		{
			points[2-i].x = tri.points[i].x * Level::BLOCK_SIZE;
			points[2-i].z = tri.points[i].z * Level::BLOCK_SIZE;
			points[2-i].y = lvl.getVertexHeight(tri.points[i].x, tri.points[i].z).getFloat();

			indices.push_back( tri.points[2-i].x * width + tri.points[2-i].z  );
		}
	}
	
	if(debugMode)
	{
		TextureHandler::getSingleton().bindTexture(0, "chessboard");
		TextureHandler::getSingleton().bindTexture(1, "chessboard");
		TextureHandler::getSingleton().bindTexture(2, "chessboard");
	}
	else
	{
		TextureHandler::getSingleton().bindTexture(0, strVals.find("TERRAIN_LOW")->second);
		TextureHandler::getSingleton().bindTexture(1, strVals.find("TERRAIN_MID")->second);
		TextureHandler::getSingleton().bindTexture(2, strVals.find("TERRAIN_HIGH")->second);
	}
	
	drawBuffers();
	
	glUseProgram(0);

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glDepthFunc(GL_LESS);
	
	TextureHandler::getSingleton().bindTexture(2, "");
	TextureHandler::getSingleton().bindTexture(1, "");
	TextureHandler::getSingleton().bindTexture(0, "");
}

