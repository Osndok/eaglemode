//------------------------------------------------------------------------------
// emWebpImageFileModel.cpp
//
// Copyright (C) 2021 Oliver Hamann.
//
// Homepage: http://eaglemode.sourceforge.net/
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 3 as published by the
// Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License version 3 for
// more details.
//
// You should have received a copy of the GNU General Public License version 3
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#include <emWebp/emWebpImageFileModel.h>
#include <webp/decode.h>


struct emWebpLoadingState {
	FILE * file;
	emArray<unsigned char> data;
	bool featuresValid;
	WebPBitstreamFeatures features;
	WebPIDecoder * decoder;
	int lastY;
};


static emString emWebpVP8StatusCodeToString(VP8StatusCode sc)
{
	switch (sc) {
	case VP8_STATUS_OK:                  return "ok";
	case VP8_STATUS_OUT_OF_MEMORY:       return "out of memory";
	case VP8_STATUS_INVALID_PARAM:       return "invalid param";
	case VP8_STATUS_BITSTREAM_ERROR:     return "bitstream error";
	case VP8_STATUS_UNSUPPORTED_FEATURE: return "unsupported feature";
	case VP8_STATUS_SUSPENDED:           return "suspended";
	case VP8_STATUS_USER_ABORT:          return "user abort";
	case VP8_STATUS_NOT_ENOUGH_DATA:     return "not enough data";
	default: return emString::Format("unknown status code: %d",(int)sc);
	}
}


emRef<emWebpImageFileModel> emWebpImageFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emWebpImageFileModel,context,name,common)
}


emWebpImageFileModel::emWebpImageFileModel(
	emContext & context, const emString & name
)
	: emImageFileModel(context,name)
{
	L=NULL;
}


emWebpImageFileModel::~emWebpImageFileModel()
{
	emWebpImageFileModel::QuitLoading();
	emWebpImageFileModel::QuitSaving();
}


void emWebpImageFileModel::TryStartLoading()
{
	L=new emWebpLoadingState;
	L->file=NULL;
	L->featuresValid=false;
	memset(&L->features,0,sizeof(L->features));
	L->decoder=NULL;
	L->lastY=0;

	L->file=fopen(GetFilePath(),"rb");
	if (!L->file) throw emException("%s",emGetErrorText(errno).Get());

	Signal(ChangeSignal);
}


bool emWebpImageFileModel::TryContinueLoading()
{
	int len,len2;
	VP8StatusCode sc;

	if (!L->featuresValid) {
		len=L->data.GetCount();
		len2=4096;
		L->data.SetCount(len+len2);
		len2=fread(L->data.GetWritable()+len,1,len2,L->file);
		if (len2<=0) {
			if (ferror(L->file)) {
				throw emException("%s",emGetErrorText(errno).Get());
			}
			else  {
				throw emException("WebP header not found");
			}
		}
		L->data.SetCount(len+len2);

		sc=WebPGetFeatures(L->data.Get(),L->data.GetCount(),&L->features);
		if (sc==VP8_STATUS_NOT_ENOUGH_DATA) {
			return false;
		}
		if (sc!=VP8_STATUS_OK) {
			throw emException("Failed to decode WebP header: %s",emWebpVP8StatusCodeToString(sc).Get());
		}

		if (
			L->features.width < 1 || L->features.height < 1 ||
			L->features.width > 0x7fffff || L->features.height > 0x7fffff
		) {
			throw emException("Unsupported WebP dimensions");
		}

		if (fseek(L->file,0,SEEK_SET)) {
			throw emException("%s",emGetErrorText(errno).Get());
		}
		L->data.Clear();
		L->featuresValid=true;

		FileFormatInfo="WebP";
		if (L->features.has_alpha) FileFormatInfo+=" with alpha";
		else FileFormatInfo+=" without alpha";
		if (L->features.has_animation) FileFormatInfo+=", animated";
		else FileFormatInfo+=", not animated";
		if (L->features.format==0) FileFormatInfo+=", undefined/mixed format";
		else if (L->features.format==1) FileFormatInfo+=", lossy format";
		else if (L->features.format==2) FileFormatInfo+=", lossless format";
		else FileFormatInfo+=emString::Format(", format %d",L->features.format);

		Image.Setup(
			L->features.width,
			L->features.height,
			L->features.has_alpha ? 4 : 3
		);

		L->decoder=WebPINewRGB(
			Image.GetChannelCount() > 3 ? MODE_RGBA : MODE_RGB,
			Image.GetWritableMap(),
			Image.GetWidth()*(size_t)Image.GetHeight()*Image.GetChannelCount(),
			Image.GetWidth()*(size_t)Image.GetChannelCount()
		);

		Signal(ChangeSignal);
		return false;
	}

	len=8192;
	L->data.SetCount(len);
	len=fread(L->data.GetWritable(),1,len,L->file);
	if (len<=0) {
		if (ferror(L->file)) {
			throw emException("%s",emGetErrorText(errno).Get());
		}
		else  {
			throw emException("WebP data incomplete");
		}
	}
	L->data.SetCount(len);

	sc=WebPIAppend(L->decoder,L->data.Get(),L->data.GetCount());
	if (sc!=VP8_STATUS_OK && sc!=VP8_STATUS_SUSPENDED) {
		throw emException("Failed to decode WebP data: %s",emWebpVP8StatusCodeToString(sc).Get());
	}
	if (!WebPIDecGetRGB(L->decoder,&L->lastY,NULL,NULL,NULL)) {
		L->lastY=0;
	}
	if (sc!=VP8_STATUS_OK) {
		return false;
	}

	Signal(ChangeSignal);
	return true;
}


void emWebpImageFileModel::QuitLoading()
{
	if (L) {
		if (L->decoder) WebPIDelete(L->decoder);
		if (L->file) fclose(L->file);
		delete L;
		L=NULL;
	}
}


void emWebpImageFileModel::TryStartSaving()
{
	throw emException("emWebpImageFileModel: Saving not implemented.");
}


bool emWebpImageFileModel::TryContinueSaving()
{
	return false;
}


void emWebpImageFileModel::QuitSaving()
{
}


emUInt64 emWebpImageFileModel::CalcMemoryNeed()
{
	if (!L) {
		return
			((emUInt64)Image.GetWidth())*
			Image.GetHeight()*
			Image.GetChannelCount()
		;
	}
	else if (L->featuresValid) {
		return
			((emUInt64)L->features.width)*
			L->features.height*
			(L->features.has_alpha?4:3)
		;
	}
	else {
		return 0;
	}
}


double emWebpImageFileModel::CalcFileProgress()
{
	if (L && L->featuresValid && L->features.height>0) {
		return 100.0*L->lastY/L->features.height;
	}
	else {
		return 0.0;
	}
}
