#ifndef ISFDoc_hpp
#define ISFDoc_hpp

#include <vector>
#include <mutex>
#include "VVISF_Base.hpp"
#include "ISFAttr.hpp"




namespace VVISF
{




class ISFScene;




/*!
\ingroup VVISF_BASIC
\brief Describes an "ISF file"- requires an actual file on disk which is parsed.  Capable of generating GLSL source code for the various shader types.

Constructing an instance of this class will load the files from disk into local std::string vars, parse them, and populate the instance with all the attributes(ISFAttr)/passes(ISFPassTarget) necessary to fully describe the ISF file.

Notes on use:
- ISFDoc's constructor will throw an ISFErr if the file cannot be opened, or if there's a problem parsing the JSON blob in the ISF file.
- ISFDoc can be used to examine the properties of "ISF files" on disk- create an ISFDoc instance, passing the constructor the path of the file to load, and then then you can examing the properties of the doc to get info about the ISF file.
- Internally, ISFDoc has all the plumbing necessary to be used to render the ISF file- when you tell an ISFScene to use a file, it's actually creating an ISFDoc which is used to store information necessary for rendering- both ISFAttr and ISFPassTarget cache GL resources and interact directly with GL to do things like set uniform values, calculate rendering resolutions by evaluating strings and substituting variables, and things like that.  Even the GLSL source code ISFScene compiles and runs for its GL programs is generated by the ISFDoc.
*/
class VVISF_EXPORT ISFDoc	{
	private:
		std::recursive_mutex		_propLock;
		
		std::string		*_path = nullptr;	//	full path to the loaded file
		std::string		*_name = nullptr;	//	just the file name (including its extension)
		std::string		*_description = nullptr;	//	description of whatever the file does
		std::string		*_credit = nullptr;	//	credit
		std::string		*_vsn = nullptr;
		ISFFileType		_type = ISFFileType_Source;
		bool			_throwExcept = true;
		
		std::vector<std::string>	_categories;	//	array of strings of the category names this doc should be listed under
		std::vector<ISFAttrRef>		_inputs;	//	array of ISFAttrRef instances for the various inputs
		std::vector<ISFAttrRef>		_imageInputs;	//	array of ISFAttrRef instances for the image inputs (the image inputs are stored in two arrays).
		std::vector<ISFAttrRef>		_audioInputs;	//	array of ISFAttrRef instances for the audio inputs
		std::vector<ISFAttrRef>		_imageImports;	//	array of ISFAttrRef instances that describe imported images. attrib's 'attribName' is the name of the sampler, attrib's 'description' is the path to the file.
		
		//bool					bufferRequiresEval = false;	//	NO by default, set to YES during file open if any of the buffers require evaluation (faster than checking every single buffer every pass)
		std::vector<ISFPassTargetRef>	_persistentPassTargets;
		std::vector<ISFPassTargetRef>	_tempPassTargets;
		std::vector<std::string>		_renderPasses;
		
		std::string			*_jsonSourceString = nullptr;	//	the JSON std::string from the source *including the comments and any linebreaks before/after it*
		std::string			*_jsonString = nullptr;	//	the JSON std::string copied from the source- doesn't include any comments before/after it
		std::string			*_vertShaderSource = nullptr;	//	the raw vert shader source before being find-and-replaced
		std::string			*_fragShaderSource = nullptr;	//	the raw frag shader source before being find-and-replaced
		
		ISFScene		*_parentScene = nullptr;	//	nil by default, weak ref to the scene that "owns" me.  only non-nil when an ISFScene is using the doc to render.
		
	public:
		
		/*!
		\name Constructors
		*/
		///@{
		
		//! Constructs an ISFDoc instance from a passed file on disk.  Consider using CreateISFDocRef() instead.
		/*!
		\param inPath The path to the ISF file you want to load as an ISFDoc.
		\param inParentScene The scene that will be used to render this ISFDoc, or null if no scene is to be used.
		\param inThrowExcept Whether or not the ISFDoc should throw any exceptions if it encounters errors parsing anything.
		Throws an ISFErr if there is a problem of some sort loading the ISF file from disk or parsing the JSON in the ISF file.
		*/
		ISFDoc(const std::string & inPath, ISFScene * inParentScene=nullptr, const bool & inThrowExcept=true) noexcept(false);
		
