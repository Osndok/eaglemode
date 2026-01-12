//------------------------------------------------------------------------------
// emJpegImageFileModel.cpp
//
// Copyright (C) 2004-2009,2014,2018-2019,2025 Oliver Hamann.
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

#include <emJpeg/emJpegImageFileModel.h>
#include <emCore/emFileStream.h>
#include <setjmp.h>
extern "C" {
#	include <jerror.h>
#	include <jpeglib.h>
}


struct emJpegImageFileModel::LoadingState {
	LoadingState()
		: ImagePrepared(false),
		CInfoInitialized(false),
		NextY(0)
	{
		memset(&CInfo,0,sizeof(jpeg_decompress_struct));
		memset(&Err,0,sizeof(struct jpeg_error_mgr));
	}

	~LoadingState()
	{
		if (CInfoInitialized) jpeg_destroy_decompress(&CInfo);
	}

	bool ImagePrepared;
	bool CInfoInitialized;
	jpeg_decompress_struct CInfo;
	struct jpeg_error_mgr Err;
	emFileStream File;
	int NextY;
};


struct emJpegImageFileModel::SavingState {
	SavingState()
		: CInfoInitialized(false),
		RowSize(0),
		NextY(0)
	{
		memset(&CInfo,0,sizeof(jpeg_compress_struct));
		memset(&Err,0,sizeof(struct jpeg_error_mgr));
	}

	~SavingState()
	{
		if (CInfoInitialized) jpeg_destroy_compress(&CInfo);
	}

	bool CInfoInitialized;
	jpeg_compress_struct CInfo;
	struct jpeg_error_mgr Err;
	emFileStream File;
	emOwnArrayPtr<emByte> RowBuf;
	int RowSize;
	int NextY;
};


static void emJpegErrorExit(j_common_ptr cinfo)
{
	char errorText[JMSG_LENGTH_MAX+1];
	errorText[0]=0;
	(*cinfo->err->format_message)(cinfo,errorText);
	if (!errorText[0]) strcpy(errorText,"Unknown JPEG error.");
	throw emException("%s",errorText);
}


static void emJpegOutputMessage(j_common_ptr)
{
}


emRef<emJpegImageFileModel> emJpegImageFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emJpegImageFileModel,context,name,common)
}


emJpegImageFileModel::emJpegImageFileModel(
	emContext & context, const emString & name
)
	: emImageFileModel(context,name)
{
}


emJpegImageFileModel::~emJpegImageFileModel()
{
}


void emJpegImageFileModel::TryStartLoading()
{
	jpeg_saved_marker_ptr smp;
	const char * csstr;

	L=new LoadingState;

	L->CInfo.err=jpeg_std_error(&L->Err);
	L->Err.error_exit=emJpegErrorExit;
	L->Err.output_message=emJpegOutputMessage;

	jpeg_create_decompress(&L->CInfo);
	L->CInfoInitialized=true;

	L->File.TryOpen(GetFilePath(),"rb");
	jpeg_stdio_src(&L->CInfo,L->File.TryGetFile());

	jpeg_save_markers(&L->CInfo,JPEG_COM,0xffff);

	jpeg_read_header(&L->CInfo,TRUE);

	for (smp=L->CInfo.marker_list; smp; smp=smp->next) {
		if (smp->marker==JPEG_COM) {
			Comment=emString(
				(const char*)smp->data,
				smp->data_length
			);
		}
	}

	switch (L->CInfo.jpeg_color_space) {
	case JCS_GRAYSCALE:
		csstr="monochrome";
		L->CInfo.out_color_space=JCS_GRAYSCALE;
		break;
	case JCS_RGB:
		csstr="RGB";
		L->CInfo.out_color_space=JCS_RGB;
		break;
	case JCS_YCbCr:
		csstr="YUV";
		L->CInfo.out_color_space=JCS_RGB;
		break;
	case JCS_CMYK:
		csstr="CMYK";
		L->CInfo.out_color_space=JCS_RGB;
		break;
	case JCS_YCCK:
		csstr="YCCK";
		L->CInfo.out_color_space=JCS_RGB;
		break;
	default:
		csstr="unknown";
		L->CInfo.out_color_space=JCS_RGB;
		break;
	}

	FileFormatInfo=emString::Format("JPEG (%s)",csstr);

	Signal(ChangeSignal);

	L->CInfo.scale_num=1;
	L->CInfo.scale_denom=1;
	L->CInfo.output_gamma=1.0;
	L->CInfo.raw_data_out=FALSE;
	L->CInfo.quantize_colors=FALSE;
	L->CInfo.dct_method=JDCT_FLOAT;
	jpeg_start_decompress(&L->CInfo);

	if (
		(L->CInfo.output_components!=1 && L->CInfo.output_components!=3) ||
		L->CInfo.output_width<1 || L->CInfo.output_width>0x7fffff ||
		L->CInfo.output_height<1 || L->CInfo.output_height>0x7fffff
	) {
		throw emException("Unsupported JPEG file format.");
	}
}


