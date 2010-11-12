#include <vector>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <cassert>

using namespace std;

int VERTICES_PRINTED = 0;

struct ChunkInfo
{
	unsigned short ID;
	int Size;
};

struct Vertex
{
	float x,y,z;
};

struct Face
{
	unsigned short p1,p2,p3;
};

struct UVCoord
{
	float U,V;
};

struct ObjMesh
{
	Vertex* vtx;
	Vertex* norm;
	int nVtx;

	int nFace;
	Face* face;

	int nUV;
	UVCoord* UV;

	char MatName[64];
	
	void print()
	{
		assert(nVtx == nUV);
		for(int i = 0; i < nVtx; ++i)
		{
			cerr << "VERTEX ";
			cerr << fixed << setprecision(3);
			cerr << vtx[i].x << " " << vtx[i].y << " " << vtx[i].z << endl;
			cerr << "TEXTURE_COORDINATES ";
			cerr << UV[i].U << " " << UV[i].V << endl;
		}
		for(int i = 0; i < nFace; ++i)
		{
			cerr << "TRIANGLE ";
			cerr << VERTICES_PRINTED + face[i].p1 << " " << VERTICES_PRINTED + face[i].p2 << " " << VERTICES_PRINTED + face[i].p3 << endl;
/*			cerr << fixed << setprecision(3);
			cerr << vtx[face[i].p1].x << " " << vtx[face[i].p1].y << " " << vtx[face[i].p1].z << endl;
			cerr << "         " << vtx[face[i].p2].x << " " << vtx[face[i].p2].y << " " << vtx[face[i].p2].z << endl;
			cerr << "         " << vtx[face[i].p3].x << " " << vtx[face[i].p3].y << " " << vtx[face[i].p3].z << endl;
*/		}
		VERTICES_PRINTED += nVtx;
	}
};

struct ObjMatl
{
	char Name[64];
	unsigned int Red;
	unsigned int Green;
	unsigned Blue;
	char TexName[64];
	int Smooth;
};


class Obj3DS
{
	public:

		int fSize;
		unsigned char* data;
		int nMesh,nMatl;

		ObjMesh* Mesh;
		ObjMatl* Matl;

		Obj3DS();
		~Obj3DS();
		int CountParts(int DataLen);
		int Load(const char* fName);
		ChunkInfo GetChunkInfo(int Offset);
		void Get3fVector(int offset, float& x1, float& x2, float& x3);
		void Get3Vector(int offset, unsigned short& x1, unsigned short& x2, unsigned short& x3);
		void Get2fVector(int offset, float& x1, float& x2);
		
		void print()
		{
			for(int i=0; i<nMesh; i++)
				Mesh[i].print();
		}
};

Obj3DS::Obj3DS()
{
	fSize = 0;
	data = 0;
	Mesh = 0;
	Matl = 0;
}

Obj3DS::~Obj3DS()
{
	delete[] data;
	data = 0;

	// Still to clear vertex, norm, etc allocation
	delete [] Mesh;
	Mesh=0;

	delete [] Matl;
	Matl=0;
}