		//! Constructs an ISFDoc instance from shader strings.  Consider using CreateISFDocRef() instead.
		/*
		\param inFSContents A std::string containing the fragment shader portion of the ISF file.  The JSON blob that defines the ISF file must be contained in here.
		\param inVSContents A std::string containing the vertex shader portion of the ISF file.  If you don't have a vertex shader to pass, VVISF defines a static std::string "ISFVertPassthru_GL2", which should work as a "passthru" vertex shader for most purposes.
		\param inImportsDir A std::string containing the path to the directory that will contain any related media files (used for IMPORT/etc).
		\param inParentScene The scene that will be used to render this ISFDoc, or null if no scene is to be used.
		\param inThrowExcept Whether or not the ISFDoc should throw any exceptions if it encounters errors parsing anything.
		Throws an ISFErr if there is a problem of some sort parsing the JSON blob from the frag shader std::string.
		*/
		ISFDoc(const std::string & inFSContents, const std::string & inVSContents, const std::string & importsDir, ISFScene * inParentScene=nullptr, const bool & inThrowExcept=true);
		
		///@}
		
		
		~ISFDoc();
		
		
		/*!
		\name ISF file properties
		*/
		///@{
		
		//!	Returns the path of the ISF file for the receiver.  This is probably the path to the frag shader.
		std::string path() const { return (_path==nullptr) ? std::string("") : std::string(*_path); }
		//!	Returns the name of the receiver's ISF file (the file name, minus the extension).
		std::string name() const { return (_name==nullptr) ? std::string("") : std::string(*_name); }
		//!	Returns the receiver's "description" std::string, as defined in its JSON blob ("DESCRIPTION").
		std::string description() const { return (_description==nullptr) ? std::string("") : std::string(*_description); }
		//!	Returns the receiver's "credit" std::string, as defined in its JSON blob ("CREDIT").
		std::string credit() const { return (_credit==nullptr) ? std::string("") : std::string(*_credit); }
		//!	Returns the receiver's "vsn" std::string, as defined in its JSON blob ("VSN")
		std::string vsn() const { return (_vsn==nullptr) ? std::string("") : std::string(*_vsn); }
		//!	Returns the receiver's file type.
		ISFFileType type() const { return _type; }
		//!	Returns a std::vector containing strings listing the receiver's categories.
		std::vector<std::string> & categories() { return _categories; }
		
		///@}
		
		
		/*!
		\name ISF attribute/INPUT getters
		*/
		///@{
		
		//!	Returns a std::vector of ISFAttrRef instances describing all of the receiver's inputs.
		std::vector<ISFAttrRef> & inputs() { return _inputs; }
		//!	Returns a std::vector of ISFAttrRef instances describing only the receiver's image inputs.
		std::vector<ISFAttrRef> & imageInputs() { return _imageInputs; }
		//!	Returns a std::vector of ISFAttrRef instances describing only the receiver's audio inputs.
		std::vector<ISFAttrRef> & audioInputs() { return _audioInputs; }
		//!	Returns a std::vector of ISFAttrRef instances describing only the receiver's audioFFT inputs.
		std::vector<ISFAttrRef> & imageImports() { return _imageImports; }
		//!	Returns a std::vector of ISFAttrRef instances describing only the receiver's inputs that match the passed type.
		std::vector<ISFAttrRef> inputsOfType(const ISFValType & inInputType);
		//!	Returns the ISFAttrRef for the input with the passed name
		ISFAttrRef input(const std::string & inAttrName);
		
		///@}
		
		
		/*!
		\name ISF render pass getters
		*/
		///@{
		
		//!	Returns a std::vector of ISFPassTargetRef instances describing every pass that has a persistent buffer.
		std::vector<ISFPassTargetRef> persistentPassTargets() const { return _persistentPassTargets; }
		//!	Returns a std::vector of ISFPassTargetRef instances describing every pass that doesn't have a persistent buffer.
		std::vector<ISFPassTargetRef> tempPassTargets() const { return _tempPassTargets; }
		//!	Returns a std::vector of std::std::string instances describing the names of the render passes, in order.  If the names were not specified properly in the JSON blob, this array will be incomplete or inaccurate and rendering won't work!
		std::vector<std::string> & renderPasses() { return _renderPasses; }
		//!	Returns the GLBufferRef for the passed key.  Checks all attributes/inputs as well as persistent and temp buffers.
		const VVGL::GLBufferRef getBufferForKey(const std::string & n);
		//!	Returns the persistent buffer for the render pass with the passed key.
		const VVGL::GLBufferRef getPersistentBufferForKey(const std::string & n);
		//!	Returns the temp buffer for the render pass with the passed key.
		const VVGL::GLBufferRef getTempBufferForKey(const std::string & n);
		//!	Returns the ISFPassTarget that matches the passed key.  Returns null if no pass could be found.
		const ISFPassTargetRef passTargetForKey(const std::string & n);
		//!	Returns the ISFPassTarget that matches the passed key.  Returns null if no pass could be found or if the pass found wasn't flagged as requiring a persistent buffer.
		const ISFPassTargetRef persistentPassTargetForKey(const std::string & n);
		//!	Returns the ISFPassTarget that matches the passed key.  Returns null if no pass could be found or if the pass found was flagged as requiring a persistent buffer.
		const ISFPassTargetRef tempPassTargetForKey(const std::string & n);
		
