/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/


#ifndef AX_D3D9SHADER_H
#define AX_D3D9SHADER_H

#define AX_SU(x, y) DX9_Uniform::setUniform(Uniforms::x,y);

AX_BEGIN_NAMESPACE

class DX9_Shader;

//--------------------------------------------------------------------------
// class UniformCache
//--------------------------------------------------------------------------

class DX9_Uniform : public UniformItem
{
public:
	DX9_Uniform(UniformItem &item, D3DXHANDLE param);
	virtual ~DX9_Uniform();

	bool isCached() const;
	void cache();

	template< class Q >
	static void setUniform(Uniforms::ItemName itemname, const Q &q) {
		UniformItem &item = g_uniforms.getItem(itemname);

		if (item.m_valueType == UniformItem::vt_Texture) {
			item.set(q);
			return;
		}

		if (memcmp(&q, item.m_datap, item.m_dataSize) == 0)
			return;

		item.set(q);

		setUniform(item, &q);
	}

	static void setUniform(UniformItem &item, const void *q);

public:
	UniformItem *m_src;
	D3DXHANDLE m_param;
};

//--------------------------------------------------------------------------
// class CGsamplerann
//--------------------------------------------------------------------------

class DX9_SamplerInfo : public SamplerInfo
{
public:
	FixedString m_paramName;
	D3DXHANDLE m_param;
	int m_register;
};
typedef Sequence<DX9_SamplerInfo*>	DX9_SamplerInfos;

//--------------------------------------------------------------------------
// class D3D9pixeltotexel
//--------------------------------------------------------------------------

class DX9_Pixel2Texel
{
public:
	String m_name;
	D3DXHANDLE m_param;
	FloatSeq m_pixelValue;
	FloatSeq m_scaledValue;
};
typedef Sequence<DX9_Pixel2Texel>	DX9_Pixel2Texels;

class DX9_Pass
{
public:
	friend class DX9_Shader;
	
	struct ParamDesc {
		D3DXCONSTANT_DESC d3dDesc;
		const DX9_Pixel2Texel *p2t;
	};

	DX9_Pass(DX9_Shader *shader, D3DXHANDLE d3dxhandle);
	~DX9_Pass();

	void begin();

protected:
	void initVs();
	void initPs();
	void initState();
	void initSampler(const D3DXCONSTANT_DESC &desc);

	const DX9_Pixel2Texel *findPixel2Texel(const String &name);
	void setParameters();
	void setParameter(const ParamDesc &param, const float *value, bool isPixelShader);

private:
	DX9_Shader *m_shader;
	D3DXHANDLE m_d3dxhandle;

	// shader
	IDirect3DVertexShader9 *m_vs;
	IDirect3DPixelShader9 *m_ps;

	// render state
	DWORD m_depthTest;
	DWORD m_depthWrite;
	DWORD m_cullMode;
	DWORD m_blendEnable;
	DWORD m_blendSrc;
	DWORD m_blendDst;

	// material sampler
	int m_matSamplers[SamplerType::NUMBER_ALL];

	// sys sampler
	Dict<int,int> m_sysSamplers;

	// batch sampler
	DX9_SamplerInfos m_batchSamplers;

	// local parameter
	Dict<String,ParamDesc> m_vsParameters;
	Dict<String,ParamDesc> m_psParameters;
};

class DX9_Technique
{
public:
	friend class DX9_Shader;

	DX9_Technique(DX9_Shader *shader, D3DXHANDLE d3dxhandle);
	~DX9_Technique();

private:
	enum {MAX_PASSES = 8};
	DX9_Shader *m_shader;
	D3DXHANDLE m_d3dxhandle;

	int m_numPasses;
	DX9_Pass *m_passes[MAX_PASSES];
};

//--------------------------------------------------------------------------
// class D3D9Shader
//--------------------------------------------------------------------------

class DX9_Shader : public Shader
{
public:
	friend class DX9_Pass;
	friend class DX9_Technique;

	DX9_Shader();
	virtual ~DX9_Shader();

	// implement Shader
	virtual bool doInit(const String &name, const ShaderMacro &macro = g_shaderMacro);
	virtual bool isDepthWrite() const;
	virtual bool haveTextureTarget() const;
	virtual int getNumSampler() const;
	virtual SamplerInfo *getSamplerAnno(int index) const;
	virtual int getNumTweakable() const;
	virtual ParameterInfo *getTweakableDef(int index);
	virtual SortHint getSortHint() const;
	virtual bool haveTechnique(Technique tech) const;
	virtual const ShaderInfo *getShaderInfo() const { return 0; }

	void setSystemMap(SamplerType maptype, IDirect3DTexture9 *tex);
	// set pixel to texel conversion parameter
	void setPixelToTexel(int width, int height);

	void setCoupled(Material *mtr);

	ID3DXEffect *getObject() const { return m_object; }

	UINT begin(Technique tech);
	void beginPass(UINT pass);
	void endPass();
	void end();

protected:
	void initTechniques();
	void initFeatures();
	void initSortHint();

	void initAnnotation();
	void initSamplerAnn(D3DXHANDLE param);
	void initParameterAnn(D3DXHANDLE param);
	void initPixelToTexel(D3DXHANDLE param);

	void initAxonObject();

	D3DXHANDLE findTechnique(Technique tech);
	D3DXHANDLE getUsedParameter(const char *name);
	bool isParameterUsed(D3DXHANDLE param);

private:
	LPD3DXEFFECT m_object;              // Effect object
	String m_keyString;
	SortHint m_sortHint;

	D3DXHANDLE m_d3dxTechniques[Technique::Number];
	IDirect3DTexture9 *m_samplerBound[SamplerType::NUMBER_ALL];
	D3DXHANDLE m_curTechnique;

	bool m_haveTextureTarget;

	DX9_SamplerInfos m_samplerannSeq;

	// pixel2texel
	DX9_Pixel2Texels pixel2Texels;
	int m_p2tWidth, m_p2tHeight;

	DX9_Technique *m_techniques[Technique::Number];
	DX9_Technique *m_curTech;
	Material *m_coupled;

	// shader info
	ShaderInfo *m_shaderInfo;
};

//--------------------------------------------------------------------------
// class D3D9shadermanager
//--------------------------------------------------------------------------

class D3D9ShaderManager : public ShaderManager
{
public:
	D3D9ShaderManager();
	virtual ~D3D9ShaderManager();

	virtual Shader *findShader(const String &name, const ShaderMacro &macro = g_shaderMacro);
	virtual Shader *findShader(const FixedString &nameId, const ShaderMacro &macro);
	virtual void saveShaderCache(const String &name);
	virtual void applyShaderCache(const String &name);

	DX9_Shader *findShaderDX(const String &name, const ShaderMacro &macro = g_shaderMacro);

protected:
	void _initialize();

private:
	typedef Dict<FixedString,Dict<ShaderMacro,DX9_Shader*> > ShaderPool;
	ShaderPool m_shaderPool;
	DX9_Shader *m_defaulted;
	ShaderInfoDict m_shaderInfoDict;
};


AX_END_NAMESPACE


#endif // end guardian