int Obj3DS::Load(const char* fName)
{
	FILE* in;
	ChunkInfo Info;
	int Offset;
	int SubChunkSize;
	int Value;
	int MatDex;
	int MeshDex;
	int Loop;
	int LOff;
	char bVal;
	short Val;
	short Val2;
	float fVal;
	float fVal2;
	float fVal3;

	bool CopyVal = false;

	MatDex = -1;
	MeshDex = -1;

	in = fopen(fName,"rb");

	if(!in)
	{
		cerr << "fopen(" << fName << ") failed\n";
		return false;
	}

	// Get filelength
	fseek(in, 0, SEEK_END);
	fSize = ftell(in);
	fseek(in, 0, SEEK_SET);

	// Allocate memory
	delete [] data;

	data = new unsigned char[fSize];

	if(!data)
	{
		cerr << "Memory allocation fail??\n";
		fclose(in);
		return false;
	}

	// Read data
	int ret = fread(data, 1, fSize, in);
	if(ret < fSize)
	{
		cerr << "Reading file failed, read " << ret << "bytes\n";
		delete[] data;
		return false;
	}

	fclose(in);

	// Check header
	Offset = 0;
	Info = GetChunkInfo(Offset);

	if(Info.ID != 0x4D4D)
	{
		cerr << "Header fail. File does not appear to be a 3ds file?\n";
		return false;
	}

	if(Info.Size != fSize)
	{
		cerr << "Size fail??\n";
		return false;
	}

	CountParts(fSize);

	Mesh = new ObjMesh[nMesh];
	Matl = new ObjMatl[nMatl];

	Offset = 6;

	while(Offset < fSize)
	{
		Info = GetChunkInfo(Offset);

		switch(Info.ID)
		{
			case 0x0002: // Version
			{
				memcpy(&Value, &data[Offset+6], 4);
//					printf("Chunk 0002 (Version) - %d\nOffset %d\n",Value,Info.Size);
				Offset += Info.Size;
				break;
			}
			case 0x0011: // RGB1
			{
//				printf("Chunk 0011 (RGB1) %d %d %d\nOffset %d\n",data[Offset+6],data[Offset+7],data[Offset+8],Info.Size);
				if(CopyVal)
				{
					Matl[MatDex].Red = data[Offset+6];
					Matl[MatDex].Green = data[Offset+7];
					Matl[MatDex].Blue = data[Offset+8];
					CopyVal = false;
				}
				Offset += Info.Size;
				break;
			}
			case 0x0012: // RGB2
			{
//					printf("Chunk 0012 (RGB2) %d %d %d\nOffset %d\n",data[Offset+6],data[Offset+7],data[Offset+8],Info.Size);
				Offset += Info.Size;
				break;
			}
			case 0x0010:
				Get3fVector(Offset+6,fVal,fVal2,fVal3);
//				printf("Chunk 0010 (RGB) %f %f %f\nOffset %d\n",fVal,fVal2,fVal3,Info.Size);
				Offset += Info.Size;
				break;
			case 0x0013:
				Get3fVector(Offset+6,fVal,fVal2,fVal3);
//				printf("Chunk 0013 (gamma corrected) %f %f %f\nOffset %d\n",fVal,fVal2,fVal3,Info.Size);
				Offset += Info.Size;
				break;
			case 0x0030: // Quantity value for parent chunks
			{
				memcpy(&Val,&data[Offset+6],2);
//					printf("Chunk 0030 (Qty Value) %d\nOffset %d\n",Val,Info.Size);
				Offset += Info.Size;
				break;
			}
			case 0x0100: // Config (Ignore)
				printf("Chunk 0100 (Config) %f\nOffset %d\n",*(double*)&data[Offset+6], Info.Size);
				Offset += Info.Size;
				break;
			case 0x1400: //Shadow map bias
				memcpy(&fVal,&data[Offset+6],4);
				printf("Chunk 1400 (shadow map bias) %f\nOffset %d\n",fVal,Info.Size);
				Offset += Info.Size;
				break;
			case 0x1420:// shadow map size
				memcpy(&Val,&data[Offset+6],2);
				printf("Chunk 1420 (shadow map size) %d\nOffset %d\n",Val,Info.Size);
				Offset += Info.Size;
				break;
			case 0x1450: //shadow map sample range
				memcpy(&fVal,&data[Offset+6],4);
				printf("Chunk 1450 (shadow map sample range) %f\nOffset %d\n",fVal,Info.Size);
				Offset += Info.Size;
				break;

			case 0x1460: //raytrace bias
				memcpy(&fVal,&data[Offset+6],4);
				printf("Chunk 1460 (Raytrace bias) %f\nOffset %d\n",fVal,Info.Size);
				Offset += Info.Size;
				break;

			case 0x1470:
				printf("Chunck 1470 (use raytrace)\nOffset %d\n",6);
				Offset += 6;
				break;

			case 0x2100: //Ambient color
				printf("Chunk 2100 (ambient color)\nOffset %d\n",Info.Size);
				Offset += 6;
				break;

			case 0x3D3D: // Start of Obj
			{
				printf("Chunk 3D3D (Start of Obj)\nOffset %d\n",Info.Size);
				SubChunkSize = Info.Size + Offset; // Set end limit for subchunk
				Offset += 6;
				break;
			}

			case 0x3D3E: // Editor config (Ignore)
				printf("Chunk 3D3E (Editor Config)\nOffset %d\n",Info.Size);
				Offset += Info.Size;
				break;

			case 0x4000: // Start of Mesh
				printf("Chunk 4000 (Start of Mesh) - %s\nOffset %d\n",&data[Offset+6],Info.Size);
				Offset += 6;
				while(data[Offset] != 0) // Seek end of string
					Offset++;

				Offset++; // One more to move past the 0
				MeshDex++;
				break;

			case 0x4100: // Mesh data
				printf("Chunk 4100 (Mesh Data)\nOffset %d\n",Info.Size);
				Offset += 6;
				break;

			case 0x4110: // Vertex List
				memcpy(&Val,&data[Offset+6],2);
//				printf("Chunk 4110 (Vertex List) %d Vertices\nOffset %d\n",Val,Info.Size);
				Mesh[MeshDex].nVtx = Val;
				Mesh[MeshDex].vtx = new Vertex[Val+1];
				for(Loop = 0, LOff = Offset+8; Loop != Val; ++Loop, LOff+=12)
				{
					Get3fVector(LOff,Mesh[MeshDex].vtx[Loop].x,Mesh[MeshDex].vtx[Loop].y,Mesh[MeshDex].vtx[Loop].z);
//					printf("X: %f, Y: %f, Z: %f\n",Mesh[MeshDex].vtx[Loop].x,Mesh[MeshDex].vtx[Loop].y,Mesh[MeshDex].vtx[Loop].z);
				}
				Offset += Info.Size;
				break;

			case 0x4111: // Vertex Options
//				printf("Chunk 4111 (Vertex Options)\nOffset %d\n",Info.Size);
				Offset += Info.Size;
				break;

			case 0x4120: // Face List
				memcpy(&Val, &data[Offset+6], 2);
				printf("Chunk 4120 (Face List) %d polys\nOffset %d\n", Val, Info.Size);
				Mesh[MeshDex].nFace = Val;
				Mesh[MeshDex].face = new Face[Val+1];
				for(Loop = 0, LOff = Offset+8; Loop != Val; ++Loop, LOff+=8)
				{
					Get3Vector(LOff, Mesh[MeshDex].face[Loop].p1, Mesh[MeshDex].face[Loop].p2, Mesh[MeshDex].face[Loop].p3);
					memcpy(&Val2, &data[LOff+6], 2);
					printf("X: %d, Y: %d, Z: %d (", Mesh[MeshDex].face[Loop].p1, Mesh[MeshDex].face[Loop].p2, Mesh[MeshDex].face[Loop].p3);
					if(Val2 & 0x01) printf("AC ");
					if(Val2 & 0x02) printf("BC ");
					if(Val2 & 0x04) printf("AB ");
					printf("visible)\n");
				}
				Offset += Info.Size;
				break;

			case 0x4130: // Material Desc
				Offset += Info.Size;
				break;

			case 0x4140: // UV Map List
				memcpy(&Val,&data[Offset+6],2);
				printf("Chunk 4140 (UV Map List) %d Vertices\nOffset %d\n",Val,Info.Size);
				Mesh[MeshDex].nUV = Val;
				Mesh[MeshDex].UV = new UVCoord[Val+1];
				for(Loop=0,LOff=Offset+8;Loop!=Val;++Loop,LOff+=8)
				{
					Get2fVector(LOff,Mesh[MeshDex].UV[Loop].U,Mesh[MeshDex].UV[Loop].V);
					printf("U: %f, V: %f,\n",Mesh[MeshDex].UV[Loop].U,Mesh[MeshDex].UV[Loop].V);
				}
				Offset += Info.Size;
				break;

			case 0x4160: //local coordinate system
				printf("Chunk 4160 (Local coordinate system)\nOffset %d\n",Info.Size);
				Get3fVector(Offset+6,fVal,fVal2,fVal3);
				printf("X1 vector: %f %f %f\n",fVal,fVal2,fVal3);
				Get3fVector(Offset+18,fVal,fVal2,fVal3);
				printf("X2 vector: %f %f %f\n",fVal,fVal2,fVal3);
				Get3fVector(Offset+30,fVal,fVal2,fVal3);
				printf("X3 vector: %f %f %f\n",fVal,fVal2,fVal3);
				Get3fVector(Offset+42,fVal,fVal2,fVal3);
				printf("O vector: %f %f %f\n",fVal,fVal2,fVal3);
				Offset += Info.Size;

				break;

			case 0x4165:
				memcpy(&bVal,&data[Offset+6],1);
				printf("Chunk 4165 (Object color in 3d editor) %d\nOffset %d\n",bVal,Info.Size);
				Offset += Info.Size;
				break;



			case 0xA000: // Material Name
				printf("Chunk A000 (Material Name) - %s\nOffset %d\n",&data[Offset+6],Info.Size);
				//      lstrcpy(Matl[MatDex].Name,(char*)&data[Offset+6]);
				strcpy(Matl[MatDex].Name,(char*)&data[Offset+6]);
				Offset += Info.Size;
				break;

			case 0xA010: // Material - Ambient Color
				printf("Chunk A010 (Material - Amb Col)\nOffset %d\n",Info.Size);
				Offset += 6;//Info.Size;
				break;

			case 0xA020: // Material - Diffuse Color
				printf("Chunk A020 (Material - Dif Col)\nOffset %d\n",Info.Size);
				CopyVal=true;
				Offset += 6;//Info.Size;
				break;

			case 0xA030: // Material - Spec Color
				printf("Chunk A030 (Material - Spec Col)\nOffset %d\n",Info.Size);
				Offset += 6;//Info.Size;
				break;

			case 0xA040: // Material - Shininess
				printf("Chunk A040 (Material - Shininess)\nOffset %d\n",Info.Size);
				Offset += 6;//Info.Size;
				break;

			case 0xA041: // Material - Shine Strength
				printf("Chunk A041 (Material - Shine Strength)\nOffset %d\n",Info.Size);
				Offset += 6;//Info.Size;
				break;

			case 0xA050: // Material - Transparency
				printf("Chunk A050 (Material - Transparency)\nOffset %d\n",Info.Size);
				Offset += 6;//Info.Size;
				break;

			case 0xA052: // Material - Transperancy fall off
				Get2fVector(Offset+6,fVal,fVal2);
				printf("Chunk A052 (Material trasn. falloff) %f %f\nOffset %d\n",fVal,fVal2,Info.Size);
				Offset += Info.Size;
				break;

			case 0xA053: // Reflection blur percent
				Get2fVector(Offset+6,fVal,fVal2);
				printf("Chunk A053 (Material reflection blur percent) %f %f\nOffset %d\n",fVal,fVal2,Info.Size);
				Offset += Info.Size;
				break;


			case 0xA081: //two sided
				printf("Chunk A081 (Mat. Two sided)\nOffset %d\n",6);
				Offset += 6;
				break;//Info.Size;

			case 0xA087: // Wire thickness
				memcpy(&fVal,&data[Offset+6],4);
				printf("Chunk A087 (Mat. Wire Thickness) %f\nOffset %d\n",fVal,Info.Size);
				Offset += Info.Size;
				break;

			case 0xA100: // Material - Type (Flat,Gourad, Phong, Metal)
				memcpy(&Val,&data[Offset+6],2);
				printf("Chunk A100 (Material Type) %d\nOffset %d\n",Val,Info.Size);
				Matl[MatDex].Smooth=Val;
				Offset += Info.Size;
				break;

			case 0xA200: // Material - Start of Texture Info
				printf("Chunk A200 (Material Tex Map)\nOffset %d\n",Info.Size);
				Offset += 6;
				break;

			case 0xA300: // Material - Texture Name
				printf("Chunk A300 (Material Tex Map Name) %s\nOffset %d\n",&data[Offset+6],Info.Size);
				//      lstrcpy(Matl[MatDex].TexName,(char*)&data[Offset+6]);
				strcpy(Matl[MatDex].TexName,(char*)&data[Offset+6]);
				Offset += Info.Size;
				break;

			case 0xA351: // Material - Texture Options
				memcpy(&Val,&data[Offset+6],2);
				printf("Chunk A351 (Material Tex Options) %d\nOffset %d\n",Val,Info.Size);
				Offset += Info.Size;
				break;

			case 0xA354: // Material - Texture U Scale
				memcpy(&fVal,&data[Offset+6],4);
				printf("Chunk A354 (Material Tex U Scale) %f\nOffset %d\n",fVal,Info.Size);
				Offset += Info.Size;
				break;

			case 0xA356: // Material - Texture V Scale
				memcpy(&fVal,&data[Offset+6],4);
				printf("Chunk A356 (Material Tex V Scale) %f\nOffset %d\n",fVal,Info.Size);
				Offset += Info.Size;
				break;

			case 0xA35A: // Material - Texture V Offset
				memcpy(&fVal,&data[Offset+6],4);
				printf("Chunk A35A (Material Tex V Offset) %f\nOffset %d\n",fVal,Info.Size);
				Offset += Info.Size;
				break;

			case 0xA35C: // Material - Texture V Offset
				memcpy(&fVal,&data[Offset+6],4);
				printf("Chunk A35C (Material Tex Rotation) %f\nOffset %d\n",fVal,Info.Size);
				Offset += Info.Size;
				break;

			case 0xAFFF: // Material Start
				printf("Chunk AFFF (Start of Material)\nOffset %d\n",Info.Size);
				MatDex++;
				Offset += 6;
				break;

			default:
				printf("Chunk %04X (Unknown)\nOffset %d\n",Info.ID,Info.Size);
				Offset += Info.Size;
				break;
		}

	}

	return true;
}