		///@}
		
		
		/*!
		\name ISF file source code getters
		*/
		///@{
		
		//! Returns the JSON std::string from the source *including the comments and any linebreaks before/after it*
		std::string * jsonSourceString() const { return _jsonSourceString; }
		//!	Returns the JSON std::string copied from the source- doesn't include any comments before/after it
		std::string * jsonString() const { return _jsonString; }
		//!	Returns the raw vert shader source before being find-and-replaced
		std::string * vertShaderSource() const { return _vertShaderSource; }
		//!	Returns the raw frag shader source before being find-and-replaced
		std::string * fragShaderSource() const { return _fragShaderSource; }
		//!	Populates the passed var with the JSON std::string from the source *including the comments and any linebreaks before/after it*
		void jsonSourceString(std::string & outStr);
		
		///@}
		
		void setParentScene(ISFScene * n) { _parentScene=n; }
		ISFScene * parentScene() { return _parentScene; }
		
		//	returns a std::string describing the type of the expected texture samplers ("2" for 2D, "R" for Rect, "C" for Cube).  save this, if it changes in a later pass the shader source must be generated again.
		std::string generateTextureTypeString();
		/*!
		\brief Returns a true if successful.  Generates GLSL source code, populating the provided vars with strings that are usable for frag/vert shaders
		\param outFragSrc A non-null pre-allocated std::std::string variable which will be populated with the fragment shader source code generated for this ISF file.
		\param outVertSrc A non-null pre-allocated std::std::string variable which will be populated with the vertex shader source code generated for this ISF file.
		\param inGLVers The version of OpenGL that the generated source code must be compatible with.
		\param inVarsAsUBO Defaults to false.  If true, variable declarations for non-image INPUTS will be assembled in a uniform block.  This option was added because a downstream utility requires it.
		*/
		bool generateShaderSource(std::string * outFragSrc, std::string * outVertSrc, VVGL::GLVersion & inGLVers, const bool & inVarsAsUBO=false);
		//	this method must be called before rendering (passes/etc may have expressions that require the render dims to be evaluated)
		void evalBufferDimensionsWithRenderSize(const VVGL::Size & inSize);
		
		VVISF_EXPORT friend std::ostream & operator<<(std::ostream & os, const ISFDoc & n);
		
	protected:
		//	used so we can have two constructors without duplicating code
		void _initWithRawFragShaderString(const std::string & inRawFile);
		//	returns a true if successful.  populates a std::string with variable declarations for a frag shader
		bool _assembleShaderSource_VarDeclarations(std::string * outVSString, std::string * outFSString, VVGL::GLVersion & inGLVers, const bool & inVarsAsUBO=false);
		//	returns a true if successful.  populates a map with std::string/value pairs that will be used to evaluate variable names in strings
		bool _assembleSubstitutionMap(std::map<std::string,double*> * outMap);
};




//! Constructs an ISFDoc instance from a passed file on disk.
/*!
\relatedalso ISFDoc
\param inPath The path to the ISF file you want to load as an ISFDoc.
\param inParentScene The scene that will be used to render this ISFDoc, or null if no scene is to be used.
\param inThrowExcept Whether or not the ISFDoc should throw any exceptions if it encounters errors parsing anything.
Throws an ISFErr if there is a problem of some sort loading the ISF file from disk or parsing the JSON in the ISF file.
*/
inline ISFDocRef CreateISFDocRef(const std::string & inPath, ISFScene * inParentScene=nullptr, const bool & inThrowExcept=true) noexcept(false) { return std::make_shared<ISFDoc>(inPath,inParentScene,inThrowExcept); }

//! Constructs an ISFDoc instance from shader strings.
/*!
\relatedalso ISFDoc
\param inFSContents A std::string containing the fragment shader portion of the ISF file.  The JSON blob that defines the ISF file must be contained in here.
\param inVSContents A std::string containing the vertex shader portion of the ISF file.  If you don't have a vertex shader to pass, VVISF defines a static std::string "ISFVertPassthru_GL2", which should work as a "passthru" vertex shader for most purposes.
\param inParentScene The scene that will be used to render this ISFDoc, or null if no scene is to be used.
\param inThrowExcept Whether or not the ISFDoc should throw any exceptions if it encounters errors parsing anything.
Throws an ISFErr if there is a problem of some sort parsing the JSON blob from the frag shader std::string.
*/
inline ISFDocRef CreateISFDocRefWith(const std::string & inFSContents, const std::string & inImportsDir=std::string("/"), const std::string & inVSContents=std::string(ISFVertPassthru_GL2), ISFScene * inParentScene=nullptr, const bool & inThrowExcept=true) { return std::make_shared<ISFDoc>(inFSContents,inVSContents,inImportsDir,inParentScene,inThrowExcept); }



}




#endif /* ISFDoc_hpp */
