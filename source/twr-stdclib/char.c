#include <ctype.h>
#include <string.h>
#include <twr-jsimports.h>

// debug versions of these functions assert that "int c" is a valid "unsigned char" or EOF(-1) per the spec
// isalpha() type functions seem to accept ISO-8859-1 in gcc, so that we do as well.
// ISO-8859-1 also maps to the first 256 unicode codepoints

int isascii(int c) {
	return c>=0 && c<=127;
}

int toascii(int c) {
	return c&127;
}

int isalnum(int c) {
	return isalnum_l(c, __get_current_locale());
}

int isalpha(int c) {
	return isalpha_l(c, __get_current_locale());
}

int isblank(int c) {
	return isblank_l(c, __get_current_locale());
}

int iscntrl(int c) {
	return iscntrl_l(c, __get_current_locale());
}

int isdigit(int c) {
	return isdigit_l(c, __get_current_locale());
}

int isgraph(int c) {
	return isgraph_l(c, __get_current_locale());
}

int islower(int c) {
	return islower_l(c,  __get_current_locale());
}

int isprint(int c) {
	return isprint_l(c,  __get_current_locale());
}

int ispunct(int c) {
	return ispunct_l(c, __get_current_locale());
}

int isspace(int c) {
	return isspace_l(c, __get_current_locale());
}

int isupper(int c) {
	return isupper_l(c, __get_current_locale());
}

int isxdigit(int c) {
	return isxdigit_l(c, __get_current_locale());
}

int tolower(int c) {
	return tolower_l(c, __get_current_locale());
}

int toupper(int c) {
	return toupper_l(c, __get_current_locale());
}

///////////////////////////////////////////////

static bool is_c_loc(locale_t loc) {
	return __is_c_locale(__get_locale_lc_ctype(loc));
}

static bool is_utf8_loc(locale_t loc) {
	return __is_utf8_locale(__get_locale_lc_ctype(loc));
}

#ifndef NDEBUG
static bool is_1252_loc(locale_t loc) {
	return __is_1252_locale(__get_locale_lc_ctype(loc));
}
#endif


int isalnum_l(int c, locale_t loc) {
	return isdigit_l(c, loc) || isalpha_l(c, loc);
}

static bool isrange(int c, int a, int b) {
	return c>=a && c<=b;
}

int isalpha_l(int c, locale_t loc) {
	assert(c==EOF || c>=0 && c<=255);
	if (is_c_loc(loc) || is_utf8_loc(loc))
		return (c>='a' && c<='z') || (c>='A' && c<='Z');
	else  {
		assert(is_1252_loc(loc));
		return twrRegExpTest1252("^\\p{Alphabetic}$", c);
		/*return isrange(c, 0x41, 0x5a) ||
			isrange(c, 0x61, 0x7a) ||
			isrange(c, 0x83, 0x83) ||
			isrange(c, 0x8a, 0x8a) ||
			isrange(c, 0x8c, 0x8c) ||
			isrange(c, 0x8e, 0x8e) ||
			isrange(c, 0x9a, 0x9a) ||
			isrange(c, 0x9c, 0x9c) ||
			isrange(c, 0x9e, 0x9f) ||
			isrange(c, 0xaa, 0xaa) ||
			isrange(c, 0xb5, 0xb5) ||
			isrange(c, 0xba, 0xba) ||
			isrange(c, 0xc0, 0xd6) ||
			isrange(c, 0xd8, 0xf6) ||
			isrange(c, 0xf8, 0xff);*/
	}
}

int isblank_l(int c, locale_t loc) {
	assert(c==EOF || c>=0 && c<=255);
	if (is_c_loc(loc) || is_utf8_loc(loc))
		return c==0x20 || c==0x09;
	else  {
		assert(is_1252_loc(loc));
		return c==0x09 || twrRegExpTest1252("^\\p{gc=Space_Separator}$", c);
		/*return c==0x20 || c==0x09 || c==0xa0;*/
	}
}

