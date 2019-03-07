/*=======================================================================
   Copyright (c) 2018 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_INPUT_H
# define GS_INPUT_H

/** \file gs_input.h 
 * parses input file or string */

// gs_array.h or gs_string.h cannot be included here because of circular references
# include <stdio.h>
# include <sig/gs.h>

class GsString;
template <typename X> class GsArray;

/*! \class GsInput gs_input.h
	\brief Parses input file or string

	GsInput reads data from a string buffer or from a file. It can
	be used to read data byte per byte, or by parsing tokens that are 
	recognized as strings, delimiters, or numbers. */
class GsInput
{  public :

	/*! Indicates the type of the current input. If it is invalid, GsInput
		needs to be connected to a file or string in order to become valid. */
	enum Type { TypeFile,   //!< Input from a file
				TypeString, //!< Input from a string buffer
				TypeInvalid //!< Input not initialized, valid() will return false
			  };

	/*! Indicates the type of the token returned in get() */
	enum TokenType { String,	//!< A sequence of chars, if inside "" escape chars are converted
					 Number,	//!< An integer or real number
					 Delimiter, //!< Any delimiter character
					 End		//!< End of the input encountered
				   };

   private :
	struct Data;
	mutable Data* _data; // some internal data
	union { FILE  *f; const char *s; } _cur; // the current input position
	int		_curline;	 // keeps track of the current line
	char	_comchar;	 // skip line when _comchar is found
	gsbyte  _type;		 // the input Type
	gsbyte  _lowercase;  // if characters should be converted to lowercase
	gsbyte  _lnumreal;   // 1 if last num read had '.', 0 otherwise
	int		_maxtoksize; // max size allowed for parsed tokens
	char*   _filename;   // stores file name if one was open for the input
	void _init ( char c );
	int	 _peekbyte () { int c=readchar(); unget(c); return c; }

   public : 

	/*! Construct an input of type GsInput::TypeInvalid. GsInput will only be operational
		when linked to a file or string by calling an init() or open() method. While 
		GsInput is of type Invalid, the valid() method will return false. The parameter
		com is the comment character for this input and is 0 by default. */
	GsInput ( char com=0 );

	/*! Construct an input of type GsInput::File and therefore GsInput will read tokens
		from the given C-style file stream. If the file pointer is null, GsInput will be
		initialized as TypeInvalid, and not as a File type. Parameter com is the comment
		character for this input and is 0 by default. */
	GsInput ( FILE *file, char com=0 );

	/*! The destructor closes the associated file if it is a TypeFile input. If the 
		associated input file has to be left open, call abandon() before. */
   ~GsInput ();

	/*! Closes actual input, and init it as TypeString. If buff is null, GsInput will
		be initialized as TypeInvalid, and not as a String type. */
	void init ( const char *buff );

	/*! Closes actual input, and init it as a File type. If file is null, GsInput will
		be initialized as TypeInvalid, and not as a File type. */
	void init ( FILE *file );

	/*! Closes current input, and init it as a File type, opening the given file name
		with fopen. If m==0 (default) the file will be open as a text file.
		Use m==b to open the file as a binary file.
		If the file could not be open, GsInput is initialized as TypeInvalid
		and false is returned, otherwise the filename is stored and true is returned. */
	bool open ( const char* filename, char m=0 );

	/*! Closes actual input and set it to be of TypeInvalid. If GsInput is of type
		file, the associated file is closed. In all cases, unget data is freed, 
		filename is cleared and the line number becomes 1. */
	void close ();

	/*! Puts GsInput into TypeInvalid mode but without closing the associated file.
		If GsInput is not of File type, the effect is the same as close(). */
	void abandon ();

	/*! If the input is done from a file, return the associated FILE pointer,
		otherwise returns 0 */
	FILE* filept () const { return _type==TypeFile? _cur.f:0; }

	/*! Returns the file name used for opening a file input, or null if not available */
	const char* filename () const { return _filename; }
	
	/*! Associates with the input a file name. The string is stored but not used by GsInput. */
	void filename ( const char* s ) { gs_string_set(_filename,s); }

	/*! Returns true if the input is a correctly initialized file or string.
		Otherwise returns false. */
	bool valid () const { return _type!=(gsbyte)TypeInvalid; }

	/*! Returns the type of the GsInput. */
	Type type () const { return (Type) _type; }

	/*! Returns the current line of the input. When GsInput is created or initialized, the
		current line is set to 1. Afterwards, the current line is updated internally each
		time a newline char is read. */
	int curline () const { return _curline; }

	/*! Defines the comment character used for all "get methods". When the specified
		character is encountered the whole line after the character is skipped. */
	void commentchar ( char c ) { _comchar=c; }

	/*! Returns the current comment character being used. */
	char commentchar () const { return _comchar; }

	/*! If set to true all read characters are transformed to lowercase. Default is false. */
	void lowercase ( bool b ) { _lowercase=b; }

	/*! Returns true if characters are being transformed to lowercase and false otherwise. */
	bool lowercase () const { return _lowercase==1; }

	/*! Returns true if the current input is pointing after the end of the file.
		If the input source is empty but there is data to unget, the input
		is not considered finished.*/
	bool end ();

	/*! Reads the next character from the associated input and returns it.
		In case the end was reached, or GsInput is of TypeInvalid, -1 is 
		returned. Comments are not parsed out, but unget characters
		are correctly handled and curline is updated. */
	int readchar (); // comments not handled, unget yes

