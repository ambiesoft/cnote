#include "stdafx.h"
#include <cassert>

#include "../../../lsMisc/UTF16toUTF8.h"

// run 'git submodule update -i'
#include "compact_enc_det/compact_enc_det.h"

#include "detect.h"

using namespace std;
using namespace Ambiesoft;
wstring ConvertEncoding(const vector<char>& input)
{
	int bytes_consumed = 0;
	bool is_reliable = false;

	Language gglLang = UNKNOWN_LANGUAGE;
	const Encoding enc = CompactEncDet::DetectEncoding(
		input.data(),
		input.size(),
		nullptr,                            // url hint
		nullptr,                            // http hint
		nullptr,                            // meta hint
		UNKNOWN_ENCODING,                   // enc hint
		gglLang,                            // lang hint
		CompactEncDet::WEB_CORPUS,
		true,                               // Include 7-bit encodings
		&bytes_consumed, 
		&is_reliable);

	// https://docs.microsoft.com/en-us/windows/win32/intl/code-page-identifiers
	// https://docs.google.com/spreadsheets/d/1oej65p9ZcbUeU-2MsGUvoU_ums9lr4G5QRlc8J0NKvM/edit?usp=sharing
	int codepage = 0;
#define CODEMAP(ggl,cp) case ggl: codepage=cp; break
	switch (enc)
	{
		CODEMAP(ISO_8859_1, 28591);
		CODEMAP(ISO_8859_2, 28592);
		CODEMAP(ISO_8859_3, 28593);
		CODEMAP(ISO_8859_4, 28594);
		CODEMAP(ISO_8859_5, 28595);
		CODEMAP(ISO_8859_6, 28596);
		CODEMAP(ISO_8859_7, 28597);
		CODEMAP(ISO_8859_8, 28598);
		CODEMAP(ISO_8859_9, 28599);
		CODEMAP(JAPANESE_EUC_JP, 51932);
		CODEMAP(JAPANESE_SHIFT_JIS, 932);
		CODEMAP(JAPANESE_JIS, 50222);
		CODEMAP(CHINESE_BIG5, 950);
		CODEMAP(CHINESE_GB, 936);
		CODEMAP(CHINESE_EUC_CN, 51936);
		CODEMAP(KOREAN_EUC_KR, 51949);
		CODEMAP(UTF8, 65001);
	
	case UTF16LE:
		return wstring((wchar_t*)input.data());
	default:
		assert(false);
	}
#undef CODEMAP

	if (codepage == 0)
		codepage = 65001;
	return toStdWstring(codepage, input.data(), input.size());
}