int iscntrl_l(int c, locale_t loc) {
	assert(c==EOF || c>=0 && c<=255);
	if (is_c_loc(loc) || is_utf8_loc(loc))
		return (c>=0x00 && c<=0x1F) || c==0x7F;
	else  {
		assert(is_1252_loc(loc));
		return twrRegExpTest1252("^\\p{gc=Control}$", c);
		/*return isrange(c, 0, 0x1f) || c==0x7F || c==0x81 || c==0x8d || isrange(c, 0x8f, 0x90) || c==0x9d || c==0xad;*/
	}
}

int isdigit_l(int c, locale_t loc) {
	assert(c==EOF || c>=0 && c<=255);
	if (is_c_loc(loc) || is_utf8_loc(loc))
		return c>='0' && c<='9';
	else  {
		assert(is_1252_loc(loc));
		return twrRegExpTest1252("^\\p{gc=Decimal_Number}$", c);
		/*return isrange(c, 0x30, 0x39) || c==0xb2 || c==0xb3 || c==0xb9;*/
	}
}

int isgraph_l(int c, locale_t loc) {
	assert(c==EOF || c>=0 && c<=255);

	if (is_c_loc(loc) || is_utf8_loc(loc))
		return c >='!' && c <= '~';  //0x21 to 0x7E 
	else  {
		assert(is_1252_loc(loc));
		return twrRegExpTest1252("^[^\\p{space}\\p{gc=Control}\\p{gc=Surrogate}\\p{gc=Unassigned}]$", c);
		/*return isrange(c, 0x21, 0x7e) || isrange(c, 0x82, 0x87) || isrange(c, 0x89, 0x8c) || c==0x8e || 
			isrange(c, 0x91, 0x97) || isrange(c, 0x9a, 0x9c) || isrange(c, 0x9e, 0x9f) || isrange(c, 0xa1, 0xff) ;*/
	}
}

int islower_l(int c, locale_t loc) {
	assert(c==EOF || c>=0 && c<=255);
	if (is_c_loc(loc) || is_utf8_loc(loc))
		return c>='a' && c<='z';
	else  {
		assert(is_1252_loc(loc));
		return twrRegExpTest1252("^\\p{Lowercase}$", c);   
		/*return isrange(c, 0x61, 0x7a) || isrange(c, 0x83, 0x83) || isrange(c, 0x9a, 0x9a) || isrange(c, 0x9c, 0x9c) || 
				 isrange(c, 0x9e, 0x9e) || isrange(c, 0xaa, 0xaa) || isrange(c, 0xb5, 0xb5) || isrange(c, 0xba, 0xba) || 
				 isrange(c, 0xdf, 0xf6) || isrange(c, 0xf8, 0xff) ;*/
	}
}

int isprint_l(int c, locale_t loc) {
	assert(c==EOF || c>=0 && c<=255);
	if (is_c_loc(loc) || is_utf8_loc(loc))
		return isgraph_l(c, loc) || c==0x20;
	else  {
		assert(is_1252_loc(loc));
		return (isgraph_l(c, loc) || isblank_l(c, loc)) && !iscntrl_l(c, loc);  // \p{graph} \p{blank} -- \p{cntrl}
		/*return c==0x09 || isrange(c, 0x20, 0x7e) || isrange(c, 0x82, 0x87) || isrange(c, 0x89, 0x8c) || c==0x8e || isrange(c, 0x91, 0x97) ||
				isrange(c, 0x9a, 0x9c) || isrange(c, 0x9e, 0xff) ;*/
	}
}

int ispunct_l(int c, locale_t loc) {
	assert(c==EOF || c>=0 && c<=255);
	if (is_c_loc(loc) || is_utf8_loc(loc))
		return isrange(c, 0x21, 0x2f) || isrange(c, 0x3a, 0x40) || isrange(c, 0x5b, 0x60) || isrange(c, 0x7b, 0x7e);
	else  {
		assert(is_1252_loc(loc));
		//return twrRegExpTest1252("^\\p{Punctuation}$", c);   // this version is "better", but not POSIX compatible
		return twrRegExpTest1252("^(?:(?!\\p{L})[\\p{P}\\p{S}])$", c);  // see https://unicode.org/reports/tr18/#punct

		/*return  isrange(c, 0x21, 0x2f) || isrange(c, 0x3a, 0x40) || isrange(c, 0x5b, 0x60) || isrange(c, 0x7b, 0x7e) ||
				isrange(c, 0x82, 0x82) || isrange(c, 0x84, 0x87) || isrange(c, 0x89, 0x89) || isrange(c, 0x8b, 0x8b) || isrange(c, 0x91, 0x97) || 
				isrange(c, 0x9b, 0x9b) || isrange(c, 0xa1, 0xbf) || isrange(c, 0xd7, 0xd7) || isrange(c, 0xf7, 0xf7)	;*/
	}
}