	/*! Reads all bytes of the current line, i.e. untill a '\n' is read,
		and put them in the given buffer (including the '\n').
		The input will point to the first byte after the new line character.
		The last character read is returned and will be either '\n' or EOF/-1.
		Uses method readchar(), and therefore comments are not parsed. */
	int readline ( GsString& buf );

	/*! Read all bytes of the input, starting from the current position, and placing 
		them in the given array. The input will point to its end. If the input is of 
		TypeInvalid, nothing is done. The input is not parsed, just copied to the array. */
	void readall ( GsArray<gsbyte>& buf );

	/*! Read all characters of the input, starting from the current position, and placing 
		them in the given string. The input will point to its end. If the input is of 
		TypeInvalid, nothing is done. The input is not parsed, just copied to the string. */
	void readall ( GsString& buf );

	/*! Reads all the current line (with readchar), stopping one byte after the
		first newline encountered. */
	void skipline ();

	/*! Sets the maximum allowed size for parsed tokens. Default is 128. */
	void max_token_size ( int s ) { _maxtoksize=s; }

	/*! Returns the current maximum allowed size for parsed tokens. */
	int max_token_size () const { return _maxtoksize; }

	/*! Skip white spaces and check the type of the next token without
		removing it from the input stream, and returns:
		1) End if end of file encountered;
		2) String if character, double quotes ('') or underline(_);
		3) Number if a digit, or -, + or . preceeding a digit;
		4) otherwise Delimiter is returned.
		If commentchar is found, the entire line after commentchar is disconsidered. */
	TokenType check ();

	/*! Skips white spaces and commented lines, parses the next token in the input
		and returns its type. The result is the same as calling get(check()).
		The read token and token type can be retrieved with ltoken() and ltype(). 
		Note: strings between quotes are parsed and stored without the quotes*/
	TokenType get ();

	/*! Version of get for a when the type has been already determined by check(). */
	TokenType get ( TokenType type );

	/*! Parses the next token and returns the first character of it. */
	char getc ();

	/*! Parses the next token and returns the string containing the token.
		Note: strings between quotes are returned without the quotes. */
	const GsString& gets ();

	/*! Parses the next token as a string or as string, delimiter '.', and another string;
		which form a filename. */
	const GsString& getfilename ();

	/*! Parses the next token, convert it to an integer and returns it. */
	int geti ();

	/*! Parses the next token, convert it to a long integer and returns it. */
	long getl ();

	/*! Parses the next token, convert it to a float and returns it. */
	float getf ();

	/*! Returns true if the last num read by get() was a real number (not integer),
		and false otherwise. The returned value is updated each time a call to get()
		returns a token of type Number. A number is considered real when '.' is found */
	bool lnumreal () { return _lnumreal==1; }

	/* Returns a reference to the internal buffer containing the last token
	   parsed with any of the get methods. */
	const GsString& ltoken() const;

	/* Returns the type of the last token parsed with get(void). */
	TokenType ltype() const;

	/*! Put the last parsed token (by any of the get methods) in the unget stack.
		with a space separating the token. If no token was parsed, nothing is done. */
	void unget ();

	/*! Put given string in the unget stack exactly as given, no additional spaces added. */
	void unget ( const char* s );

	/*! Put given character in the unget stack. */
	void unget ( char c );

	/*! Skip the next n tokens, by calling get() n times. 
		Parameter n has a default value of 1.
		Returns true if all tokens could be successfully skipped */
	bool skip ( int n=1 );

	/*! Reads tokens using get() until a string is found equal to the given name. 
		Case-insensitive comparison is used.
		Returns false if End was reached before, otherwise returns true. */
	bool skipto ( const char *name );

	/*! Operator to read an integer. */
	friend GsInput& operator>> ( GsInput& in, int& i ) { i=in.geti(); return in; }

	/*! Operator to read a gsint16 integer. */
	friend GsInput& operator>> ( GsInput& in, gsint16& i ) { i=(gsint16)in.geti(); return in; }

	/*! Operator to read a gsuint16 integer. */
	friend GsInput& operator>> ( GsInput& in, gsuint16& i ) { i=(gsuint16)in.geti(); return in; }

	/*! Operator to read an unsigned integer. */
	friend GsInput& operator>> ( GsInput& in, gsuint& i ) { i=(gsuint)in.geti(); return in; }

	/*! Operator to read an unsigned char. */
	friend GsInput& operator>> ( GsInput& in, gsbyte& c ) { c=(gsbyte)in.geti(); return in; }

	/*! Operator to parse next token and get 1st character. */
	friend GsInput& operator>> ( GsInput& in, char& c ) { c=in.getc(); return in; }

	/*! Operator to read a float number. */
	friend GsInput& operator>> ( GsInput& in, float& f ) { f=in.getf(); return in; }

	/*! Operator to read a double number. */
	friend GsInput& operator>> ( GsInput& in, double& d ) { d=(double)in.getf(); return in; }

	/*! Operator to read a string into GsString. */
	friend GsInput& operator>> ( GsInput& in, GsString& s );

	/*! Operator to read a string into a given buffer. Pointer st must point to
		a buffer with enought space to receive the parsed string. */
	friend GsInput& operator>> ( GsInput& in, char* st );
};

//============================== end of file ===============================

# endif  // GS_INPUT_H