int Obj3DS::CountParts(int DataLen)
{
	long Offset=6;
	ChunkInfo Info;
	nMesh=0,nMatl=0;


	while(Offset<DataLen)
	{
		Info=GetChunkInfo(Offset);

		switch(Info.ID)
		{
			case 0x3D3D: // Start of Obj
			{
				Offset += 6;
				break;
			}

			case 0x4000: // Start of Mesh
				nMesh++;
				Offset += Info.Size;
				break;

			case 0xAFFF: // Start of Material
				nMatl++;
				Offset += Info.Size;
				break;


			default:
				Offset += Info.Size;
				break;
		}

	}

	return nMesh;
}


ChunkInfo Obj3DS::GetChunkInfo(int Offset)
{
	ChunkInfo Chunk;
	memcpy(&Chunk.ID,&data[Offset],2);
	memcpy(&Chunk.Size,&data[Offset+2],4);
	return Chunk;
}

void Obj3DS::Get3fVector(int offset,float &x1,float &x2,float &x3)
{
	memcpy(&x1,&data[offset],4);
	memcpy(&x2,&data[offset+4],4);
	memcpy(&x3,&data[offset+8],4);
}

void Obj3DS::Get3Vector(int offset,unsigned short &x1,unsigned short &x2,unsigned short &x3)
{
	memcpy(&x1,&data[offset],2);
	memcpy(&x2,&data[offset+2],2);
	memcpy(&x3,&data[offset+4],2);
}

void Obj3DS::Get2fVector(int offset,float &x1,float &x2)
{
	memcpy(&x1,&data[offset],4);
	memcpy(&x2,&data[offset+4],4);
}

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		cout << "Usage: ./program file.3ds\n";
		return 1;
	}


	Obj3DS model;
	model.Load(argv[1]);
	
	model.print();

	return 0;
}