bool emJpegImageFileModel::TryContinueLoading()
{
	JSAMPROW row;

	if (!L->ImagePrepared) {
		Image.Setup(
			L->CInfo.output_width,
			L->CInfo.output_height,
			L->CInfo.output_components
		);
		L->ImagePrepared=true;
		Signal(ChangeSignal);
	}

	if (L->NextY<Image.GetHeight()) {
		row=
			(JSAMPROW)Image.GetWritableMap()+
			L->NextY*(size_t)Image.GetWidth()*Image.GetChannelCount()
		;
		jpeg_read_scanlines(&L->CInfo,&row,1);
		L->NextY++;
		Signal(ChangeSignal);
	}

	if (L->NextY>=Image.GetHeight()) {
		jpeg_finish_decompress(&L->CInfo);
		return true;
	}

	return false;
}


void emJpegImageFileModel::QuitLoading()
{
	L.Reset();
}


void emJpegImageFileModel::TryStartSaving()
{
	S=new SavingState;

	S->CInfo.err=jpeg_std_error(&S->Err);
	S->Err.error_exit=emJpegErrorExit;
	S->Err.output_message=emJpegOutputMessage;

	jpeg_create_compress(&S->CInfo);
	S->CInfoInitialized=true;

	S->File.TryOpen(GetFilePath(),"wb");
	jpeg_stdio_dest(&S->CInfo,S->File.TryGetFile());

	S->CInfo.image_width=GetImage().GetWidth();
	S->CInfo.image_height=GetImage().GetHeight();
	if (GetImage().HasAnyNonGreyPixel()) {
		S->CInfo.input_components=3;
		S->CInfo.in_color_space=JCS_RGB;
	}
	else {
		S->CInfo.input_components=1;
		S->CInfo.in_color_space=JCS_GRAYSCALE;
	}
	jpeg_set_defaults(&S->CInfo);
	S->CInfo.dct_method=JDCT_FLOAT;
	S->CInfo.optimize_coding=TRUE;
	jpeg_set_quality(&S->CInfo,GetSavingQuality(),TRUE);
	jpeg_start_compress(&S->CInfo,TRUE);
	if (!Comment.IsEmpty()) {
		jpeg_write_marker(
			&S->CInfo,JPEG_COM,
			(const JOCTET*)Comment.Get(),
			Comment.GetLen()
		);
	}
	S->RowSize=GetImage().GetWidth()*S->CInfo.input_components;
	S->RowBuf=new emByte[S->RowSize];
}


bool emJpegImageFileModel::TryContinueSaving()
{
	emString str;
	emByte * p;
	JSAMPROW row;
	int x,y,w;
	emColor c;

	y=S->NextY++;
	if (y<GetImage().GetHeight()) {
		w=GetImage().GetWidth();
		row=S->RowBuf;
		p=row;
		if (S->CInfo.input_components==1) {
			for (x=0; x<w; x++) {
				c=GetImage().GetPixel(x,y);
				*p++=c.GetGrey();
			}
		}
		else {
			for (x=0; x<w; x++) {
				c=GetImage().GetPixel(x,y);
				p[0]=c.GetRed();
				p[1]=c.GetGreen();
				p[2]=c.GetBlue();
				p+=3;
			}
		}
		jpeg_write_scanlines(&S->CInfo,&row,1);
		return false;
	}

	jpeg_finish_compress(&S->CInfo);
	S->File.TryClose();

	str="JPEG";
	if (FileFormatInfo!=str) {
		FileFormatInfo=str;
		Signal(ChangeSignal);
	}

	return true;
}


void emJpegImageFileModel::QuitSaving()
{
	S.Reset();
}


emUInt64 emJpegImageFileModel::CalcMemoryNeed()
{
	if (L) {
		return
			((emUInt64)L->CInfo.output_width)*
			L->CInfo.output_height*
			L->CInfo.output_components+
			Comment.GetLen()
		;
	}
	else {
		return
			((emUInt64)Image.GetWidth())*
			Image.GetHeight()*
			Image.GetChannelCount()+
			Comment.GetLen()
		;
	}
}


double emJpegImageFileModel::CalcFileProgress()
{
	if (L && L->CInfo.output_height>0) {
		return 100.0*L->NextY/L->CInfo.output_height;
	}
	if (S && Image.GetHeight()>0) {
		return 100.0*S->NextY/Image.GetHeight();
	}
	else {
		return 0.0;
	}
}