//In the POSIX locale, at a minimum, the <space>, <form-feed>, <newline>, <carriage-return>, <tab>, and <vertical-tab> shall be included.
int isspace_l(int c, locale_t loc) {
	assert(c==EOF || c>=0 && c<=255);
	if (is_c_loc(loc) || is_utf8_loc(loc))
		return c==' ' || c=='\f' || c=='\n'  || c=='\r'  || c=='\t'  || c=='\v' ;
	else  {
		assert(is_1252_loc(loc));
		return twrRegExpTest1252("\\s", c);
		//return c==' ' || c=='\f' || c=='\n'  || c=='\r'  || c=='\t'  || c=='\v' || c==0xa0;
	}
}

int isupper_l(int c, locale_t loc) {
	assert(c==EOF || c>=0 && c<=255);
	if (is_c_loc(loc) || is_utf8_loc(loc))
		return c>='A' && c<='Z';
	else  {
		assert(is_1252_loc(loc));
		return twrRegExpTest1252("^\\p{Uppercase}$", c);   
		/*return isrange(c, 0x41, 0x5a) || isrange(c, 0x8a, 0x8a) || isrange(c, 0x8c, 0x8c) || isrange(c, 0x8e, 0x8e) || isrange(c, 0x9f, 0x9f) || isrange(c, 0xc0, 0xd6) || isrange(c, 0xd8, 0xde) ;*/
	}
}

int isxdigit_l(int c, locale_t loc) {
	assert(c==EOF || c>=0 && c<=255);
	return (c>='0' && c<='9') || (c>='a' && c<='f') || (c>='A' && c<='F');
}

int tolower_l(int c, locale_t loc) {
	assert(c==EOF || c>=0 && c<=255);
	if (is_c_loc(loc) || is_utf8_loc(loc)) {
		if (c>='A' && c<='Z')
			return c-'A'+'a';
		else
			return c;
	}
	else
		return twrToLower1252(c);
}

int toupper_l(int c, locale_t loc) {
	assert(c==EOF || c>=0 && c<=255);
	if (is_c_loc(loc) || is_utf8_loc(loc)) {
		if (c>='a' && c<='z')
			return c-'a'+'A';
		else
			return c;
	}
	else
		return twrToUpper1252(c);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

static int test_all(int pick);
static int ascii_unit_tests(void);
static int test_tolower(int pick);
static int test_toupper(int pick);

int char_unit_test(void) {

	const char* lang=twrUserLanguage();
   if (strcmp(lang,"en-US")!=0) {
		printf("warning - char_unit_test built for en-US locale.  These tests may not be valid. lang=%s\n",lang);
	}

	if (ascii_unit_tests()==0) return 0;
	
	// 0xAE is (r) symbol in 1252 only (not in "C" or UTF-8)
	if (ispunct(0xAE)) return 0;
	if (isalpha(0xAE)) return 0;
	if (isgraph(0xAE)) return 0;
	if (isdigit(0xAE)) return 0;
	if (!test_all(0)) return 0;

	setlocale(LC_ALL, "C");
	if (ascii_unit_tests()==0) return 0;
	if (ispunct(0xAE)) return 0;
	if (isalpha(0xAE)) return 0;
	if (isgraph(0xAE)) return 0;
	if (isdigit(0xAE)) return 0;
	if (!test_all(0)) return 0;
	if (!test_tolower(0)) return 0;
	if (!test_toupper(0)) return 0;

	setlocale(LC_ALL, "");   // default is UTF-8 encoding
	if (ascii_unit_tests()==0) return 0;
	if (ispunct(0xAE)) return 0;
	if (isalpha(0xAE)) return 0;
	if (isgraph(0xAE)) return 0;
	if (isdigit(0xAE)) return 0;
	if (!test_all(0)) return 0;

	setlocale(LC_ALL, ".1252");
	if (!test_all(1)) return 0;
	if (ascii_unit_tests()==0) return 0;
	if (!ispunct(0xAE)) return 0;
	if (isalpha(0xAE)) return 0;
	if (!isgraph(0xAE)) return 0;
	if (isdigit(0xAE)) return 0;
	if (!test_tolower(1)) return 0;
	if (!test_toupper(1)) return 0;
	
	setlocale(LC_ALL, "C");

	return 1;
}

static int ascii_unit_tests(void) {

	if (isascii('g')!=1) return 0;

	if (toascii('Y'|128)!='Y') return 0;

	if (isalnum('!')) return 0;
	if (!isalnum('t')) return 0;
	if (!isalnum('0')) return 0;

	if (isalpha('1')) return 0;
	if (!isalpha('a')) return 0;
	if (!isalpha('Z')) return 0;
	if (isalpha('#')) return 0;

	if (!isblank(' ')) return 0;
	if (isblank('*')) return 0;

	if (iscntrl('6')) return 0;
	if (!iscntrl(7)) return 0;

	if (isdigit('a')) return 0;
	if (!isdigit('0')) return 0;
	if (!isdigit('9')) return 0;

	if (!isgraph('~')) return 0;
	if (isgraph(13)) return 0;

	if (islower('A')) return 0;
	if (!islower('z')) return 0;

	if (isprint(127)) return 0;
	if (!isprint(' ')) return 0;
	if (!isprint('A')) return 0;
	if (!isprint('!')) return 0;

	if (ispunct(127)) return 0;
	if (!ispunct('@')) return 0;
	if (!ispunct('\\')) return 0;
	if (!ispunct('!')) return 0;
	
	if (isspace(127)) return 0;
	if (isspace('b')) return 0;
	if (!isspace(' ')) return 0;
	if (!isspace('\t')) return 0;

	if (isupper('a')) return 0;
	if (isupper('!')) return 0;
	if (!isupper('Z')) return 0;

	if (isxdigit('G')) return 0;
	if (!isxdigit('0')) return 0;
	if (!isxdigit('f')) return 0;
	if (!isxdigit('A')) return 0;
	
	if (tolower('a')!='a') return 0;
	if (tolower('A')!='a') return 0;
	if (tolower('$')!='$') return 0;
	if (tolower('Z')!='z') return 0;
	if (tolower('5')!='5') return 0;


	if (toupper('a')!='A') return 0;
	if (toupper('A')!='A') return 0;
	if (toupper('$')!='$') return 0;
	if (toupper('z')!='Z') return 0;

	return 1;
}

static int test_do(int (*func)(int c), int a1[], int a2[], int pick) {
	int *a;
	a=pick==0?a1:a2;
	for (int i=0; i<256; i++) {
		int r=func(i)?1:0;
		if (r!=a[i]) 
			return 0;
	}
	return 1;
}

/*
static int test_show(char*name, int (*func)(int c), int a1[], int a2[], int pick) {
	printf("test_show '%s' %d:\n", name, pick);
	int *a;
	a=pick==0?a1:a2;
	for (int i=0; i<256; i++) {
		int r=func(i)?1:0;
		if (r!=a[i]) 
			printf("mismatch %d %d\n", i, r);
	}
	return 1;
}
*/

static int test_tolower(int pick) {
	int a1[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255};
	int a2[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,154,139,156,141,158,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,255,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,215,248,249,250,251,252,253,254,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255};

	if (pick==0) {
		for (int i=0; i<256; i++) {
			if (a1[i]!=tolower(i)) {
				printf("test_tolower fail %d %d %d %d\n", pick, i, a1[i], tolower(i));
				return 0;
			}
		}
	}

	if (pick==1) {
		for (int i=0; i<256; i++) {
			if (a2[i]!=tolower(i)) {
				printf("test_tolower fail %d %d %d %d\n", pick, i, a2[i], tolower(i));
				return 0;
			}
		}
	}

	return 1;
}

static int test_toupper(int pick) {
	int a1[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255};
	int a2[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,138,155,140,157,142,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,247,216,217,218,219,220,221,222,159};
	if (pick==0) {
		for (int i=0; i<256; i++) {
			if (a1[i]!=toupper(i)) {
				printf("test_toupper fail %d %d %d %d\n", pick, i, a1[i], toupper(i));
				return 0;
			}
		}
	}

	if (pick==1) {
		for (int i=0; i<256; i++) {
			if (a2[i]!=toupper(i)) {
				printf("test_toupper fail %d %d %d %d\n", pick, i, a2[i], toupper(i));
				return 0;
			}
		}
	}

	return 1;
}

static int test_isascii(int pick) {
	int a1[]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int a2[]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	return test_do(isascii, a1, a2, pick);
}

static int test_isalnum(int pick) {
	int a1[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int a2[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,0,1,0,0,0,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1};

	a2[136]=1;  // circumflex -- mismatch between windows (test case generation) and javascript

	a2[178]=0;  //sub/super scripts aren't really digits
	a2[179]=0; // sub/super scripts aren't really digits
	a2[185]=0; // sub/super scripts aren't really digits

	return test_do(isalnum, a1, a2, pick);
}

static int test_isalpha(int pick) {
	int a1[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int a2[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1};

	a2[136]=1;  // circumflex -- mismatch between windows (test case generation) and javascript

	return test_do(isalpha, a1, a2, pick);
}

static int test_isblank(int pick) {
	int a1[]={0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int a2[]={0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	return test_do(isblank, a1, a2, pick);
}

static int test_iscntrl(int pick) {
	int a1[]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int a2[]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	a2[173]=0;  // soft=hyphen, windows/unicode difference

	return test_do(iscntrl, a1, a2, pick);
}

static int test_isdigit(int pick) {
	int a1[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int a2[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	a2[178]=0;  // sub/super scripts aren't really digits
	a2[179]=0; // sub/super scripts aren't really digits
	a2[185]=0; // sub/super scripts aren't really digits

	return test_do(isdigit, a1, a2, pick);
}

static int test_isgraph(int pick) {
	int a1[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int a2[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,0,1,1,1,1,0,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

	// conform to unicode
   a2[128]=1;  
   a2[136]=1;
   a2[152]=1;
   a2[153]=1;

	return test_do(isgraph, a1, a2, pick);
}

static int test_islower(int pick) {
	int a1[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int a2[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1};

	return test_do(islower, a1, a2, pick);
}

static int test_isprint(int pick) {
	int a1[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int a2[]={0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,0,1,1,1,1,0,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
 
 // conform to unicode
   a2[9]=0;   
   a2[128]=1;
   a2[136]=1;
   a2[152]=1;
   a2[153]=1;

	return test_do(isprint, a1, a2, pick);
}

static int test_ispunct(int pick) {
	int a1[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int a2[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,1,0,1,1,1,1,0,1,0,1,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0};

	// differences betweens windows and unicode posix approximation
	a2[128]=1;  // euro
	a2[152]=1;  // tilde
	a2[153]=1;  // TM

	a2[170]=0;
	a2[173]=0;
	a2[178]=0;
	a2[179]=0;
	a2[181]=0;
	a2[185]=0;
	a2[186]=0;
	a2[188]=0;
	a2[189]=0;
	a2[190]=0;

	return test_do(ispunct, a1, a2, pick);
}

static int test_isspace(int pick) {
	int a1[]={0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int a2[]={0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	return test_do(isspace, a1, a2, pick);
}

static int test_isupper(int pick) {
	int a1[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int a2[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	return test_do(isupper, a1, a2, pick);
}

static int test_isxdigit(int pick) {
	int a1[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int a2[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	return test_do(isxdigit, a1, a2, pick);
}

static int test_all(int pick) {
	if (test_isascii(pick)==0) return 0;
	if (test_isdigit(pick)==0) return 0;
	if (test_isalpha(pick)==0) return 0;
	if (test_isalnum(pick)==0) return 0;
	if (test_isblank(pick)==0) return 0;
	if (test_iscntrl(pick)==0) return 0;
	if (test_isgraph(pick)==0) return 0;
	if (test_islower(pick)==0) return 0;
	if (test_isprint(pick)==0) return 0;
	if (test_ispunct(pick)==0) return 0;
	if (test_isspace(pick)==0) return 0;
	if (test_isupper(pick)==0) return 0;
	if (test_isxdigit(pick)==0) return 0;

	return 1;
}



